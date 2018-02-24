#include "experimenttask.h"

ExperimentTask::ExperimentTask( const QString & name )
{
    this->name = name;
    taskId = 0;
}

void ExperimentTask::createTask()
{
#ifndef LINUX_BOX
    DAQmxErrChk( DAQmxCreateTask( name.toStdString().c_str(), &taskId ) );
#endif
}

void ExperimentTask::destroyTask()
{
#ifndef LINUX_BOX
    DAQmxErrChk( DAQmxStopTask( taskId ) );
    DAQmxErrChk( DAQmxClearTask( taskId ) );
#endif
    taskId = 0;
}

void ExperimentTask::startTask()
{
#ifndef LINUX_BOX
    DAQmxErrChk( DAQmxStartTask( taskId ) );
#endif
}
