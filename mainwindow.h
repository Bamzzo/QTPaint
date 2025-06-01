#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
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
    void openImage();
    void undo();
    void redo();
    void updateCursorPosition(const QPoint& pos);

private:
    void createToolBar();
    void createStatusBar();
    QPushButton* createToolButton(const QString& text, const QString& tooltip = "");

    PaintArea *paintArea;
    QColor currentColor;
    QPushButton *colorBtn;
    QSpinBox *sizeSpinBox;
    QComboBox *shapeComboBox;
    QAction *undoAction;
    QAction *redoAction;

    // Status bar widgets
    QLabel *cursorPosLabel;
    QLabel *shapeInfoLabel;
    QLabel *zoomLabel;
};
#endif // MAINWINDOW_H
