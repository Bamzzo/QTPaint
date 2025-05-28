#ifndef PAINTAREA_H
#define PAINTAREA_H
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QStack>
#include <QPoint>

class PaintArea : public QWidget
{
    Q_OBJECT
    // paintarea.h
public:
    void resizeImage(const QSize &newSize);

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

    explicit PaintArea(QWidget *parent = nullptr);
    void setPenColor(const QColor &color);
    void setPenWidth(int width);
    void setDrawShape(DrawShape shape);
    void saveImage(const QString &fileName);
    void loadImage(const QString &fileName);
    void undo();
    void redo();
    // 在PaintArea类声明中添加
    void clearSelection();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    // 添加以下成员变量
    // paintarea.h
private:
    QPoint physicalToLogical(const QPoint &physicalPoint) const;
    QPoint logicalToPhysical(const QPoint &logicalPoint) const;
    QRect logicalToPhysical(const QRect &logicalRect) const;
    void updateScaleAndOffset();

    QSize origImageSize;       // 原始图片尺寸
    double scaleFactor;        // 当前缩放比例
    QPoint offset;             // 图片偏移量（用于居中）
    void drawShapeToImage(const QPoint &endPoint);
    void drawStar(QPainter &painter, const QRect &rect);
    void drawDiamond(QPainter &painter, const QRect &rect);
    void drawHeart(QPainter &painter, const QRect &rect);

    QImage image;
    QImage originalImage;
    QImage tempImage;
    QPoint lastPoint;
    QPoint firstPoint;
    QPoint selectionStart;
    QPoint selectionEnd;
    bool isSelecting = false;
    QRect selectionRect;
    bool drawing;
    DrawShape currentShape;
    QColor penColor;
    int penWidth;
    void saveState();
    QStack<QImage> undoStack;
    QStack<QImage> redoStack;


};
#endif // PAINTAREA_H
