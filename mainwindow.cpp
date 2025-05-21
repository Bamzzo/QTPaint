#include "mainwindow.h"
#include <QApplication>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QFileDialog>
#include <QColorDialog>
#include <QToolBar>
#include <QAction>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentColor(Qt::black)
{
    qDebug() << "MainWindow constructor called";

    setWindowTitle("Paint Program");
    resize(1200, 800);

    // 创建绘图区域
    paintArea = new PaintArea(this);
    if (!paintArea) {
        qCritical() << "Failed to create PaintArea";
        return;
    }
    setCentralWidget(paintArea);
    qDebug() << "PaintArea created and set as central widget";

    // 初始化颜色按钮
    colorBtn = new QPushButton(this);
    if (!colorBtn) {
        qCritical() << "Failed to create color button";
        return;
    }
    colorBtn->setStyleSheet("background-color: black");
    colorBtn->setFixedSize(32, 32);
    connect(colorBtn, &QPushButton::clicked, this, &MainWindow::changeColor);
    qDebug() << "Color button created and connected";

    // 画笔大小设置
    sizeSpinBox = new QSpinBox(this);
    if (!sizeSpinBox) {
        qCritical() << "Failed to create size spin box";
        return;
    }
    sizeSpinBox->setRange(1, 50);
    sizeSpinBox->setValue(3);
    sizeSpinBox->setFixedWidth(60); // 设置固定宽度
    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeBrushSize);
    qDebug() << "Size spin box created and connected";

    // 形状选择
    shapeComboBox = new QComboBox(this);
    if (!shapeComboBox) {
        qCritical() << "Failed to create shape combo box";
        return;
    }
    shapeComboBox->addItems({"自由绘制", "直线", "矩形", "椭圆", "箭头", "五角星", "菱形", "心形", "橡皮擦"});
    shapeComboBox->setFixedWidth(120); // 设置固定宽度
    connect(shapeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeShape);
    qDebug() << "Shape combo box created and connected";

    // 撤销/重做按钮
    undoAction = new QAction("撤销", this);
    if (!undoAction) {
        qCritical() << "Failed to create undo action";
        return;
    }
    redoAction = new QAction("重做", this);
    if (!redoAction) {
        qCritical() << "Failed to create redo action";
        return;
    }
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);
    qDebug() << "Undo and redo actions created and connected";

    // 创建工具栏
    createToolBar();

    qDebug() << "MainWindow constructor finished";
}

void MainWindow::createToolBar()
{
    qDebug() << "Creating toolbar";

    QToolBar *toolBar = addToolBar("工具");
    if (!toolBar) {
        qCritical() << "Failed to create toolbar";
        return;
    }

    // 工具栏左侧部分
    QLabel *sizeLabel = new QLabel(" 大小:", this);
    if (!sizeLabel) {
        qCritical() << "Failed to create size label";
        return;
    }
    toolBar->addWidget(sizeLabel);
    qDebug() << "Size label added to toolbar";

    toolBar->addWidget(sizeSpinBox);
    qDebug() << "Size spin box added to toolbar";

    QLabel *shapeLabel = new QLabel(" 形状:", this);
    if (!shapeLabel) {
        qCritical() << "Failed to create shape label";
        return;
    }
    toolBar->addWidget(shapeLabel);
    qDebug() << "Shape label added to toolbar";

    toolBar->addWidget(shapeComboBox);
    qDebug() << "Shape combo box added to toolbar";

    toolBar->addSeparator();
    qDebug() << "Separator added to toolbar";

    toolBar->addWidget(colorBtn);
    qDebug() << "Color button added to toolbar";

    QPushButton *saveBtn = new QPushButton("保存", this);
    if (!saveBtn) {
        qCritical() << "Failed to create save button in toolbar";
        return;
    }
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveImage);
    toolBar->addWidget(saveBtn);
    qDebug() << "Save button added to toolbar";

    // 工具栏右侧部分
    toolBar->addSeparator();
    qDebug() << "Separator added to toolbar";

    toolBar->addAction(undoAction);
    qDebug() << "Undo action added to toolbar";

    toolBar->addAction(redoAction);
    qDebug() << "Redo action added to toolbar";

    qDebug() << "Toolbar created";
}

void MainWindow::changeColor()
{
    QColor newColor = QColorDialog::getColor(currentColor);
    if (newColor.isValid()) {
        currentColor = newColor;
        colorBtn->setStyleSheet(QString("background-color: %1").arg(currentColor.name()));
        paintArea->setPenColor(currentColor);
    }
}

void MainWindow::changeBrushSize(int size)
{
    paintArea->setPenWidth(size);
}

void MainWindow::changeShape(int index)
{
    paintArea->setDrawShape(static_cast<PaintArea::DrawShape>(index));
}

void MainWindow::saveImage()
{
    qDebug() << "Saving image";

    QString filePath = QFileDialog::getSaveFileName(this, "保存图片", "", "PNG文件(*.png)");
    if (!filePath.isEmpty()) {
        paintArea->saveImage(filePath);
    }

    qDebug() << "Image saved";
}

void MainWindow::undo()
{
    qDebug() << "Undoing last action";

    paintArea->undo();
    update(); // 强制更新界面
}

void MainWindow::redo()
{
    qDebug() << "Redoing last undone action";

    paintArea->redo();
    update(); // 强制更新界面
}
