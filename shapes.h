#ifndef SHAPES_H
#define SHAPES_H

#include <QPoint>
#include <QColor>
#include <QPainter>
#include <QRect>
#include <QVector>

/**
 * @brief 形状基类，定义所有形状的通用接口和属性
 */
class Shape {
public:
    /**
     * @brief 构造函数
     * @param start 起点坐标
     * @param color 颜色
     * @param width 画笔宽度
     */
    Shape(const QPoint& start, const QColor& color, int width);
    virtual ~Shape() {}  // 虚析构函数

    // 虚函数
    virtual void draw(QPainter& painter) const = 0;  // 绘制形状
    virtual QRect boundingRect() const;  // 计算边界矩形
    virtual void update(const QPoint& toPoint);  // 更新终点坐标
    virtual Shape* clone() const = 0;  // 克隆形状

protected:
    QPoint startPoint;  // 起点坐标
    QPoint endPoint;  // 终点坐标
    QColor penColor;  // 画笔颜色
    int penWidth;  // 画笔宽度
};

/**
 * @brief 直线形状类
 */
class LineShape : public Shape {
public:
    LineShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制直线
    Shape* clone() const override;  // 克隆直线
};

/**
 * @brief 矩形形状类
 */
class RectangleShape : public Shape {
public:
    RectangleShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制矩形
    Shape* clone() const override;  // 克隆矩形
};

/**
 * @brief 椭圆形状类
 */
class EllipseShape : public Shape {
public:
    EllipseShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制椭圆
    Shape* clone() const override;  // 克隆椭圆
};

/**
 * @brief 箭头形状类
 */
class ArrowShape : public Shape {
public:
    ArrowShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制箭头
    Shape* clone() const override;  // 克隆箭头
};

/**
 * @brief 五角星形状类
 */
class StarShape : public Shape {
public:
    StarShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制五角星
    Shape* clone() const override;  // 克隆五角星
};

/**
 * @brief 菱形形状类
 */
class DiamondShape : public Shape {
public:
    DiamondShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制菱形
    Shape* clone() const override;  // 克隆菱形
};

/**
 * @brief 心形形状类
 */
class HeartShape : public Shape {
public:
    HeartShape(const QPoint& start, const QColor& color, int width);
    void draw(QPainter& painter) const override;  // 绘制心形
    Shape* clone() const override;  // 克隆心形
};

/**
 * @brief 路径形状类，用于自由绘制和橡皮擦
 */
class PathShape : public Shape {
public:
    /**
     * @brief 构造函数
     * @param start 起点
     * @param color 颜色
     * @param width 宽度
     * @param isEraser 是否为橡皮擦
     */
    PathShape(const QPoint& start, const QColor& color, int width, bool isEraser = false);
    void draw(QPainter& painter) const override;  // 绘制路径
    void update(const QPoint& toPoint) override;  // 更新路径点
    QRect boundingRect() const override;  // 计算路径边界矩形
    Shape* clone() const override;  // 克隆路径

private:
    QVector<QPoint> points;  // 路径点集合
    bool eraser;  // 是否为橡皮擦模式
};

#endif // SHAPES_H
