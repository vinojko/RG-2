QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets #v QT6 je po novem OpenGL premaknjen v modul openglwidgets

TARGET = RG_Template
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    widgetopengldraw.cpp

HEADERS  += mainwindow.h \
    widgetopengldraw.h

FORMS    += mainwindow.ui

CONFIG += c++17  # Qt6 zahteva novejši prevajalnik, Qt5 bo zadovoljen s C++11

#INCLUDEPATH += pot do GLM, ne pozabi spremeniti
INCLUDEPATH += C:/glm/
