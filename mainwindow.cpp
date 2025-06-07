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

// 主窗口构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentColor(Qt::black)  // 初始化父类和当前颜色(默认为黑色)
{
    setWindowTitle("绘图工具");  // 设置窗口标题
    resize(1256, 800);          // 设置窗口初始大小

    // 创建绘图区域
    paintArea = new PaintArea(this);  // 创建绘图区域对象
    setCentralWidget(paintArea);      // 将绘图区域设置为中心窗口部件

    // 初始化UI组件
    createToolBar();    // 创建工具栏
    createStatusBar();  // 创建状态栏

    // 连接信号槽：当绘图区域光标位置改变时，更新状态栏显示
    connect(paintArea, &PaintArea::cursorPositionChanged,
            this, &MainWindow::updateCursorPosition);
}

// 创建工具栏函数
void MainWindow::createToolBar()
{
    // 主工具栏
    QToolBar *mainToolBar = addToolBar("主工具栏");  // 创建工具栏
    mainToolBar->setMovable(false);                  // 禁止工具栏移动
    mainToolBar->setIconSize(QSize(24, 24));         // 设置工具栏图标大小
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);  // 设置按钮样式(图标在上，文字在下)

    // 文件操作组 ==============================================

    // 创建"打开"动作
    QAction *openAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "  打开  ", this);
    openAction->setShortcut(QKeySequence::Open);  // 设置快捷键(Ctrl+O)
    openAction->setStatusTip("打开图像文件");     // 设置状态栏提示
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);  // 连接信号槽

    // 创建"保存"动作
    QAction *saveAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "  保存  ", this);
    saveAction->setShortcut(QKeySequence::Save);  // 设置快捷键(Ctrl+S)
    saveAction->setStatusTip("保存当前绘图");     // 设置状态栏提示
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);  // 连接信号槽

    // 将动作添加到工具栏
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addSeparator();  // 添加分隔线

    // 编辑操作组 ==============================================
    // 创建"撤销"动作
    undoAction = new QAction(style()->standardIcon(QStyle::SP_ArrowBack), "  撤销  ", this);
    undoAction->setShortcut(QKeySequence::Undo);  // 设置快捷键(Ctrl+Z)
    undoAction->setStatusTip("撤销上一步操作");   // 设置状态栏提示
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);  // 连接信号槽

    // 创建"重做"动作
    redoAction = new QAction(style()->standardIcon(QStyle::SP_ArrowForward), "  重做  ", this);
    redoAction->setShortcut(QKeySequence::Redo);  // 设置快捷键(Ctrl+Y)
    redoAction->setStatusTip("重做上一步撤销的操作");  // 设置状态栏提示
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);  // 连接信号槽

    // 将动作添加到工具栏
    mainToolBar->addAction(undoAction);
    mainToolBar->addAction(redoAction);
    mainToolBar->addSeparator();  // 添加分隔线

    // 绘图工具组 ==============================================
    // 添加"绘制类型"标签
    QLabel *toolsLabel = new QLabel("   绘制类型:  ", this);
    toolsLabel->setStyleSheet("QLabel { color: #555; }");  // 设置标签样式(灰色文字)
    mainToolBar->addWidget(toolsLabel);

    // 创建形状选择下拉框
    shapeComboBox = new QComboBox(this);
    shapeComboBox->addItems({"自由绘制", "直线", "矩形", "椭圆", "箭头", "五角星", "菱形", "心形", "橡皮擦", "编组选择"});  // 添加各种绘图工具选项
    shapeComboBox->setFixedWidth(120);  // 设置固定宽度
    shapeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);  // 设置大小调整策略
    // 连接下拉框选择变化信号到槽函数
    connect(shapeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeShape);

    mainToolBar->addWidget(shapeComboBox);  // 将下拉框添加到工具栏
    mainToolBar->addSeparator();            // 添加分隔线

    // 画笔设置组 ==============================================
    // 添加"画笔粗细"标签
    QLabel *penLabel = new QLabel("    画笔粗细:  ", this);
    penLabel->setStyleSheet("QLabel { color: #555; }");  // 设置标签样式(灰色文字)
    mainToolBar->addWidget(penLabel);

    // 创建画笔粗细选择框
    sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(1, 100);  // 设置范围(1-100)
    sizeSpinBox->setValue(3);       // 设置默认值(3)
    sizeSpinBox->setFixedWidth(60); // 设置固定宽度
    sizeSpinBox->setSuffix(" ");    // 设置后缀(空格)
    sizeSpinBox->setToolTip("画笔粗细");  // 设置工具提示
    // 连接值变化信号到槽函数
    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeBrushSize);

    mainToolBar->addWidget(sizeSpinBox);  // 将选择框添加到工具栏

    // 添加"画笔颜色"标签
    QLabel *colorLabel = new QLabel("    画笔颜色:  ", this);
    colorLabel->setStyleSheet("QLabel { color: #555; }");  // 设置标签样式(灰色文字)
    mainToolBar->addWidget(colorLabel);

    // 创建颜色选择按钮
    colorBtn = new QPushButton(this);
    colorBtn->setFixedSize(32, 32);  // 设置固定大小
    // 设置按钮样式(背景色为当前颜色，带边框和圆角)
    colorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 3px;")
                                .arg(currentColor.name()));
    colorBtn->setToolTip("选择颜色");  // 设置工具提示
    connect(colorBtn, &QPushButton::clicked, this, &MainWindow::changeColor);  // 连接点击信号到槽函数

    mainToolBar->addWidget(colorBtn);  // 将按钮添加到工具栏
}

// 创建状态栏函数
void MainWindow::createStatusBar()
{
    // 设置状态栏样式(浅灰色背景，顶部有边框)
    statusBar()->setStyleSheet("QStatusBar{background-color: #f0f0f0; border-top: 1px solid #ccc;}");

    // 创建光标位置标签
    cursorPosLabel = new QLabel("位置: 0, 0", this);
    cursorPosLabel->setStyleSheet("QLabel { padding: 2px 8px; }");  // 设置内边距

    // 创建形状信息标签
    shapeInfoLabel = new QLabel("工具: 自由绘制", this);
    shapeInfoLabel->setStyleSheet("QLabel { padding: 2px 8px; }");  // 设置内边距

    // 创建缩放比例标签
    zoomLabel = new QLabel("缩放: 100%", this);
    zoomLabel->setStyleSheet("QLabel { padding: 2px 8px; }");  // 设置内边距

    // 将标签添加到状态栏(永久部件，不会被挤掉)
    statusBar()->addPermanentWidget(cursorPosLabel);
    statusBar()->addPermanentWidget(shapeInfoLabel);
    statusBar()->addPermanentWidget(zoomLabel);
}

// 改变颜色槽函数
void MainWindow::changeColor()
{
    // 创建颜色对话框
    QColorDialog colorDialog(this);
    colorDialog.setOption(QColorDialog::ShowAlphaChannel);  // 显示透明度通道
    colorDialog.setCurrentColor(currentColor);              // 设置当前颜色
    colorDialog.setWindowTitle("选择画笔颜色");             // 设置对话框标题

    // 如果用户点击了确定按钮
    if (colorDialog.exec() == QDialog::Accepted) {
        currentColor = colorDialog.currentColor();  // 获取选择的颜色
        // 更新颜色按钮的样式
        colorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 3px;")
                                    .arg(currentColor.name()));
        paintArea->setPenColor(currentColor);  // 设置绘图区域的画笔颜色
    }
}

// 改变画笔大小槽函数
void MainWindow::changeBrushSize(int size)
{
    paintArea->setPenWidth(size);  // 设置绘图区域的画笔宽度
}

// 改变形状槽函数
void MainWindow::changeShape(int index)
{
    // 设置绘图区域的绘制形状(将索引转换为枚举值)
    paintArea->setDrawShape(static_cast<PaintArea::DrawShape>(index));

    // 更新状态栏信息
    QString shapeName = shapeComboBox->itemText(index);  // 获取当前形状名称
    shapeInfoLabel->setText("工具: " + shapeName);       // 更新状态栏显示
}

// 保存图像槽函数
void MainWindow::saveImage()
{
    // 打开文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    "保存图片",
                                                    "",
                                                    "PNG图像 (*.png);;JPEG图像 (*.jpg *.jpeg);;BMP图像 (*.bmp)");

    // 如果用户选择了文件路径
    if (!filePath.isEmpty()) {
        paintArea->saveImage(filePath);  // 保存图像到指定路径
    }
}

// 打开图像槽函数
void MainWindow::openImage()
{
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "打开图片",
                                                    "",
                                                    "图像文件 (*.png *.jpg *.jpeg *.bmp)");

    // 如果用户选择了文件
    if (!filePath.isEmpty()) {
        paintArea->loadImage(filePath);  // 加载图像文件
    }
}

// 撤销操作槽函数
void MainWindow::undo()
{
    paintArea->undo();  // 执行撤销操作
    update();           // 更新界面
}

// 重做操作槽函数
void MainWindow::redo()
{
    paintArea->redo();  // 执行重做操作
    update();          // 更新界面
}

// 更新光标位置槽函数
void MainWindow::updateCursorPosition(const QPoint& pos)
{
    // 更新状态栏显示的光标位置
    cursorPosLabel->setText(QString("位置: %1, %2").arg(pos.x()).arg(pos.y()));
}
