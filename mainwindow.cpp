#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    started=false;
    interrupt=false;
    ui->useSinFunctRadioBtn->setChecked(true);
    connect(ui->startBtn,SIGNAL(clicked(bool)),this,SLOT(startBtnClicked()));
    connect(ui->clearBtn,SIGNAL(clicked(bool)),this,SLOT(clearBtnClicked()));
    nnt=0;
    learningThread=new ThreadEx();
    learningThread->functionToUse=strdup("sin");
    learningThread->functionXOffset=0.0;
    connect(learningThread,SIGNAL(feedback(double,double)),this,SLOT(feedbackReceived(double,double)));
    scene=new GraphicsSceneEx(this);
    pixmapItem=new QGraphicsPixmapItem();
    scene->addItem(pixmapItem);
    correctPathItem=new QGraphicsPathItem();
    functionPathItem=new QGraphicsPathItem();
    correctPathItem->setPen(QPen(QColor::fromRgb(0,0,255)));
    functionPathItem->setPen(QPen(QColor::fromRgb(255,0,0)));
    scene->addItem(correctPathItem);
    scene->addItem(functionPathItem);
    pixmapItem->setZValue(0);
    correctPathItem->setZValue(1);
    functionPathItem->setZValue(2);
    ui->graphicsView->setScene(scene);
    functionInterval=DEFAULT_FUNCTION_INTERVAL;
    // We want to draw the interval [-Pi,...,Pi] of the function.
    functionValueCount=(uint32_t)ceil((2.0*M_PI)/functionInterval);
    receivedFunctionValues=(double*)malloc(functionValueCount*sizeof(double));
    for(uint32_t i=0;i<functionValueCount;i++)
        receivedFunctionValues[i]=0.0;
    redrawPixmap();
}

uint32_t MainWindow::mapValueToIndex(double value, double functionXOffset)
{
    return floor(((value-functionXOffset)+M_PI)/functionInterval);
}

double MainWindow::mapIndexToValue(uint32_t index, double functionXOffset)
{
    return -M_PI+index*functionInterval+functionXOffset;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startBtnClicked()
{
    if(started)
    {
        started=false;
        interrupt=true;
        learningThread->interrupt=true;
        learningThread->wait();
        redrawPixmap();
        ui->learningCycleLbl->setText("Learning cycle: "+QString::number(learningCycle));
        ui->startBtn->setText("Start");
        interrupt=false;
    }
    else if(!interrupt)
    {
        redrawPixmap();
        uint32_t hiddenLayers=ui->hiddenLayerCountBox->value();
        uint32_t layerCount=hiddenLayers+2;
        uint32_t lastLayer=layerCount-1;
        uint32_t neuronsPerHiddenLayer=ui->neuronsPerHiddenLayerBox->value();
        uint32_t totalNeuronCount=hiddenLayers*neuronsPerHiddenLayer+2;
        double learningRate=ui->learningRateBox->value();
        double momentum=ui->momentumBox->value();
        char *functionToUse=strdup(ui->useSinFunctRadioBtn->isChecked()?"sin":(ui->useCosFunctRadioBtn->isChecked()?"cos":(ui->useAbsFunctRadioBtn->isChecked()?"abs":"floor")));
        double functionXOffset=ui->xOffsetBox->value();
        if(nnt==0||learningThread->functionXOffset!=functionXOffset||(stricmp(learningThread->functionToUse,functionToUse)!=0||nnt->layerCount!=layerCount||nnt->totalNeuronCount!=totalNeuronCount))
        {
            if(nnt!=0)
            {
                clearBtnClicked();
                delete nnt;
            }
            learningCycle=0;
            ui->learningCycleLbl->setText("Learning cycle: 0");
            uint32_t *layerNeuronCounts=(uint32_t*)malloc(layerCount*sizeof(uint32_t));
            for(uint32_t layer=0;layer<layerCount;layer++)
                layerNeuronCounts[layer]=(layer==0||layer==lastLayer?1:neuronsPerHiddenLayer);
            nnt=new FFNNT(layerCount,layerNeuronCounts,totalNeuronCount,learningRate,momentum,false);
            learningThread->associatedNNT=nnt;
            learningThread->functionToUse=functionToUse;
            learningThread->functionXOffset=functionXOffset;
        }
        else
        {
            // Apply any changes that could have been made.
            nnt->learningRate=learningRate;
            nnt->momentum=momentum;
        }
        learningThread->start();
        started=true;
        ui->startBtn->setText("Stop");
    }
}

void MainWindow::clearBtnClicked()
{
    for(uint32_t i=0;i<functionValueCount;i++)
        receivedFunctionValues[i]=0.0;
    if(!started)
    {
        delete nnt;
        nnt=0;
        ui->learningCycleLbl->setText("Learning cycle: 0");
    }
    redrawPixmap();
}

void MainWindow::feedbackReceived(double input, double output)
{
    if(!started||interrupt)
        return;
    receivedFunctionValues[mapValueToIndex(input,learningThread->functionXOffset)]=output;
    ++learningCycle;
    if(learningCycle%2000==0)
        redrawPixmap();
    else
        redrawPixmap(input);
    if(learningCycle%500==0)
        ui->learningCycleLbl->setText("Learning cycle: approx. "+QString::number(learningCycle));
}

void MainWindow::redrawPixmap()
{
    uint32_t imageWidth=DEFAULT_PIXMAP_WIDTH;
    uint32_t imageHeight=DEFAULT_PIXMAP_HEIGHT;
    QImage image=QImage(imageWidth,imageHeight,QImage::Format_ARGB32);
    image.fill(0xFFFFFFFF);
    QPainter painter(&image);
    painter.setPen(QColor::fromRgb(0,0,0));
    int verticalCenter=text::round(imageHeight/2);
    int rightEnd=imageWidth-1;
    painter.drawLine(0,verticalCenter,rightEnd,verticalCenter);
    //painter.drawLine(horizontalCenter,0,horizontalCenter,bottomEnd); // If you do draw the Y axis, you should move it to fit the function's X offset!
    // Do not use radioBtn...->isChecked(): the user can change the function while the NNT is learning!
    bool isSin=stricmp(learningThread->functionToUse,"sin")==0;
    bool isCos=stricmp(learningThread->functionToUse,"cos")==0;
    bool isAbs=stricmp(learningThread->functionToUse,"abs")==0;
    uint32_t pixelsPerFunctionValue=floor(((double)imageWidth)/((double)functionValueCount));
    double functionXOffset=ui->xOffsetBox->value();
    if(pixelsPerFunctionValue==0)
    {
        throw "pixelsPerFunctionValue is 0. functionValueCount is too high!";
        return;
    }
    // Draw correct function
    correctPath=QPainterPath();
    uint32_t xOffset=text::round((double)((double)imageWidth-((double)pixelsPerFunctionValue*functionValueCount))/2.0);
    for(uint32_t i=0;i<functionValueCount;i++)
    {
        double x=-M_PI+((double)i)*functionInterval+functionXOffset;
        double y=isSin?sin(x):(isCos?cos(x):(isAbs?abs(x):floor(x)));
        uint32_t imageX=xOffset+i*pixelsPerFunctionValue;
        int32_t imageY=verticalCenter-y*0.5*(double)imageHeight; // Define "0.5*imageHeight" as deltaY=1.0
        if(imageY<0)
            imageY=0;
        else if((uint32_t)imageY>imageHeight-1) // We already know imageY is greater than 0; just keep (uint32_t) here to remove the signed/unsigned mismatch warning
            imageY=imageHeight-1;
        if(i==0) // Or also: if function jumps?
            correctPath.moveTo(imageX,imageY);
        else
            correctPath.lineTo(imageX,imageY);
    }
    correctPathItem->setPath(correctPath);
    // Draw function based on values returned by NNT
    functionPath=QPainterPath();
    for(uint32_t i=0;i<functionValueCount;i++)
    {
        double y=receivedFunctionValues[i];
        uint32_t imageX=xOffset+i*pixelsPerFunctionValue;
        int32_t imageY=verticalCenter-y*0.5*(double)imageHeight; // Define "0.5*imageHeight" as deltaY=1.0
        if(imageY<0)
            imageY=0;
        else if((uint32_t)imageY>imageHeight-1) // We already know imageY is greater than 0; just keep (uint32_t) here to remove the signed/unsigned mismatch warning
            imageY=imageHeight-1;
        if(i==0) // Or also: if function jumps?
            functionPath.moveTo(imageX,imageY);
        else
            functionPath.lineTo(imageX,imageY);
    }
    functionPathItem->setPath(functionPath);
    pixmapItem->setPixmap(QPixmap::fromImage(image));
    ui->graphicsView->repaint();
}

void MainWindow::redrawPixmap(double atValue)
{
    if(rand()%100!=0)
        return;
    uint32_t imageHeight=DEFAULT_PIXMAP_HEIGHT;
    double functionXOffset=learningThread->functionXOffset;
    uint32_t index=mapValueToIndex(atValue,functionXOffset);
    int verticalCenter=text::round(imageHeight/2);
    double y=receivedFunctionValues[index];
    int32_t imageY=verticalCenter-y*0.5*(double)imageHeight; // Define "0.5*imageHeight" as deltaY=1.0
    if(imageY<0)
        imageY=0;
    else if((uint32_t)imageY>imageHeight-1) // We already know imageY is greater than 0; just keep (uint32_t) here to remove the signed/unsigned mismatch warning
        imageY=imageHeight-1;
    functionPath.setElementPositionAt(index,functionPath.elementAt(index).x,imageY);
    functionPathItem->setPath(functionPath);
    ui->graphicsView->repaint();
}
