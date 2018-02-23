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

    startExperiment();

    while ( !isFinished() )
        QThread::msleep(100);

    parent->setFinished( true );

    finishExperiment();

    qDebug() << "experiment thread finished";
}

MainWindow * ExperimentThread::getParentWindow()
{
    return parent;
}
