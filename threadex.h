#ifndef THREADEX_H
#define THREADEX_H

#include <QThread>
#include <QApplication>
#include <math.h>

#include "../FeedforwardNeuralNetwork/ffnnt.h"

class ThreadEx : public QThread
{
    Q_OBJECT

public:
    FFNNT *associatedNNT;
    char *functionToUse;
    double functionXOffset;
    bool interrupt;

    void run();

signals:
    void feedback(double input,double output);
};

#endif // THREADEX_H
