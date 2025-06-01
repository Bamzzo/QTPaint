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

class PaintArea : public QWidget
{
    Q_OBJECT
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
    void clearSelection();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPoint physicalToLogical(const QPoint &physicalPoint) const;
    QPoint logicalToPhysical(const QPoint &logicalPoint) const;
    QRect logicalToPhysical(const QRect &logicalRect) const;
    void updateScaleAndOffset();
    void saveState();

    QSize origImageSize;
    double scaleFactor;
    QPoint offset;

    QImage image;
    QImage originalImage;
    QImage tempImage;

    bool isSelecting;
    QRect selectionRect;
    QPoint selectionStart;
    QPoint selectionEnd;

    bool drawing;
    DrawShape currentShapeType;
    Shape* currentShape; // 当前正在绘制的形状

    QColor penColor;
    int penWidth;

    QStack<QImage> undoStack;
    QStack<QImage> redoStack;
signals:
    void cursorPositionChanged(const QPoint& pos);

};

#endif // PAINTAREA_H
