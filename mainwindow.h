#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include "threadex.h"
#include "../FeedforwardNeuralNetwork/ffnnt.h"
#include "graphicssceneex.h"

#define DEFAULT_FUNCTION_INTERVAL 0.05
#define DEFAULT_PIXMAP_WIDTH 800
#define DEFAULT_PIXMAP_HEIGHT 600

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Ui::MainWindow *ui;

    bool started;
    FFNNT *nnt;
    ThreadEx *learningThread;
    GraphicsSceneEx *scene;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPathItem *correctPathItem;
    QGraphicsPathItem *functionPathItem;
    double functionInterval;
    uint32_t functionValueCount;
    double *receivedFunctionValues;
    QPainterPath correctPath;
    QPainterPath functionPath;
    uint64_t learningCycle;

    explicit MainWindow(QWidget *parent = 0);
    uint32_t mapValueToIndex(double value,double functionXOffset);
    double mapIndexToValue(uint32_t index,double functionXOffset);
    ~MainWindow();

public slots:
    void startBtnClicked();
    void clearBtnClicked();
    void feedbackReceived(double input,double output);
    void redrawPixmap();
    void redrawPixmap(double atValue);
};

#endif // MAINWINDOW_H
