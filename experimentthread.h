#ifndef ExperimentTHREAD_H
#define ExperimentTHREAD_H

#include <QThread>
#include <QMutex>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experimenttask.h"
#include "taskrepetitions.h"
#include "taskrfgate.h"
#include "taskacquisitiongate.h"
#include "taskread.h"

class ExperimentThread : public QThread
{
    Q_OBJECT

protected:
    QString binDir;
    MainWindow * parent;
    QString experiment;

    QMutex mutex;
    bool finished;

    ExperimentTask * taskRepetitions;
    ExperimentTask * taskRFGate;
    ExperimentTask * taskAcqGate;
    ExperimentTask * taskRead;

public:

    ExperimentThread( QString binDir, const QString & experiment, MainWindow * parent );

    bool isFinished();
    void setFinished( bool value );

    virtual void createExperiment() = 0;

    void startExperiment();
    void stopExperiment();

    virtual int getProgressCount() = 0;
    virtual int getProgressTimer() = 0;

    MainWindow * getParentWindow();

protected:
    void run();
};

#endif // ExperimentTHREAD_H
