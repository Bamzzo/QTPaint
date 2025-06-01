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
#include <QShortcut>
#include <QStyleFactory>
#include <QStatusBar>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentColor(Qt::black)
{
    // 设置现代风格
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // 设置调色板
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25,25,25));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(palette);

    setWindowTitle("绘图工具");
    resize(1200, 800);

    // 创建绘图区域
    paintArea = new PaintArea(this);
    setCentralWidget(paintArea);
    connect(paintArea, &PaintArea::cursorPositionChanged, this, &MainWindow::updateCursorPosition);

    // 初始化UI组件
    createToolBar();
    createStatusBar();
    setupShortcuts();
}

void MainWindow::createToolBar()
{
    // 主工具栏
    QToolBar *mainToolBar = addToolBar("主工具栏");
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(24, 24));

    // 形状选择
    shapeComboBox = new QComboBox(this);
    shapeComboBox->addItems({"自由绘制", "直线", "矩形", "椭圆", "箭头", "五角星", "菱形", "心形", "橡皮擦", "编组选择"});
    shapeComboBox->setFixedWidth(120);
    shapeComboBox->setStyleSheet("QComboBox { color: white; }");
    connect(shapeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeShape);

    // 画笔大小设置
    sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(1, 100);
    sizeSpinBox->setValue(3);
    sizeSpinBox->setFixedWidth(60);
    sizeSpinBox->setStyleSheet("QSpinBox { color: white; }");
    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::changeBrushSize);

    // 颜色按钮
    colorBtn = new QPushButton(this);
    colorBtn->setFixedSize(32, 32);
    colorBtn->setStyleSheet(QString(
                                "QPushButton { background-color: %1; border: 1px solid #666; border-radius: 3px; }"
                                "QPushButton:hover { border: 1px solid #aaa; }"
                                ).arg(currentColor.name()));
    connect(colorBtn, &QPushButton::clicked, this, &MainWindow::changeColor);

    // 文件操作按钮
    openBtn = new QPushButton("打开", this);
    openBtn->setStyleSheet("QPushButton { padding: 3px 8px; }");
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::openImage);

    saveBtn = new QPushButton("保存", this);
    saveBtn->setStyleSheet("QPushButton { padding: 3px 8px; }");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveImage);

    // 撤销/重做按钮
    undoAction = new QAction("撤销", this);
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction = new QAction("重做", this);
    redoAction->setShortcut(QKeySequence::Redo);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);

    // 添加工具栏组件
    mainToolBar->addWidget(new QLabel(" 工具: ", this));
    mainToolBar->addWidget(shapeComboBox);
    mainToolBar->addSeparator();

    mainToolBar->addWidget(new QLabel(" 大小: ", this));
    mainToolBar->addWidget(sizeSpinBox);
    mainToolBar->addSeparator();

    mainToolBar->addWidget(new QLabel(" 颜色: ", this));
    mainToolBar->addWidget(colorBtn);
    mainToolBar->addSeparator();

    mainToolBar->addWidget(openBtn);
    mainToolBar->addWidget(saveBtn);
    mainToolBar->addSeparator();

    mainToolBar->addAction(undoAction);
    mainToolBar->addAction(redoAction);
}

void MainWindow::createStatusBar()
{
    cursorPosLabel = new QLabel("坐标: (0, 0)", this);
    shapeLabel = new QLabel("工具: 自由绘制", this);
    sizeLabel = new QLabel("大小: 3px", this);

    statusBar()->addPermanentWidget(cursorPosLabel);
    statusBar()->addPermanentWidget(shapeLabel);
    statusBar()->addPermanentWidget(sizeLabel);

    statusBar()->setStyleSheet("QStatusBar { color: #aaa; }");
}

void MainWindow::setupShortcuts()
{
    // 添加更多快捷键
    new QShortcut(QKeySequence::Save, this, SLOT(saveImage()));
    new QShortcut(QKeySequence::Open, this, SLOT(openImage()));
}

void MainWindow::changeColor()
{
    QColor newColor = QColorDialog::getColor(currentColor, this, "选择颜色",
                                             QColorDialog::DontUseNativeDialog);
    if (newColor.isValid()) {
        currentColor = newColor;
        colorBtn->setStyleSheet(QString(
                                    "QPushButton { background-color: %1; border: 1px solid #666; border-radius: 3px; }"
                                    "QPushButton:hover { border: 1px solid #aaa; }"
                                    ).arg(currentColor.name()));
        paintArea->setPenColor(currentColor);
    }
}

void MainWindow::changeBrushSize(int size)
{
    paintArea->setPenWidth(size);
    sizeLabel->setText(QString("大小: %1px").arg(size));
}

void MainWindow::changeShape(int index)
{
    paintArea->setDrawShape(static_cast<PaintArea::DrawShape>(index));
    shapeLabel->setText(QString("工具: %1").arg(shapeComboBox->currentText()));
}

void MainWindow::updateCursorPosition(const QPoint& pos)
{
    cursorPosLabel->setText(QString("坐标: (%1, %2)").arg(pos.x()).arg(pos.y()));
}

void MainWindow::saveImage()
{
    QString filePath = QFileDialog::getSaveFileName(this, "保存图片", "", "PNG文件(*.png);;JPEG文件(*.jpg *.jpeg);;BMP文件(*.bmp)");
    if (!filePath.isEmpty()) {
        paintArea->saveImage(filePath);
    }
}

void MainWindow::openImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "打开图片", "", "图片文件(*.png *.jpg *.jpeg *.bmp)");
    if (!filePath.isEmpty()) {
        paintArea->loadImage(filePath);
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
