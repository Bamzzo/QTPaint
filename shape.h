#ifndef SHAPE_H
#define SHAPE_H

#include <QPainter>
#include <QPoint>

/**
 * @brief 形状基类，定义所有形状的通用接口
 */
class Shape {
public:
    virtual ~Shape() = default;  // 虚析构函数

    // 纯虚函数，子类必须实现
    virtual void draw(QPainter& painter) const = 0;  // 绘制形状
    virtual QRect boundingRect() const = 0;  // 获取边界矩形
    virtual void update(const QPoint& toPoint) = 0;  // 更新形状到指定点
    virtual Shape* clone() const = 0;  // 克隆形状

    // 公共属性
    QColor color;  // 形状颜色
    int penWidth;  // 画笔宽度
    QPoint startPoint;  // 起点
    QPoint endPoint;  // 终点
};

#endif // SHAPE_H
