#ifndef PAINTAREA_H
#define PAINTAREA_H

#include <QPaintEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QStack>
#include <QPoint>
#include "shapes.h"

/**
 * @brief 绘图区域类，负责实际的绘图功能和图像处理
 */
class PaintArea : public QWidget
{
    Q_OBJECT  // Qt元对象系统宏

public:
    /**
     * @brief 调整图像大小
     * @param newSize 新的尺寸
     */
    void resizeImage(const QSize &newSize);

    /**
     * @brief 绘图形状枚举
     */
    enum DrawShape {
        Freehand,      // 0:自由绘制
        Line,          // 1:直线
        Rectangle,     // 2:矩形
        Ellipse,       // 3:椭圆
        Arrow,         // 4:箭头
        Star,          // 5:五角星
        Diamond,       // 6:菱形
        Heart,         // 7:心形
        Eraser,        // 8:橡皮擦
        GroupSelect    // 9:编组选择
    };

    // 构造函数和功能方法
    explicit PaintArea(QWidget *parent = nullptr);
    void setPenColor(const QColor &color);  // 设置画笔颜色
    void setPenWidth(int width);  // 设置画笔宽度
    void setDrawShape(DrawShape shape);  // 设置绘图形状
    void saveImage(const QString &fileName);  // 保存图像到文件
    void loadImage(const QString &fileName);  // 从文件加载图像
    void undo();  // 撤销操作
    void redo();  // 重做操作
    void clearSelection();  // 清除选择

protected:
    // 重写的Qt事件处理函数
    void paintEvent(QPaintEvent *event) override;  // 绘制事件
    void mousePressEvent(QMouseEvent *event) override;  // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent *event) override;  // 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *event) override;  // 鼠标释放事件
    void resizeEvent(QResizeEvent *event) override;  // 大小改变事件

private:
    // 坐标转换辅助函数
    QPoint physicalToLogical(const QPoint &physicalPoint) const;  // 物理坐标转逻辑坐标
    QPoint logicalToPhysical(const QPoint &logicalPoint) const;  // 逻辑坐标转物理坐标
    QRect logicalToPhysical(const QRect &logicalRect) const;  // 逻辑矩形转物理矩形
    void updateScaleAndOffset();  // 更新缩放比例和偏移量
    void saveState();  // 保存当前状态到撤销栈

    // 图像相关成员
    QSize origImageSize;  // 原始图像尺寸
    double scaleFactor;  // 缩放因子
    QPoint offset;  // 偏移量

    QImage image;  // 主图像
    QImage originalImage;  // 原始图像(用于缩放)
    QImage tempImage;  // 临时图像(用于绘制过程中的预览)

    // 选择相关成员
    bool isSelecting;  // 是否正在选择
    QRect selectionRect;  // 选择矩形
    QPoint selectionStart;  // 选择开始点
    QPoint selectionEnd;  // 选择结束点

    // 绘图相关成员
    bool drawing;  // 是否正在绘图
    DrawShape currentShapeType;  // 当前绘图形状类型
    Shape* currentShape;  // 当前正在绘制的形状对象

    QColor penColor;  // 画笔颜色
    int penWidth;  // 画笔宽度

    // 撤销/重做栈
    QStack<QImage> undoStack;  // 撤销栈
    QStack<QImage> redoStack;  // 重做栈

signals:
    /**
     * @brief 光标位置改变信号
     * @param pos 新的光标位置
     */
    void cursorPositionChanged(const QPoint& pos);
};

#endif // PAINTAREA_H
