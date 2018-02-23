#ifndef PulseAcquire_H
#define PulseAcquire_H

#include <QThread>
#include <QMutex>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experiment.h"

class PulseAcquireThread : public ExperimentThread
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

    PulseAcquireThread( QString binDir, const QString & experiment, MainWindow * parent );

    virtual void startExperiment();
    virtual void finishExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();
};

#endif // PulseAcquire_H
