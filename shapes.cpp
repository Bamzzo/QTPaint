#include "shapes.h"  // 包含形状类的头文件
#include <cmath>     // 包含数学函数库
#include <QPainterPath>  // Qt绘图路径类

// 如果系统没有定义M_PI(π的值)，则手动定义
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ========== Shape 基类实现 ========== */

// Shape构造函数
// 参数：start - 起始点坐标；color - 画笔颜色；width - 画笔宽度
Shape::Shape(const QPoint& start, const QColor& color, int width)
    : startPoint(start), endPoint(start), penColor(color), penWidth(width) {}

// 获取形状的边界矩形
QRect Shape::boundingRect() const {
    // 根据起点和终点创建矩形，并返回规范化后的矩形(确保左上角在左上方)
    return QRect(startPoint, endPoint).normalized();
}

// 更新形状的终点坐标
void Shape::update(const QPoint& toPoint) {
    endPoint = toPoint;  // 将终点更新为指定点
}

/* ========== LineShape 直线实现 ========== */

// LineShape构造函数，调用基类构造函数
LineShape::LineShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制直线
void LineShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);  // 创建指定颜色和宽度的画笔
    pen.setCapStyle(Qt::RoundCap); // 设置线帽为圆角
    painter.setPen(pen);           // 将画笔设置给绘制器
    painter.drawLine(startPoint, endPoint);  // 绘制从起点到终点的直线
}

// 克隆直线对象
Shape* LineShape::clone() const {
    return new LineShape(*this);  // 返回当前对象的副本
}

/* ========== RectangleShape 矩形实现 ========== */

// 矩形构造函数
RectangleShape::RectangleShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制矩形
void RectangleShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);  // 创建画笔
    pen.setJoinStyle(Qt::RoundJoin); // 设置连接点为圆角
    painter.setPen(pen);            // 设置画笔
    painter.setBrush(QBrush(penColor)); // 设置填充画刷
    // 绘制规范化后的矩形(确保宽度和高度为正)
    painter.drawRect(QRect(startPoint, endPoint).normalized());
}

// 克隆矩形对象
Shape* RectangleShape::clone() const {
    return new RectangleShape(*this);
}

/* ========== EllipseShape 椭圆实现 ========== */

// 椭圆构造函数
EllipseShape::EllipseShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制椭圆
void EllipseShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);  // 创建画笔
    painter.setPen(pen);           // 设置画笔
    painter.setBrush(QBrush(penColor)); // 设置填充画刷
    // 在规范化后的矩形内绘制椭圆
    painter.drawEllipse(QRect(startPoint, endPoint).normalized());
}

// 克隆椭圆对象
Shape* EllipseShape::clone() const {
    return new EllipseShape(*this);
}

/* ========== ArrowShape 箭头实现 ========== */

// 箭头构造函数
ArrowShape::ArrowShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制箭头
void ArrowShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);  // 创建画笔
    pen.setCapStyle(Qt::RoundCap); // 设置线帽为圆角
    painter.setPen(pen);           // 设置画笔

    // 1. 绘制主线条(箭头杆)
    painter.drawLine(startPoint, endPoint);

    // 2. 绘制箭头头部
    qreal arrowSize = penWidth * 4;  // 箭头大小与线宽成正比
    QLineF line(endPoint, startPoint); // 创建从终点到起点的线(用于计算角度)
    double angle = std::atan2(-line.dy(), line.dx()); // 计算线的角度(弧度)

    // 计算箭头两个分支点的位置
    QPointF arrowP1 = endPoint + QPointF(
                          std::sin(angle + M_PI/3) * arrowSize,  // 第一个分支点x坐标
                          std::cos(angle + M_PI/3) * arrowSize   // 第一个分支点y坐标
                          );
    QPointF arrowP2 = endPoint + QPointF(
                          std::sin(angle + M_PI - M_PI/3) * arrowSize, // 第二个分支点x坐标
                          std::cos(angle + M_PI - M_PI/3) * arrowSize  // 第二个分支点y坐标
                          );

    // 绘制箭头两个分支线
    painter.drawLine(endPoint, arrowP1);
    painter.drawLine(endPoint, arrowP2);
}

// 克隆箭头对象
Shape* ArrowShape::clone() const {
    return new ArrowShape(*this);
}

/* ========== StarShape 五角星实现 ========== */

// 五角星构造函数
StarShape::StarShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制五角星
void StarShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);  // 创建画笔
    painter.setPen(pen);           // 设置画笔
    painter.setBrush(QBrush(penColor)); // 设置填充画刷

    QRect rect = QRect(startPoint, endPoint).normalized(); // 获取规范化矩形
    qreal radius = qMin(rect.width(), rect.height()) / 2;  // 计算外接圆半径(取宽高较小者的一半)
    QPoint center = rect.center();  // 获取中心点

    QPainterPath path;  // 创建绘图路径
    // 绘制五角星的五个顶点
    for (int i = 0; i < 5; ++i) {
        // 计算外顶点角度(均匀分布在圆周上，从12点钟方向开始)
        qreal angle = 2 * M_PI * i / 5 - M_PI/2;
        // 计算外顶点坐标
        QPointF outerPoint = center + QPointF(radius * cos(angle), radius * sin(angle));
        if (i == 0) {
            path.moveTo(outerPoint);  // 第一个点移动画笔
        } else {
            path.lineTo(outerPoint);  // 其他点画线连接
        }

        // 计算内顶点角度(在外顶点之间)
        angle += M_PI / 5;
        // 计算内顶点坐标(半径为外半径的一半)
        QPointF innerPoint = center + QPointF(radius/2 * cos(angle), radius/2 * sin(angle));
        path.lineTo(innerPoint);  // 画线到内顶点
    }
    path.closeSubpath();  // 闭合路径
    painter.drawPath(path);  // 绘制完整路径
}

// 克隆五角星对象
Shape* StarShape::clone() const {
    return new StarShape(*this);
}

/* ========== DiamondShape 菱形实现 ========== */

// 菱形构造函数
DiamondShape::DiamondShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制菱形
void DiamondShape::draw(QPainter& painter) const {
    QPen pen(penColor, penWidth);  // 创建画笔
    painter.setPen(pen);           // 设置画笔
    painter.setBrush(QBrush(penColor)); // 设置填充画刷

    QRect rect = QRect(startPoint, endPoint).normalized(); // 获取规范化矩形
    QPolygon diamond;  // 创建多边形
    // 添加菱形的四个顶点(上、右、下、左)
    diamond << QPoint(rect.center().x(), rect.top())      // 上顶点
            << QPoint(rect.right(), rect.center().y())    // 右顶点
            << QPoint(rect.center().x(), rect.bottom())   // 下顶点
            << QPoint(rect.left(), rect.center().y());    // 左顶点
    painter.drawPolygon(diamond);  // 绘制菱形多边形
}

// 克隆菱形对象
Shape* DiamondShape::clone() const {
    return new DiamondShape(*this);
}

/* ========== HeartShape 心形实现 ========== */

// 心形构造函数
HeartShape::HeartShape(const QPoint& start, const QColor& color, int width)
    : Shape(start, color, width) {}

// 绘制心形
void HeartShape::draw(QPainter& painter) const {
    QRect rect = QRect(startPoint, endPoint).normalized(); // 获取规范化矩形
    qreal scale = qMin(rect.width(), rect.height()) / 100; // 计算缩放比例(基于100像素基准)
    QPoint center = rect.center();  // 获取中心点

    QPainterPath path;  // 创建绘图路径
    // 从心形底部开始
    path.moveTo(center.x(), center.y() + 25*scale);
    // 绘制右侧贝塞尔曲线
    path.cubicTo(center.x() + 45*scale, center.y() - 55*scale,  // 控制点1
                 center.x() + 95*scale, center.y() - 35*scale,   // 控制点2
                 center.x(), center.y() + 45*scale);            // 终点
    // 绘制左侧贝塞尔曲线
    path.cubicTo(center.x() - 95*scale, center.y() - 35*scale,  // 控制点1
                 center.x() - 45*scale, center.y() - 55*scale,  // 控制点2
                 center.x(), center.y() + 25*scale);            // 终点

    painter.fillPath(path, penColor);  // 填充心形路径
}

// 克隆心形对象
Shape* HeartShape::clone() const {
    return new HeartShape(*this);
}

/* ========== PathShape 路径实现(用于自由绘制和橡皮擦) ========== */

// 路径构造函数
// 参数：isEraser - 是否为橡皮擦模式
PathShape::PathShape(const QPoint& start, const QColor& color, int width, bool isEraser)
    : Shape(start, color, width), eraser(isEraser) {}

// 绘制路径
void PathShape::draw(QPainter& painter) const {
    if (points.empty()) return;  // 如果没有点则直接返回

    // 设置画笔：橡皮擦模式使用白色，否则使用指定颜色
    QPen pen(eraser ? Qt::white : penColor, penWidth);
    pen.setCapStyle(Qt::RoundCap);  // 设置圆角线帽
    painter.setPen(pen);            // 设置画笔

    // 连接所有点形成路径
    for (int i = 1; i < points.size(); ++i) {
        painter.drawLine(points[i-1], points[i]);
    }
}

// 更新路径，添加新点
void PathShape::update(const QPoint& toPoint) {
    points.append(toPoint);  // 将新点添加到路径中
    endPoint = toPoint;      // 更新终点
}

// 获取路径的边界矩形
QRect PathShape::boundingRect() const {
    if (points.empty()) return QRect();  // 如果没有点则返回空矩形

    // 初始化最小/最大坐标
    int minX = points[0].x();
    int minY = points[0].y();
    int maxX = minX;
    int maxY = minY;

    // 遍历所有点找到边界
    for (const QPoint& p : points) {
        if (p.x() < minX) minX = p.x();
        if (p.y() < minY) minY = p.y();
        if (p.x() > maxX) maxX = p.x();
        if (p.y() > maxY) maxY = p.y();
    }

    // 返回包含所有点的矩形，并考虑线宽向外扩展
    return QRect(QPoint(minX, minY), QPoint(maxX, maxY))
        .adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

// 克隆路径对象
Shape* PathShape::clone() const {
    PathShape* clone = new PathShape(startPoint, penColor, penWidth, eraser);
    clone->points = points;    // 复制所有点
    clone->endPoint = endPoint; // 复制终点
    return clone;
}
