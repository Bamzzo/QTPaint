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

/**
 * @brief 主窗口类，负责应用程序的主界面和功能控制
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT  // Qt元对象系统宏，支持信号槽机制

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    // 以下是各种功能槽函数
    void changeColor();  // 改变绘图颜色
    void changeBrushSize(int size);  // 改变画笔大小
    void changeShape(int index);  // 改变绘图形状
    void saveImage();  // 保存图像
    void openImage();  // 打开图像
    void undo();  // 撤销操作
    void redo();  // 重做操作
    void updateCursorPosition(const QPoint& pos);  // 更新光标位置显示

private:
    // 私有辅助函数
    void createToolBar();  // 创建工具栏
    void createStatusBar();  // 创建状态栏
    QPushButton* createToolButton(const QString& text, const QString& tooltip = "");  // 创建工具按钮

    // 成员变量
    PaintArea *paintArea;  // 绘图区域组件
    QColor currentColor;  // 当前绘图颜色
    QPushButton *colorBtn;  // 颜色选择按钮
    QSpinBox *sizeSpinBox;  // 画笔大小调节框
    QComboBox *shapeComboBox;  // 形状选择下拉框
    QAction *undoAction;  // 撤销动作
    QAction *redoAction;  // 重做动作

    // 状态栏控件
    QLabel *cursorPosLabel;  // 显示光标位置
    QLabel *shapeInfoLabel;  // 显示形状信息
    QLabel *zoomLabel;  // 显示缩放比例
};

#endif // MAINWINDOW_H
