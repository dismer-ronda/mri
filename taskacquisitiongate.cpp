#include "taskacquisitiongate.h"

TaskAcquisitionGate::TaskAcquisitionGate( const QString & name, double t90, double techo, int nechoes )
    : ExperimentTask( name )
{
    this->t90 = t90;
    this->techo = techo;
    this->nechoes = nechoes;
}

void TaskAcquisitionGate::createTask()
{
    ExperimentTask::createTask();

    double tacq = 1e-03;
    double InitialDelaycount3 = 0;
    int32 nReadouts;

    if ( nechoes > 0 )
    {
        InitialDelaycount3 = t90/2 + techo - (tacq/2);
        nReadouts = nechoes;
    }
    else
    {
        double ringdowndelay = 0.5e-03;
        InitialDelaycount3 = t90 + ringdowndelay;
        nReadouts = 1;
    }

    double Freqcount3 = 1/techo;
    double dutycyclecount3 = tacq/techo;

#ifndef LINUX_BOX
    DAQmxErrChk( DAQmxCreateCOPulseChanFreq( taskId, "Dev1/ctr3", "contador3", DAQmx_Val_Hz, DAQmx_Val_Low, InitialDelaycount3, Freqcount3, dutycyclecount3 ) );
    DAQmxErrChk( DAQmxCfgImplicitTiming( taskId, DAQmx_Val_FiniteSamps, nReadouts ) );
    DAQmxErrChk( DAQmxCfgDigEdgeStartTrig( taskId, "/Dev1/Ctr1InternalOutput", DAQmx_Val_Rising ) );
    DAQmxErrChk( DAQmxSetCOEnableInitialDelayOnRetrigger( taskId, "contador3", TRUE ) );
    DAQmxErrChk( DAQmxSetStartTrigRetriggerable( taskAcqGate, TRUE ) );
#endif
}
