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

    data = NULL;
}

ExperimentThread::~ExperimentThread()
{
    if ( data != NULL )
        delete data;
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

    errorCode = 0;

    taskAcqGate = taskRepetitions = taskRFGate = taskRead = NULL;

    setFinished(false);

    try
    {
        createExperiment();

        startExperiment();

        while ( !isFinished() )
            QThread::msleep(100);
    }
    catch ( ExperimentException e )
    {
        qDebug() << "error executing experiment";
        errorCode = e.code;
    }
    catch ( ...  )
    {
        errorCode = ExperimentException::ERROR_UNKNOWN;
    }

    stopExperiment();

    parent->setFinished( true );

    qDebug() << "experiment thread finished";
}

MainWindow * ExperimentThread::getParentWindow()
{
    return parent;
}

void ExperimentThread::createExperiment()
{
    nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();
    zeroOffset = Settings::getExperimentParameter( experiment, "ZeroOffset" ).toInt();
    graph1 = Settings::getExperimentParameter( experiment, "Graph1" ).toString();
    graph2 = Settings::getExperimentParameter( experiment, "Graph2" ).toString();
    graph3 = Settings::getExperimentParameter( experiment, "Graph3" ).toString();
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
    if ( taskRead != NULL )
        taskRead->destroyTask();

    if ( taskRFGate != NULL )
        taskRFGate->destroyTask();

    if ( taskAcqGate != NULL )
        taskAcqGate->destroyTask();

    if ( taskRepetitions != NULL)
        taskRepetitions->destroyTask();
}
