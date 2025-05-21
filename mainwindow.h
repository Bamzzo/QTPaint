#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QToolBar>
#include "paintarea.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void changeColor();
    void changeBrushSize(int size);
    void changeShape(int index);
    void saveImage();
    void undo();
    void redo();

private:
    PaintArea *paintArea;
    QColor currentColor;
    QPushButton *colorBtn;
    QSpinBox *sizeSpinBox;
    QComboBox *shapeComboBox;
    QAction *undoAction;
    QAction *redoAction;

    void createToolBar();
};
#endif // MAINWINDOW_H
