#ifndef ExperimentTHREAD_H
#define ExperimentTHREAD_H

#include <QThread>
#include <QMutex>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experiment.h"

class ExperimentThread : public QThread
{
    Q_OBJECT

protected:
    QString binDir;
    MainWindow * parent;
    QString experiment;

    QMutex mutex;
    bool finished;

public:

    ExperimentThread( QString binDir, const QString & experiment, MainWindow * parent );

    bool isFinished();
    void setFinished( bool value );

    virtual void startExperiment() = 0;
    virtual void finishExperiment() = 0;

    virtual int getProgressCount() = 0;
    virtual int getProgressTimer() = 0;

    MainWindow * getParentWindow();

protected:
    void run();
};

#endif // ExperimentTHREAD_H
