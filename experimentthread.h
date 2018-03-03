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

    float64 * data;
    QString module;
    int nsamples;

public:
    int errorCode;

    ExperimentThread( QString binDir, const QString & experiment, MainWindow * parent );
    virtual ~ExperimentThread();

    bool isFinished();
    void setFinished( bool value );

    virtual void createExperiment() = 0;

    void stopExperiment();

    virtual void startExperiment();
    virtual int getProgressCount() = 0;
    virtual int getProgressTimer() = 0;

    virtual void registerSamples( float64 * samples ) = 0;

    MainWindow * getParentWindow();

protected:
    void run();
};

#endif // ExperimentTHREAD_H
