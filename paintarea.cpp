#include "paintarea.h"
#include <QPainterPath>
#include <QDebug>
#include <QResizeEvent>

PaintArea::PaintArea(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    image = QImage(800, 600, QImage::Format_RGB32);
    image.fill(Qt::white);
    tempImage = image;
    drawing = false;
    currentShape = Freehand;
    penColor = Qt::black;
    penWidth = 3;
}

void PaintArea::resizeEvent(QResizeEvent *event)
{
    resizeImage(event->size());
    QWidget::resizeEvent(event);
}

void PaintArea::setPenColor(const QColor &color)
{
    penColor = color;
    if (currentShape == Eraser) {
        penColor = Qt::white;
    }
}

void PaintArea::setPenWidth(int width)
{
    penWidth = width;
}

void PaintArea::setDrawShape(DrawShape shape)
{
    currentShape = shape;
    if (currentShape == Eraser) {
        penColor = Qt::white;
    }
}

void PaintArea::saveImage(const QString &fileName)
{
    image.save(fileName, "PNG");
}

void PaintArea::undo()
{
    if (!undoStack.isEmpty()) {
        redoStack.push(image);
        image = undoStack.pop();
        update();
    }
}

void PaintArea::redo()
{
    if (!redoStack.isEmpty()) {
        undoStack.push(image);
        image = redoStack.pop();
        update();
    }
}

void PaintArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, image, dirtyRect);
    painter.drawImage(dirtyRect, tempImage, dirtyRect);
}

void PaintArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        firstPoint = lastPoint = event->pos();
        drawing = true;
        tempImage = image;
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && drawing) {
        tempImage = image;
        drawShapeToImage(event->pos());
        lastPoint = event->pos();
    }
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && drawing) {
        drawShapeToImage(event->pos());
        drawing = false;
        saveState();
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
    QPen pen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    QRect rect = QRect(firstPoint, endPoint).normalized();

    switch (currentShape) {
    case Freehand:
        painter.drawLine(lastPoint, endPoint);
        image = tempImage;
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
        // 绘制箭头头部
        qreal arrowSize = penWidth * 4;
        QLineF line(endPoint, firstPoint);
        double angle = std::atan2(-line.dy(), line.dx());
        QPointF arrowP1 = endPoint + QPointF(std::sin(angle + M_PI/3) * arrowSize,
                                             std::cos(angle + M_PI/3) * arrowSize);
        QPointF arrowP2 = endPoint + QPointF(std::sin(angle + M_PI - M_PI/3) * arrowSize,
                                             std::cos(angle + M_PI - M_PI/3) * arrowSize);
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
    case Eraser:
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawLine(lastPoint, endPoint);
        image = tempImage;
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
        QPointF outerPoint = center + QPointF(radius * std::cos(angle), radius * std::sin(angle));
        if (i == 0)
            path.moveTo(outerPoint);
        else
            path.lineTo(outerPoint);

        angle += M_PI / 5;
        QPointF innerPoint = center + QPointF(radius/2 * std::cos(angle), radius/2 * std::sin(angle));
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
    undoStack.push(image);
    redoStack.clear();
    image = tempImage;
    update();
}
