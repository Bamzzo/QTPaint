#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>

/**
 * @brief 应用程序的主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序的退出码
 */
int main(int argc, char *argv[])
{
    // 创建Qt应用程序实例
    QApplication a(argc, argv);

    // 设置应用程序样式为Fusion(现代Qt样式)
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // 创建并配置自定义调色板以实现统一主题
    QPalette palette;
    // 窗口背景色(浅灰色)
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    // 窗口文字颜色(黑色)
    palette.setColor(QPalette::WindowText, Qt::black);
    // 输入控件背景色(白色)
    palette.setColor(QPalette::Base, Qt::white);
    // 交替行颜色(浅灰色)
    palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    // 工具提示背景色(白色)
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    // 工具提示文字颜色(黑色)
    palette.setColor(QPalette::ToolTipText, Qt::black);
    // 文本颜色(黑色)
    palette.setColor(QPalette::Text, Qt::black);
    // 按钮背景色(浅灰色)
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    // 按钮文字颜色(黑色)
    palette.setColor(QPalette::ButtonText, Qt::black);
    // 高亮文字颜色(红色)
    palette.setColor(QPalette::BrightText, Qt::red);
    // 选中项高亮颜色(蓝色)
    palette.setColor(QPalette::Highlight, QColor(61, 142, 255));
    // 选中项文字颜色(白色)
    palette.setColor(QPalette::HighlightedText, Qt::white);
    // 禁用状态文字颜色(深灰色)
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    // 禁用状态按钮文字颜色(深灰色)
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    // 应用自定义调色板
    QApplication::setPalette(palette);

    // 创建主窗口并显示
    MainWindow w;
    w.show();

    // 进入主事件循环
    return a.exec();
}
