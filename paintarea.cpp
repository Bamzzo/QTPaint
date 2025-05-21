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

    // 使用固定初始尺寸（后续会根据窗口自动调整）
    image = QImage(800, 600, QImage::Format_RGB32);
    image.fill(Qt::white);
    tempImage = image;
    drawing = false;
    currentShape = Freehand;
    penColor = Qt::black;
    penWidth = 3;
    undoStack.push(image);
}

void PaintArea::resizeEvent(QResizeEvent *event)
{
    // 先调整父类尺寸
    QWidget::resizeEvent(event);

    // 获取新的窗口尺寸
    QSize newSize = event->size();

    // 创建新尺寸图像
    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(Qt::white);

    // 将原有图像绘制到新图像中心
    QPainter painter(&newImage);
    painter.drawImage((newSize.width() - image.width())/2,
                      (newSize.height() - image.height())/2,
                      image);

    image = newImage;
    tempImage = image;
    update();
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

void PaintArea::undo()
{
    if (undoStack.size() > 1) {
        redoStack.push(undoStack.pop());

        // 获取当前窗口尺寸
        QSize currentSize = size();

        // 从历史记录恢复图像
        QImage restored = undoStack.top();

        // 调整历史图像到当前窗口尺寸
        QImage scaledImage(currentSize, QImage::Format_RGB32);
        scaledImage.fill(Qt::white);
        QPainter painter(&scaledImage);
        painter.drawImage((currentSize.width() - restored.width())/2,
                          (currentSize.height() - restored.height())/2,
                          restored);

        image = scaledImage;
        tempImage = image;
        update();
    }
}

void PaintArea::redo()
{
    if (!redoStack.isEmpty()) {
        // 获取当前窗口尺寸
        QSize currentSize = size();

        QImage restored = redoStack.top();

        // 调整图像到当前尺寸
        QImage scaledImage(currentSize, QImage::Format_RGB32);
        scaledImage.fill(Qt::white);
        QPainter painter(&scaledImage);
        painter.drawImage((currentSize.width() - restored.width())/2,
                          (currentSize.height() - restored.height())/2,
                          restored);

        undoStack.push(scaledImage);
        image = scaledImage;
        tempImage = image;
        update();
    }
}

void PaintArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), image, event->rect());
    if (drawing) {
        painter.drawImage(event->rect(), tempImage, event->rect());
    }
}

void PaintArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        firstPoint = lastPoint = event->pos();
        drawing = true;
        tempImage = image; // 保存当前状态到临时图像
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && drawing) {
        if (currentShape == Freehand || currentShape == Eraser) {
            drawShapeToImage(event->pos());
            lastPoint = event->pos();
        } else {
            tempImage = image;
            drawShapeToImage(event->pos());
        }
    }
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event)
{
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
        QImage newImage(newSize, QImage::Format_RGB32);
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
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(penColor, penWidth);
    QBrush brush(penColor);

    if (currentShape == Eraser) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        pen.setColor(Qt::transparent);
        brush.setColor(Qt::transparent);
    } else {
        painter.setBrush(brush);
    }

    painter.setPen(pen);

    QRect rect = QRect(firstPoint, endPoint).normalized();

    switch (currentShape) {
    case Freehand:
    case Eraser:
        painter.drawLine(lastPoint, endPoint);
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
    redoStack.clear();
}
