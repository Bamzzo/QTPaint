#include "paintarea.h"
#include <QPainterPath>
#include <QDebug>
#include <QResizeEvent>
#include <cmath>
#include <QFileDialog>
#include "shapes.h"

PaintArea::PaintArea(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);

    isSelecting = false;
    selectionRect = QRect();
    drawing = false;
    currentShapeType = Freehand;
    image = QImage(800, 600, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    tempImage = image;

    penColor = Qt::black;
    penWidth = 3;
    undoStack.push(image);
}

void PaintArea::setPenColor(const QColor &color)
{
    penColor = color;
}

void PaintArea::setPenWidth(int width)
{
    penWidth = width;
}

void PaintArea::setDrawShape(DrawShape shape)
{
    currentShapeType = shape;
}

void PaintArea::updateScaleAndOffset()
{
    QSize widgetSize = size();

    if (originalImage.isNull()) {
        scaleFactor = 1.0;
        offset = QPoint(0, 0);
        origImageSize = widgetSize;
    } else {
        origImageSize = originalImage.size();
        scaleFactor = qMin(static_cast<double>(widgetSize.width()) / origImageSize.width(),
                           static_cast<double>(widgetSize.height()) / origImageSize.height());
        offset = QPoint((widgetSize.width() - origImageSize.width() * scaleFactor) / 2,
                        (widgetSize.height() - origImageSize.height() * scaleFactor) / 2);
    }
}

QPoint PaintArea::physicalToLogical(const QPoint &physicalPoint) const
{
    QPoint adjustedPoint = physicalPoint;

    if (!originalImage.isNull()) {
        QRect contentRect(offset, origImageSize * scaleFactor);
        adjustedPoint.setX(qBound(contentRect.left(), physicalPoint.x(), contentRect.right()));
        adjustedPoint.setY(qBound(contentRect.top(), physicalPoint.y(), contentRect.bottom()));
    }

    if (originalImage.isNull()) {
        return QPoint(
            qBound(0, physicalPoint.x(), width()-1),
            qBound(0, physicalPoint.y(), height()-1));
    }

    int x = qBound(0.0, (physicalPoint.x() - offset.x()) / scaleFactor, origImageSize.width() - 1.0);
    int y = qBound(0.0, (physicalPoint.y() - offset.y()) / scaleFactor, origImageSize.height() - 1.0);
    return QPoint(x, y);
}

QPoint PaintArea::logicalToPhysical(const QPoint &logicalPoint) const
{
    if (originalImage.isNull()) {
        return logicalPoint;
    }
    return QPoint(
        logicalPoint.x() * scaleFactor + offset.x(),
        logicalPoint.y() * scaleFactor + offset.y()
        );
}

QRect PaintArea::logicalToPhysical(const QRect &logicalRect) const
{
    return QRect(logicalToPhysical(logicalRect.topLeft()),
                 QSize(logicalRect.width() * scaleFactor,
                       logicalRect.height() * scaleFactor));
}

void PaintArea::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaleAndOffset();


    if (!originalImage.isNull()) {
        if (image.size() != originalImage.size()) {
            QImage newImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
            newImage.fill(Qt::transparent);
            QPainter painter(&newImage);
            painter.drawImage(0, 0, image);
            image = newImage;
        }
    } else {
        if (image.size() != event->size()) {
            QImage newImage(event->size(), QImage::Format_ARGB32_Premultiplied);
            newImage.fill(Qt::transparent);
            QPainter painter(&newImage);
            painter.drawImage(0, 0, image);
            image = newImage;
        }
    }

    tempImage = image;
    update();
}

void PaintArea::saveImage(const QString &fileName)
{
    QImage finalImage = originalImage.isNull() ? image : originalImage.copy();
    QPainter painter(&finalImage);
    painter.drawImage(0, 0, image);
    finalImage.save(fileName, "PNG");
}

void PaintArea::loadImage(const QString &fileName)
{
    QImage loadedImage;
    if (!loadedImage.load(fileName)) return;

    // 保存当前状态到撤销栈
    saveState();

    originalImage = loadedImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    image = QImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    // 保存仅包含图片的状态到撤销栈
    QImage initialState = originalImage.copy();
    undoStack.push(initialState);
    redoStack.clear();

    updateScaleAndOffset();
    update();
}


void PaintArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), Qt::white);

    if (!originalImage.isNull()) {
        QRect drawRect(offset.x(), offset.y(),
                       origImageSize.width() * scaleFactor,
                       origImageSize.height() * scaleFactor);
        painter.drawImage(drawRect, originalImage);
    }

    QRect contentRect = !originalImage.isNull() ?
                            QRect(offset, origImageSize * scaleFactor) :
                            rect();
    painter.drawImage(contentRect, image, image.rect());

    if (drawing) {
        painter.drawImage(contentRect, tempImage, tempImage.rect());
    }

    if (isSelecting) {
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(QRect(
            logicalToPhysical(selectionRect.topLeft()),
            logicalToPhysical(selectionRect.bottomRight()))
                         );
    }
}

void PaintArea::mousePressEvent(QMouseEvent *event)
{
    if (currentShapeType == GroupSelect) {
        selectionStart = physicalToLogical(event->pos());
        isSelecting = true;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        QPoint logicalPoint = physicalToLogical(event->pos());
        drawing = true;
        tempImage = image.copy();

        // 根据当前形状类型创建对应的Shape对象
        switch(currentShapeType) {
        case Freehand:
            currentShape = new PathShape(logicalPoint, penColor, penWidth, false);
            break;
        case Line:
            currentShape = new LineShape(logicalPoint, penColor, penWidth);
            break;
        case Rectangle:
            currentShape = new RectangleShape(logicalPoint, penColor, penWidth);
            break;
        case Ellipse:
            currentShape = new EllipseShape(logicalPoint, penColor, penWidth);
            break;
        case Arrow:
            currentShape = new ArrowShape(logicalPoint, penColor, penWidth);
            break;
        case Star:
            currentShape = new StarShape(logicalPoint, penColor, penWidth);
            break;
        case Diamond:
            currentShape = new DiamondShape(logicalPoint, penColor, penWidth);
            break;
        case Heart:
            currentShape = new HeartShape(logicalPoint, penColor, penWidth);
            break;
        case Eraser:
            currentShape = new PathShape(logicalPoint, Qt::white, penWidth, true);
            break;
        case GroupSelect:
            // 已在上面处理
            break;
        }
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentLogicalPos = physicalToLogical(event->pos());
    emit cursorPositionChanged(currentLogicalPos);


    if (isSelecting) {
        selectionEnd = currentLogicalPos;
        selectionRect = QRect(selectionStart, selectionEnd).normalized();
        tempImage = image.copy();
        QPainter painter(&tempImage);
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(selectionRect);
        update();
        return;
    }

    if ((event->buttons() & Qt::LeftButton) && drawing && currentShape) {
        currentShape->update(currentLogicalPos);

        // 实时绘制到临时图像
        tempImage = image.copy();
        QPainter painter(&tempImage);
        currentShape->draw(painter);

        update();
    }
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (isSelecting) {
        isSelecting = false;
        if (!selectionRect.isNull()) {
            QImage newImage = image.copy();
            QPainter painter(&newImage);
            QPoint logicalEnd = physicalToLogical(event->pos());
            QPoint offset = logicalEnd - selectionStart;

            QImage selectedArea = image.copy(selectionRect);
            painter.fillRect(selectionRect, Qt::white);
            painter.drawImage(selectionRect.translated(offset), selectedArea, selectionRect);

            image = newImage;
            saveState();
            update();
        }
        return;
    }


    if (event->button() == Qt::LeftButton && drawing && currentShape) {
        QPainter painter(&image);
        currentShape->draw(painter);

        delete currentShape;
        currentShape = nullptr;  // 添加这行

        saveState();
        drawing = false;
        update();
    }

}

void PaintArea::clearSelection()
{
    isSelecting = false;
    selectionRect = QRect();
    update();
}

void PaintArea::undo()
{
    if (undoStack.size() > 1) {
        redoStack.push(undoStack.pop());
        QImage stateImage = undoStack.top();

        originalImage = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        originalImage.fill(Qt::transparent);
        QPainter painter(&originalImage);
        painter.drawImage(0, 0, stateImage);

        image = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QResizeEvent fakeEvent(size(), size());
        resizeEvent(&fakeEvent);
        update();
    }
}

void PaintArea::redo()
{
    if (!redoStack.isEmpty()) {
        undoStack.push(redoStack.pop());
        QImage stateImage = undoStack.top();

        originalImage = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        originalImage.fill(Qt::transparent);
        QPainter painter(&originalImage);
        painter.drawImage(0, 0, stateImage);

        image = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QResizeEvent fakeEvent(size(), size());
        resizeEvent(&fakeEvent);
        update();
    }
}

void PaintArea::saveState()
{
    QImage stateImage = originalImage.isNull() ?
                            image.copy() :
                            originalImage.copy();

    QPainter painter(&stateImage);
    painter.drawImage(0, 0, image);

    if (undoStack.isEmpty() || undoStack.top() != stateImage) {
        undoStack.push(stateImage);
        if (undoStack.size() > 50) undoStack.removeFirst();
        redoStack.clear();
    }
}
