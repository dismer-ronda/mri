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
    PulseAcquireThread( QString binDir, const QString & experiment, MainWindow * parent );

    virtual void createExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();
};

#endif // PulseAcquire_H
