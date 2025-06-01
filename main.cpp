#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置全局样式表
    a.setStyleSheet(
        "QMainWindow { background: #353535; }"
        "QToolBar { background: #404040; border: none; padding: 2px; }"
        "QToolButton { padding: 3px; }"
        "QComboBox, QSpinBox { background: #353535; border: 1px solid #555; padding: 2px; }"
        "QComboBox:hover, QSpinBox:hover { border: 1px solid #777; }"
        "QComboBox::drop-down, QSpinBox::up-button, QSpinBox::down-button {"
        "   width: 16px; border-left: 1px solid #555;"
        "}"
        "QStatusBar { background: #404040; border-top: 1px solid #555; }"
        );

    MainWindow w;
    w.show();
    return a.exec();
}
