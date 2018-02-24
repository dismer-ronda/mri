#include <stdio.h>

#include <QDebug>

#include "experimentthread.h"
#include "mainwindow.h"
#include "settings.h"

ExperimentThread::ExperimentThread( QString binDir, const QString & experiment, MainWindow * parent )
{
    this->binDir = binDir;
    this->parent = parent;
    this->experiment = experiment;
}

bool ExperimentThread::isFinished()
{
    bool ret;
    mutex.lock();
    ret = finished;
    mutex.unlock();
    return ret;
}

void ExperimentThread::setFinished( bool value )
{
    mutex.lock();
    finished = value;
    mutex.unlock();
}

void ExperimentThread::run()
{
    qDebug() << "experiment thread started";

    setFinished(false);

    createExperiment();

    try
    {
        startExperiment();

        while ( !isFinished() )
            QThread::msleep(100);
    }
    catch ( ... )
    {
        qDebug() << "error executing experiment";
    }

    stopExperiment();

    parent->setFinished( true );

    qDebug() << "experiment thread finished";
}

MainWindow * ExperimentThread::getParentWindow()
{
    return parent;
}

void ExperimentThread::startExperiment()
{
    taskRFGate->startTask();
    taskRead->startTask();
    taskAcqGate->startTask();

    taskRepetitions->startTask();
}

void ExperimentThread::stopExperiment()
{
    taskRead->destroyTask();
    taskRFGate->destroyTask();
    taskAcqGate->destroyTask();
    taskRepetitions->destroyTask();
}
