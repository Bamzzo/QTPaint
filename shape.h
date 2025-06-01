// shape.h
#ifndef SHAPE_H
#define SHAPE_H

#include <QPainter>
#include <QPoint>

class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw(QPainter& painter) const = 0;
    virtual QRect boundingRect() const = 0;
    virtual void update(const QPoint& toPoint) = 0;
    virtual Shape* clone() const = 0;

    QColor color;
    int penWidth;
    QPoint startPoint;
    QPoint endPoint;
};

#endif // SHAPE_H
