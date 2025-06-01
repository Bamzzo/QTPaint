#ifndef SHAPES_H
#define SHAPES_H

#include <QPoint>
#include <QColor>
#include <QPainter>
#include <QRect>
#include <QVector>

// 抽象基类
class Shape {
public:
    Shape(const QPoint& start, const QColor& color, int width);
    virtual ~Shape() {}

    virtual void draw(QPainter& painter) const = 0;
    virtual QRect boundingRect() const;
    virtual void update(const QPoint& toPoint);
    virtual Shape* clone() const = 0;

protected:
    QPoint startPoint;
    QPoint endPoint;
    QColor penColor;
    int penWidth;
};

// 具体图形类
class LineShape : public Shape {
public:
    LineShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

class RectangleShape : public Shape {
public:
    RectangleShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

class EllipseShape : public Shape {
public:
    EllipseShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

class ArrowShape : public Shape {
public:
    ArrowShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

class StarShape : public Shape {
public:
    StarShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

class DiamondShape : public Shape {
public:
    DiamondShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

class HeartShape : public Shape {
public:
    HeartShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;
    Shape* clone() const override;
};

// 路径类（用于自由绘制和橡皮擦）
class PathShape : public Shape {
public:
    PathShape(const QPoint& start, const QColor& color, int width, bool isEraser = false);
    void draw(QPainter& painter) const override;
    void update(const QPoint& toPoint) override;
    QRect boundingRect() const override;
    Shape* clone() const override;

private:
    QVector<QPoint> points;
    bool eraser;
};

#endif // SHAPES_H
