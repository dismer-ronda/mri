#include "taskrepetitions.h"

TaskRepetitions::TaskRepetitions( const QString & name, double tr, int repetitions )
    : ExperimentTask( name )
{
    this->tr = tr;
    this->repetitions = repetitions;
}

void TaskRepetitions::createTask()
{
    ExperimentTask::createTask();

#ifndef LINUX_BOX
    DAQmxErrChk( "DAQmxCreateCOPulseChanFreq", DAQmxCreateCOPulseChanFreq( taskId, "Dev1/ctr1", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0, 1/tr, 5e-03/tr ) );
    DAQmxErrChk( "DAQmxCfgImplicitTiming", DAQmxCfgImplicitTiming( taskId, DAQmx_Val_FiniteSamps, repetitions ) );
#endif
}
