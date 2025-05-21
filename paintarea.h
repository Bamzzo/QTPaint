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
        Freehand, Line, Rectangle, Ellipse, Arrow,
        Star, Diamond, Heart, Eraser
    };

    explicit PaintArea(QWidget *parent = nullptr);
    void setPenColor(const QColor &color);
    void setPenWidth(int width);
    void setDrawShape(DrawShape shape);
    void saveImage(const QString &fileName);
    void undo();
    void redo();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    void drawShapeToImage(const QPoint &endPoint);
    void drawStar(QPainter &painter, const QRect &rect);
    void drawDiamond(QPainter &painter, const QRect &rect);
    void drawHeart(QPainter &painter, const QRect &rect);

    QImage image;
    QImage tempImage;
    QPoint lastPoint;
    QPoint firstPoint;
    bool drawing;
    DrawShape currentShape;
    QColor penColor;
    int penWidth;

    QStack<QImage> undoStack;
    QStack<QImage> redoStack;

    void saveState();
};
#endif // PAINTAREA_H
