#include "taskacquisitiongate.h"

TaskAcquisitionGate::TaskAcquisitionGate( const QString & name, double tr, double t90, double t180, double techo, int nechoes, int nsamples, int bandwidth, double ringdowndelay )
    : ExperimentTask( name )
{
    this->tr = tr;
    this->t90 = t90;
    this->t180 = t180;
    this->techo = techo;
    this->nechoes = nechoes;
    this->nsamples = nsamples;
    this->bandwidth = bandwidth;
    this->ringdowndelay = ringdowndelay;
}

void TaskAcquisitionGate::createTask()
{
    ExperimentTask::createTask();

    double tacq = (double)nsamples/(double)bandwidth;
    double InitialDelaycount3 = 0;
    int32 nReadouts;

    double Freqcount3 = 1/tr;
    double dutycycle = tacq/tr;

    if ( nechoes > 0 )
    {
        InitialDelaycount3 = t90/2 + techo - (tacq/2);
        nReadouts = nechoes;

        Freqcount3 = 1/techo;
        dutycycle = tacq/techo;
    }
    else
    {
        InitialDelaycount3 = t90 + ringdowndelay;
        nReadouts = 1;
    }

    if ( dutycycle > 1 )
        throw ExperimentException( ExperimentException::ERROR_INVALID_ACQ_DURATION );

    qDebug() << "dutycyclecount3=" << dutycycle;

#ifndef LINUX_BOX
    DAQmxErrChk( "DAQmxCreateCOPulseChanFreq", DAQmxCreateCOPulseChanFreq( taskId, "Dev1/ctr3", "contador3", DAQmx_Val_Hz, DAQmx_Val_Low, InitialDelaycount3, Freqcount3, dutycycle ) );
    DAQmxErrChk( "DAQmxCfgImplicitTiming", DAQmxCfgImplicitTiming( taskId, DAQmx_Val_FiniteSamps, nReadouts ) );
    DAQmxErrChk( "DAQmxCfgDigEdgeStartTrig", DAQmxCfgDigEdgeStartTrig( taskId, "/Dev1/Ctr1InternalOutput", DAQmx_Val_Rising ) );
    DAQmxErrChk( "DAQmxSetCOEnableInitialDelayOnRetrigger", DAQmxSetCOEnableInitialDelayOnRetrigger( taskId, "contador3", TRUE ) );
    DAQmxErrChk( "DAQmxSetStartTrigRetriggerable", DAQmxSetStartTrigRetriggerable( taskId, TRUE ) );
#endif
}
