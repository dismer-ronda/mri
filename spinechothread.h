#ifndef SpinEchoThread_H
#define SpinEchoThread_H

#include <QThread>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experiment.h"

class SpinEchoThread : public ExperimentThread
{
    Q_OBJECT

private:
    TaskHandle  taskRepetitions;
    TaskHandle  taskRead;
    TaskHandle  taskTimer;
    TaskHandle  taskRFGate;
    TaskHandle  taskAcqGate;

    float64 dataFreq[1024];
    float64 dataDC[1024];

public:

    SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent );

    virtual void startExperiment();
    virtual void finishExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();
};

#endif // SpinEchoThread_H
