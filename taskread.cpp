#include "taskread.h"
#include "experimentthread.h"

int32 CVICALLBACK TaskReadCallback( TaskHandle taskHandle, int32, uInt32 nSamples, void *callbackData )
{
    int32 read = 0;
    float64 * samples = new float64[2 * nSamples];

    ExperimentThread * thread = (ExperimentThread*)callbackData;

#ifndef LINUX_BOX
    DAQmxReadAnalogF64( taskHandle, nSamples, 0.1, DAQmx_Val_GroupByScanNumber, samples, 2 * nSamples, &read, NULL );
#endif

    bool ok = read == nSamples;

    qDebug() << "read samples " << (ok ? "ok" : "failed");

    if ( ok )
        thread->registerSamples( samples );

    delete samples;

    return 0;
}

TaskRead::TaskRead(const QString & name, int samplingRate, int nsamples, void * callbackData )
    : ExperimentTask( name )
{
    this->samplingRate = samplingRate;
    this->nsamples = nsamples;
    this->callbackData = callbackData;
}

void TaskRead::createTask()
{
    ExperimentTask::createTask();

#ifndef LINUX_BOX
    DAQmxErrChk( "DAQmxCreateAIVoltageChan", DAQmxCreateAIVoltageChan( taskId, "Dev1/ai0:Dev1/ai1", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL ) );
    DAQmxErrChk( "DAQmxCfgSampClkTiming", DAQmxCfgSampClkTiming( taskId, "", samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, nsamples ) );
    DAQmxErrChk( "DAQmxCfgDigEdgeStartTrig", DAQmxCfgDigEdgeStartTrig( taskId, "/Dev1/Ctr3InternalOutput", DAQmx_Val_Rising ) );
    DAQmxErrChk( "DAQmxSetStartTrigRetriggerable", DAQmxSetStartTrigRetriggerable( taskId, TRUE ) );
    DAQmxErrChk( "DAQmxRegisterEveryNSamplesEvent", DAQmxRegisterEveryNSamplesEvent( taskId, DAQmx_Val_Acquired_Into_Buffer, nsamples, 0, TaskReadCallback, callbackData ) );
#endif
}
