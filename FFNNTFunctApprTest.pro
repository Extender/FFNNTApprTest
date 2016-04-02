#-------------------------------------------------
#
# Project created by QtCreator 2016-04-01T12:03:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FFNNTFunctApprTest
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graphicssceneex.cpp \
    graphicsviewex.cpp \
    ../FeedforwardNeuralNetwork/text.cpp \
    ../FeedforwardNeuralNetwork/io.cpp \
    ../FeedforwardNeuralNetwork/ffnnt.cpp \
    threadex.cpp

HEADERS  += mainwindow.h \
    graphicssceneex.h \
    graphicsviewex.h \
    ../FeedforwardNeuralNetwork/text.h \
    ../FeedforwardNeuralNetwork/io.h \
    ../FeedforwardNeuralNetwork/ffnnt.h \
    threadex.h

FORMS    += mainwindow.ui
