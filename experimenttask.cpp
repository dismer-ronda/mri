#include "experimenttask.h"

ExperimentTask::ExperimentTask( const QString & name )
{
    this->name = name;
    taskId = 0;
}

void ExperimentTask::createTask()
{
#ifndef LINUX_BOX
    qDebug() << "creating " << name;
    DAQmxErrChk( "DAQmxCreateTask", DAQmxCreateTask( name.toStdString().c_str(), &taskId ) );
#endif
}

void ExperimentTask::destroyTask()
{
#ifndef LINUX_BOX
    qDebug() << "destroying " + name;
    DAQmxErrChk( "DAQmxStopTask", DAQmxStopTask( taskId ) );
    DAQmxErrChk( "DAQmxClearTask", DAQmxClearTask( taskId ) );
#endif
    taskId = 0;
}

void ExperimentTask::startTask()
{
#ifndef LINUX_BOX
    qDebug() << "starting " + name;
//    DAQmxErrChk( "DAQmxStartTask", DAQmxStartTask( taskId ) );
    DAQmxStartTask( taskId );
#endif
}
