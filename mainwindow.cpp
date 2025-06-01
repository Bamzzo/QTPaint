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
#include <QStyle>
#include <QToolButton>
#include <QStatusBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentColor(Qt::black)
{
    setWindowTitle("绘图工具");
    resize(1256, 800);

    // 创建绘图区域
    paintArea = new PaintArea(this);
    setCentralWidget(paintArea);

    // 初始化UI组件
    createToolBar();
    createStatusBar();

    // 连接信号槽
    connect(paintArea, &PaintArea::cursorPositionChanged,
            this, &MainWindow::updateCursorPosition);
}

void MainWindow::createToolBar()
{
    // 主工具栏
    QToolBar *mainToolBar = addToolBar("主工具栏");
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(24, 24));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // 文件操作组

    QAction *openAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "  打开  ", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("打开图像文件");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    QAction *saveAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "  保存  ", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("保存当前绘图");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);


    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addSeparator();

    // 编辑操作组
    undoAction = new QAction(style()->standardIcon(QStyle::SP_ArrowBack), "  撤销  ", this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setStatusTip("撤销上一步操作");
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);

    redoAction = new QAction(style()->standardIcon(QStyle::SP_ArrowForward), "  重做  ", this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setStatusTip("重做上一步撤销的操作");
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);

    mainToolBar->addAction(undoAction);
    mainToolBar->addAction(redoAction);
    mainToolBar->addSeparator();

    // 绘图工具组
    QLabel *toolsLabel = new QLabel("   绘制类型:  ", this);
    toolsLabel->setStyleSheet("QLabel { color: #555; }");
    mainToolBar->addWidget(toolsLabel);

    shapeComboBox = new QComboBox(this);
    shapeComboBox->addItems({"自由绘制", "直线", "矩形", "椭圆", "箭头", "五角星", "菱形", "心形", "橡皮擦", "编组选择"});
    shapeComboBox->setFixedWidth(120);
    shapeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(shapeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeShape);

    mainToolBar->addWidget(shapeComboBox);
    mainToolBar->addSeparator();

    // 画笔设置组
    QLabel *penLabel = new QLabel("    画笔粗细:  ", this);
    penLabel->setStyleSheet("QLabel { color: #555; }");
    mainToolBar->addWidget(penLabel);

    sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(1, 100);
    sizeSpinBox->setValue(3);
    sizeSpinBox->setFixedWidth(60);
    sizeSpinBox->setSuffix(" ");
    sizeSpinBox->setToolTip("画笔粗细");
    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeBrushSize);

    mainToolBar->addWidget(sizeSpinBox);

    // 添加颜色选择标签
    QLabel *colorLabel = new QLabel("    画笔颜色:  ", this);
    colorLabel->setStyleSheet("QLabel { color: #555; }");
    mainToolBar->addWidget(colorLabel);

    colorBtn = new QPushButton(this);
    colorBtn->setFixedSize(32, 32);
    colorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 3px;")
                                .arg(currentColor.name()));
    colorBtn->setToolTip("选择颜色");
    connect(colorBtn, &QPushButton::clicked, this, &MainWindow::changeColor);

    mainToolBar->addWidget(colorBtn);
}

void MainWindow::createStatusBar()
{
    statusBar()->setStyleSheet("QStatusBar{background-color: #f0f0f0; border-top: 1px solid #ccc;}");

    cursorPosLabel = new QLabel("位置: 0, 0", this);
    cursorPosLabel->setStyleSheet("QLabel { padding: 2px 8px; }");

    shapeInfoLabel = new QLabel("工具: 自由绘制", this);
    shapeInfoLabel->setStyleSheet("QLabel { padding: 2px 8px; }");

    zoomLabel = new QLabel("缩放: 100%", this);
    zoomLabel->setStyleSheet("QLabel { padding: 2px 8px; }");

    statusBar()->addPermanentWidget(cursorPosLabel);
    statusBar()->addPermanentWidget(shapeInfoLabel);
    statusBar()->addPermanentWidget(zoomLabel);
}

void MainWindow::changeColor()
{
    QColorDialog colorDialog(this);
    colorDialog.setOption(QColorDialog::ShowAlphaChannel);
    colorDialog.setCurrentColor(currentColor);
    colorDialog.setWindowTitle("选择画笔颜色");

    if (colorDialog.exec() == QDialog::Accepted) {
        currentColor = colorDialog.currentColor();
        colorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 3px;")
                                    .arg(currentColor.name()));
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

    // 更新状态栏信息
    QString shapeName = shapeComboBox->itemText(index);
    shapeInfoLabel->setText("工具: " + shapeName);
}

void MainWindow::saveImage()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    "保存图片",
                                                    "",
                                                    "PNG图像 (*.png);;JPEG图像 (*.jpg *.jpeg);;BMP图像 (*.bmp)");

    if (!filePath.isEmpty()) {
        paintArea->saveImage(filePath);
        // 移除错误检查，因为saveImage()返回void
    }
}

void MainWindow::openImage()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "打开图片",
                                                    "",
                                                    "图像文件 (*.png *.jpg *.jpeg *.bmp)");

    if (!filePath.isEmpty()) {
        paintArea->loadImage(filePath);
        // 移除错误检查，因为loadImage()返回void
    }
}

void MainWindow::undo()
{
    paintArea->undo();
    update();
}

void MainWindow::redo()
{
    paintArea->redo();
    update();
}

void MainWindow::updateCursorPosition(const QPoint& pos)
{
    cursorPosLabel->setText(QString("位置: %1, %2").arg(pos.x()).arg(pos.y()));
}
