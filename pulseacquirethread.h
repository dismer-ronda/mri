#ifndef PulseAcquire_H
#define PulseAcquire_H

#include <QThread>
#include <QMutex>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experimenttask.h"

class PulseAcquireThread : public ExperimentThread
{
    Q_OBJECT

public:
    float64 * data;

    PulseAcquireThread( QString binDir, const QString & experiment, MainWindow * parent );
    virtual ~PulseAcquireThread();

    virtual void createExperiment();
    virtual void startExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();
};

#endif // PulseAcquire_H
