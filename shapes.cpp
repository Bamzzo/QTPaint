#include "shapes.h"
#include <cmath>
#include <QPainterPath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Shape 基类实现
Shape::Shape(const QPoint& start, const QColor& color, int width)
    : startPoint(start), endPoint(start), penColor(color), penWidth(width) {}

QRect Shape::boundingRect() const {
    return QRect(startPoint, endPoint).normalized();
}

void Shape::update(const QPoint& toPoint) {
    endPoint = toPoint;
}

// LineShape 实现
LineShape::LineShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void LineShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawLine(startPoint, endPoint);
}

Shape* LineShape::clone() const {
    return new LineShape(*this);
}

// RectangleShape 实现
RectangleShape::RectangleShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void RectangleShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(QBrush(penColor));
    painter.drawRect(QRect(startPoint, endPoint).normalized());
}

Shape* RectangleShape::clone() const {
    return new RectangleShape(*this);
}

// EllipseShape 实现
EllipseShape::EllipseShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void EllipseShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(penColor));
    painter.drawEllipse(QRect(startPoint, endPoint).normalized());
}

Shape* EllipseShape::clone() const {
    return new EllipseShape(*this);
}

// ArrowShape 实现
ArrowShape::ArrowShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void ArrowShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    // 绘制主线条
    painter.drawLine(startPoint, endPoint);

    // 绘制箭头
    qreal arrowSize = penWidth * 4;
    QLineF line(endPoint, startPoint);
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
}

Shape* ArrowShape::clone() const {
    return new ArrowShape(*this);
}

// StarShape 实现
StarShape::StarShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void StarShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(penColor));

    QRect rect = QRect(startPoint, endPoint).normalized();
    qreal radius = qMin(rect.width(), rect.height()) / 2;
    QPoint center = rect.center();

    QPainterPath path;
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

Shape* StarShape::clone() const {
    return new StarShape(*this);
}

// DiamondShape 实现
DiamondShape::DiamondShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void DiamondShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(penColor));

    QRect rect = QRect(startPoint, endPoint).normalized();
    QPolygon diamond;
    diamond << QPoint(rect.center().x(), rect.top())
            << QPoint(rect.right(), rect.center().y())
            << QPoint(rect.center().x(), rect.bottom())
            << QPoint(rect.left(), rect.center().y());
    painter.drawPolygon(diamond);
}

Shape* DiamondShape::clone() const {
    return new DiamondShape(*this);
}

// HeartShape 实现
HeartShape::HeartShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

void HeartShape::draw(QPainter& painter) const {
    QRect rect = QRect(startPoint, endPoint).normalized();
    qreal scale = qMin(rect.width(), rect.height()) / 100;
    QPoint center = rect.center();

    QPainterPath path;
    path.moveTo(center.x(), center.y() + 25*scale);
    path.cubicTo(center.x() + 45*scale, center.y() - 55*scale,
                 center.x() + 95*scale, center.y() - 35*scale,
                 center.x(), center.y() + 45*scale);
    path.cubicTo(center.x() - 95*scale, center.y() - 35*scale,
                 center.x() - 45*scale, center.y() - 55*scale,
                 center.x(), center.y() + 25*scale);

    painter.fillPath(path, penColor);
}

Shape* HeartShape::clone() const {
    return new HeartShape(*this);
}

// PathShape 实现（用于自由绘制和橡皮擦）
PathShape::PathShape(const QPoint& start, const QColor& color, int width, bool isEraser)
    : Shape(start, color, width), eraser(isEraser) {}

void PathShape::draw(QPainter& painter) const {
    if (points.empty()) return;

    QPen pen(eraser ? Qt::white : penColor, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    for (int i = 1; i < points.size(); ++i) {
        painter.drawLine(points[i-1], points[i]);
    }
}

void PathShape::update(const QPoint& toPoint) {
    points.append(toPoint);
    endPoint = toPoint;
}

QRect PathShape::boundingRect() const {
    if (points.empty()) return QRect();

    int minX = points[0].x();
    int minY = points[0].y();
    int maxX = minX;
    int maxY = minY;

    for (const QPoint& p : points) {
        if (p.x() < minX) minX = p.x();
        if (p.y() < minY) minY = p.y();
        if (p.x() > maxX) maxX = p.x();
        if (p.y() > maxY) maxY = p.y();
    }

    return QRect(QPoint(minX, minY), QPoint(maxX, maxY))
        .adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

Shape* PathShape::clone() const {
    PathShape* clone = new PathShape(startPoint, penColor, penWidth, eraser);
    clone->points = points;
    clone->endPoint = endPoint;
    return clone;
}
