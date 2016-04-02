#include "threadex.h"

void ThreadEx::run()
{
    srand(time(NULL));
    bool isSin=stricmp(functionToUse,"sin")==0;
    bool isCos=stricmp(functionToUse,"cos")==0;
    bool isAbs=stricmp(functionToUse,"abs")==0;
    for(;;)
    {
        if(isInterruptionRequested())
            break;

        double inputValue=-M_PI+2.0*((double)rand()/(double)RAND_MAX)*M_PI+functionXOffset;
        double *inputArray=(double*)malloc(1*sizeof(double));
        double *desiredOutput=(double*)malloc(1*sizeof(double));
        inputArray[0]=inputValue;
        desiredOutput[0]=isSin?sin(inputValue):(isCos?cos(inputValue):(isAbs?abs(inputValue):floor(inputValue)));
        double *output=associatedNNT->process(inputArray);
        feedback(inputValue,output[0]);
        associatedNNT->learn(desiredOutput);
        free(output);
        free(desiredOutput);
        free(inputArray);
    }
}
