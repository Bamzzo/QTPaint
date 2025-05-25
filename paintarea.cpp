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

    isSelecting = false;
    selectionRect = QRect();
    drawing = false;
    currentShape = Freehand;
    // 使用固定初始尺寸（后续会根据窗口自动调整）
    image = QImage(800, 600, QImage::Format_ARGB32_Premultiplied); // 修改格式
    image.fill(Qt::white);
    tempImage = image;

    penColor = Qt::black;
    penWidth = 3;
    undoStack.push(image);
    isSelecting = false;
    selectionRect = QRect(0,0,0,0);
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

void PaintArea::saveImage(const QString &fileName)
{
    image.save(fileName, "PNG");
}

void PaintArea::updateScaleAndOffset()
{
    if (originalImage.isNull()) {
        scaleFactor = 1.0;
        offset = QPoint(0, 0);
        return;
    }

    QSize widgetSize = size();
    QSize imageSize = originalImage.size();

    scaleFactor = qMin(static_cast<double>(widgetSize.width()) / imageSize.width(),
                       static_cast<double>(widgetSize.height()) / imageSize.height());

    offset = QPoint((widgetSize.width() - imageSize.width() * scaleFactor) / 2,
                    (widgetSize.height() - imageSize.height() * scaleFactor) / 2);
}

QPoint PaintArea::physicalToLogical(const QPoint &physicalPoint) const
{
    return QPoint((physicalPoint - offset).x() / scaleFactor,
                  (physicalPoint - offset).y() / scaleFactor);
}

QPoint PaintArea::logicalToPhysical(const QPoint &logicalPoint) const
{
    return QPoint(logicalPoint.x() * scaleFactor + offset.x(),
                  logicalPoint.y() * scaleFactor + offset.y());
}

QRect PaintArea::logicalToPhysical(const QRect &logicalRect) const
{
    return QRect(logicalToPhysical(logicalRect.topLeft()),
                 QSize(logicalRect.width() * scaleFactor,
                       logicalRect.height() * scaleFactor));
}

// 修改resizeEvent
void PaintArea::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaleAndOffset();

    // 创建新尺寸的透明层，并保留原有内容（缩放）
    QImage newImage(event->size(), QImage::Format_ARGB32_Premultiplied);
    newImage.fill(Qt::transparent);
    QPainter painter(&newImage);
    painter.drawImage(0, 0, image.scaled(event->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    image = newImage;
    tempImage = image;

    update();
}

// 修改loadImage
// paintarea.cpp
void PaintArea::loadImage(const QString &fileName)
{
    QImage loadedImage;
    if (!loadedImage.load(fileName)) {
        qWarning() << "Failed to load image:" << fileName;
        return;
    }
    saveState();
    originalImage = loadedImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // 初始化用户绘制层为原始图片尺寸（非窗口尺寸）
    image = QImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    updateScaleAndOffset();
    resize(size());  // 触发resizeEvent进行缩放
    update();
}

// 修改paintEvent
void PaintArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制原始图片（带缩放）
    if (!originalImage.isNull()) {
        QImage scaled = originalImage.scaled(originalImage.size() * scaleFactor,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        painter.drawImage(offset, scaled);
    }

    // 绘制用户内容（实时缩放）
    QImage userScaled = image.scaled(image.size() * scaleFactor,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
    painter.drawImage(offset, userScaled);

    // 绘制临时图形（物理坐标）
    if (drawing) {
        painter.drawImage(0, 0, tempImage);
    }
}

// 修改鼠标事件处理（以press事件为例）
void PaintArea::mousePressEvent(QMouseEvent *event)
{
    if (currentShape == GroupSelect) {
        selectionStart = physicalToLogical(event->pos());  // 转换为逻辑坐标
        isSelecting = true;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        firstPoint = lastPoint = physicalToLogical(event->pos());  // 转换为逻辑坐标
        drawing = true;
        tempImage = image.copy();
    }
}
// 修改1：优化resizeEvent处理逻辑


// 修改4：修复撤销/重做逻辑
void PaintArea::undo()
{
    if (undoStack.size() > 1) {
        redoStack.push(undoStack.pop());
        image = undoStack.top().copy();
        tempImage = image;
        update(); // 确保界面刷新
    }
}
void PaintArea::redo()
{
    if (!redoStack.isEmpty()) {
        undoStack.push(redoStack.top());
        image = redoStack.pop().copy(); // 直接恢复历史状态
        tempImage = image;
        update();
    }
}



void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    selectionEnd = physicalToLogical(event->pos());
    if (isSelecting) {
        selectionEnd = physicalToLogical(event->pos());
        selectionRect = QRect(selectionStart, selectionEnd).normalized();
        tempImage = image.copy();
        QPainter painter(&tempImage);
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(selectionRect);
        update();
            }
    if ((event->buttons() & Qt::LeftButton) && drawing) {
        if (currentShape == Freehand || currentShape == Eraser) {
            drawShapeToImage(event->pos());
            lastPoint = physicalToLogical(event->pos());
        } else {
            tempImage = image;
            drawShapeToImage(event->pos());
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

            // 计算逻辑坐标偏移量
            QPoint logicalEnd = physicalToLogical(event->pos());
            QPoint offset = logicalEnd - selectionStart;

            // 复制并移动选中区域
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
        // 合并临时图像到主图像
        QPainter painter(&image);
        painter.drawImage(0, 0, tempImage);

        saveState();
        drawing = false;
        update();
    }
}

void PaintArea::resizeImage(const QSize &newSize)
{
    if (image.size() != newSize) {
        QImage newImage(newSize, QImage::Format_ARGB32_Premultiplied);
        newImage.fill(Qt::white);
        QPainter painter(&newImage);
        painter.drawImage(QPoint(0, 0), image);
        image = newImage;
        tempImage = image;
        update();
    }
}

void PaintArea::drawShapeToImage(const QPoint &endPoint)
{
    QPainter painter(&tempImage);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPen pen(penColor, penWidth);
    QBrush brush(penColor);


    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setOpacity(1.0);



    if (currentShape == Eraser) {
        QPen eraserPen(Qt::white);
        eraserPen.setWidth(penWidth);
        eraserPen.setCapStyle(Qt::RoundCap);
        eraserPen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(eraserPen);
        painter.drawLine(lastPoint, endPoint);
    } else {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        QPen pen(penColor, penWidth);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(penColor);
    }

    QRect rect = QRect(firstPoint, endPoint).normalized();

    switch (currentShape) {
    case Freehand:
    case Eraser: // 合并处理自由绘制和橡皮擦
        painter.drawLine(lastPoint, endPoint);
        break;
    case Line:
        painter.drawLine(firstPoint, endPoint);
        break;
    case GroupSelect:  // 添加GroupSelect case处理
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
    }
    update();
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

void PaintArea::saveState()
{
    if (undoStack.isEmpty() || undoStack.top() != image) {
        undoStack.push(image);
        if (undoStack.size() > 50) {
            undoStack.removeFirst();
        }
    }
    if (!undoStack.isEmpty() && undoStack.top() == image) return;

    undoStack.push(image.copy()); // 确保存储深拷贝
    if (undoStack.size() > 50) undoStack.removeFirst();

    redoStack.clear();
}
