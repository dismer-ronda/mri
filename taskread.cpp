#include "taskread.h"

TaskRead::TaskRead(const QString & name, int sampleRate, int nsamples, DAQmxEveryNSamplesEventCallbackPtr callbackFunction )
    : ExperimentTask( name )
{
    this->sampleRate = sampleRate;
    this->nsamples = nsamples;
    this->callbackFunction = callbackFunction;
}

void TaskRead::createTask()
{
    ExperimentTask::createTask();

#ifndef LINUX_BOX
    DAQmxErrChk( DAQmxCreateAIVoltageChan( taskId, "Dev1/ai1", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL ) ;
    DAQmxErrChk( DAQmxCfgSampClkTiming( taskId, "", sampleRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, nsamples ) );
    DAQmxErrChk( DAQmxCfgDigEdgeStartTrig( taskId, "/Dev1/Ctr3InternalOutput", DAQmx_Val_Rising ) );
    DAQmxErrChk( DAQmxSetStartTrigRetriggerable( taskId, TRUE ) );
    DAQmxErrChk( DAQmxRegisterEveryNSamplesEvent( taskId, DAQmx_Val_Acquired_Into_Buffer, nsamples, 0, callbackFunction, this ) );
#endif
}
