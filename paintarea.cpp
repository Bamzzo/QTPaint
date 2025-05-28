#include "paintarea.h"
#include <QPainterPath>
#include <QDebug>
#include <QResizeEvent>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PaintArea::PaintArea(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);  // 新增：启用鼠标追踪

    isSelecting = false;
    selectionRect = QRect();
    drawing = false;
    currentShape = Freehand;
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
    currentShape = shape;
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

    // 添加边界保护
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

    QSize newSize = event->size();
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
            newImage.fill(Qt::white);
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

    saveState();
    originalImage = loadedImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // 修改为直接使用原始尺寸
    image = QImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.drawImage(0, 0, originalImage); // 将原始图像绘制到当前画布

    // 触发尺寸更新
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
    if (currentShape == GroupSelect) {
        selectionStart = physicalToLogical(event->pos());
        isSelecting = true;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        firstPoint = lastPoint = physicalToLogical(event->pos());
        drawing = true;
        tempImage = image.copy();
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentLogicalPos = physicalToLogical(event->pos());

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

    if ((event->buttons() & Qt::LeftButton) && drawing) {
        if (currentShape == Freehand || currentShape == Eraser) {
            QPainter painter(&tempImage);
            painter.setPen(currentShape == Eraser ?
                               QPen(Qt::white, penWidth, Qt::SolidLine, Qt::RoundCap) :
                               QPen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(lastPoint, currentLogicalPos);
            lastPoint = currentLogicalPos;
            update();
        } else {
            tempImage = image.copy();
            drawShapeToImage(currentLogicalPos);
        }
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

    if (event->button() == Qt::LeftButton && drawing) {
        QPainter painter(&image);
        painter.drawImage(0, 0, tempImage);
        saveState();
        drawing = false;
        update();
    }
}

// 新增清除选择方法
void PaintArea::clearSelection()
{
    isSelecting = false;
    selectionRect = QRect();
    update();
}

void PaintArea::drawShapeToImage(const QPoint &endPoint)
{
    QPainter painter(&tempImage);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect rect = QRect(firstPoint, endPoint).normalized();  // 修正变量名

    QPen pen(currentShape == Eraser ? Qt::white : penColor, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    if (currentShape != Eraser) {
        QBrush brush(currentShape == Heart ? penColor : (currentShape == Rectangle ||
                                                         currentShape == Ellipse || currentShape == Star ||
                                                         currentShape == Diamond) ? penColor : Qt::NoBrush);
        painter.setBrush(brush);
    }
    switch (currentShape) {
    case Freehand:  // 已在mouseMoveEvent处理
    case Eraser:    // 已在mouseMoveEvent处理
        break;
    case Line:
        painter.drawLine(firstPoint, endPoint);
        break;
    case Rectangle:
        painter.drawRect(rect);
        break;
    case Ellipse:
        painter.drawEllipse(rect);
        break;
    case Arrow: {
        painter.drawLine(firstPoint, endPoint);
        qreal arrowSize = penWidth * 4;
        QLineF line(endPoint, firstPoint);
        double angle = std::atan2(-line.dy(), line.dx());

        QPointF arrowP1 = endPoint + QPointF(
                              std::sin(angle + M_PI/3) * arrowSize,
                              std::cos(angle + M_PI/3) * arrowSize
                              );
        QPointF arrowP2 = endPoint + QPointF(
                              std::sin(angle + M_PI - M_PI/3) * arrowSize,
                              std::cos(angle + M_PI - M_PI/3) * arrowSize
                              );

        painter.drawLine(endPoint, arrowP1);
        painter.drawLine(endPoint, arrowP2);
        break;
    }
    case Star:
        drawStar(painter, rect);
        break;
    case Diamond:
        drawDiamond(painter, rect);
        break;
    case Heart:
        drawHeart(painter, rect);
        break;
    case GroupSelect: // 无需绘制
        break;
    default:
        qWarning() << "Unhandled shape type:" << currentShape;
        break;
    }
    update();
}


void PaintArea::undo()
{
    if (undoStack.size() > 1) {
        redoStack.push(undoStack.pop());
        QImage stateImage = undoStack.top();

        // 分离原始图像和绘制层
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
    // 创建组合图像进行保存
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


void PaintArea::drawStar(QPainter &painter, const QRect &rect)
{
    QPainterPath path;
    qreal radius = qMin(rect.width(), rect.height()) / 2;
    QPoint center = rect.center();

    for (int i = 0; i < 5; ++i) {
        qreal angle = 2 * M_PI * i / 5 - M_PI/2;
        QPointF outerPoint = center + QPointF(radius * cos(angle), radius * sin(angle));
        if (i == 0) {
            path.moveTo(outerPoint);
        } else {
            path.lineTo(outerPoint);
        }

        angle += M_PI / 5;
        QPointF innerPoint = center + QPointF(radius/2 * cos(angle), radius/2 * sin(angle));
        path.lineTo(innerPoint);
    }
    path.closeSubpath();
    painter.drawPath(path);
}

void PaintArea::drawDiamond(QPainter &painter, const QRect &rect)
{
    QPolygon diamond;
    diamond << QPoint(rect.center().x(), rect.top())
            << QPoint(rect.right(), rect.center().y())
            << QPoint(rect.center().x(), rect.bottom())
            << QPoint(rect.left(), rect.center().y());
    painter.drawPolygon(diamond);
}




void PaintArea::drawHeart(QPainter &painter, const QRect &rect)
{
    QPainterPath path;
    qreal scale = qMin(rect.width(), rect.height()) / 100;
    QPoint center = rect.center();

    path.moveTo(center.x(), center.y() + 25*scale);
    path.cubicTo(center.x() + 45*scale, center.y() - 55*scale,
                 center.x() + 95*scale, center.y() - 35*scale,
                 center.x(), center.y() + 45*scale);
    path.cubicTo(center.x() - 95*scale, center.y() - 35*scale,
                 center.x() - 45*scale, center.y() - 55*scale,
                 center.x(), center.y() + 25*scale);
    painter.fillPath(path, penColor);
}
