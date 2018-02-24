#include <QDebug>

#include "taskrfgate.h"

TaskRFGate::TaskRFGate( const QString & name, double t90, double t180, double techo, int nechoes )
    : ExperimentTask( name )
{
    this->t90 = t90;
    this->t180 = t180;
    this->techo = techo;
    this->nechoes = nechoes;
}

void TaskRFGate::createTask()
{
    ExperimentTask::createTask();

    float64 dataFreq[1024];
    float64 dataDC[1024];

    dataFreq[0] = 1 / t90;
    dataDC[0] = 0.9999;

    qDebug() << "dataFreq" << dataFreq[0] << dataDC[0];

    if ( nechoes > 0 )
    {
        dataFreq[1] = 1/ ((t180/2)+(techo/2)-(t90/2));
        dataDC[1] = t180 / ((t180/2)+(techo/2)-(t90/2));

        qDebug() << "dataFreq" << dataFreq[1] << dataDC[1];
    }

    int32 totalPulses = nechoes + 1;
    for ( int i = 2; i < totalPulses; i++ )
    {
        dataFreq[i] = 1 / techo;
        dataDC[i] = t180 / techo;

        qDebug() << "dataFreq" << dataFreq[i] << dataDC[i];
    }

#ifndef LINUX_BOX
    DAQmxErrChk( DAQmxCreateCOPulseChanFreq( taskId, "Dev1/ctr2", "", DAQmx_Val_Hz,DAQmx_Val_Low, InitialDelaycount2, dataFreq[0], dataDC[0] ) );
    DAQmxErrChk( DAQmxCfgImplicitTiming( taskId, DAQmx_Val_FiniteSamps, totalPulses ) );
    DAQmxErrChk( DAQmxCfgDigEdgeStartTrig( taskId, "/Dev1/Ctr1InternalOutput", DAQmx_Val_Rising ) );
    DAQmxErrChk( DAQmxSetStartTrigRetriggerable( taskId, TRUE ) );

    if ( nechoes > 0 )
        DAQmxErrChk( DAQmxWriteCtrFreq( taskId, totalPulses, 1, 10.0, DAQmx_Val_GroupByChannel, dataFreq, dataDC, NULL, NULL ) );
#endif
}
