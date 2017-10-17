#ifndef PROGRAMMERTHREAD_H
#define PROGRAMMERTHREAD_H

#include <QThread>
#include <QMutex>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experiment.h"

class ProgrammerThread : public QThread
{
    Q_OBJECT

private:
    QString binDir;
    MainWindow * parent;
    Experiment * experiment;

    TaskHandle  taskRepetitions;
    TaskHandle  taskRead;
    TaskHandle  taskTimer;
    TaskHandle  taskRFGate;
    TaskHandle  taskAcqGate;



    float64 dataFreq[1024];
    float64 dataDC[1024];

    QMutex mutex;
    bool finished;

public:

    ProgrammerThread( QString binDir, Experiment * experiment, MainWindow * parent );

    void SpinEcho();

    bool isFinished();
    void setFinished( bool value );

    MainWindow * getParentWindow();

protected:
    void run();
};

#endif // PROGRAMMERTHREAD_H
