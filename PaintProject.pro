QT       += core gui widgets printsupport
CONFIG += c++17 utf8
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    paintarea.cpp \
    shapes.cpp

HEADERS += \
    mainwindow.h \
    paintarea.h \
    shape.h \
    shapes.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
