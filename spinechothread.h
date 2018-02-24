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

public:
    SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent );

    virtual void createExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();
};

#endif // SpinEchoThread_H
