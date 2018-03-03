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
    virtual void startExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();

    virtual void registerSamples( float64 * samples );
};

#endif // PulseAcquire_H
