#include <stdio.h>

#include <QDebug>

#include "pulseacquirethread.h"
#include "mainwindow.h"
#include "settings.h"

PulseAcquireThread::PulseAcquireThread( QString binDir, const QString & experiment, MainWindow * parent )
    : ExperimentThread( binDir, experiment, parent )
{
    qDebug() << "PulseAcquireThread";
}

int32 CVICALLBACK PulseAcquireThreadCallback( TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData )
{
    int32       read=0;
    float64     * data  = new float64[nSamples];

    ExperimentThread * thread = (ExperimentThread*)callbackData;

    qDebug() << "reading " << nSamples << " samples";
#ifndef LINUX_BOX
    DAQmxReadAnalogF64( taskHandle, nSamples, 0.1, DAQmx_Val_GroupByScanNumber, data, nSamples, &read, NULL );
#endif

    bool ok = read != nSamples;

    if ( read > 0 )
    {
        QLineSeries * series = new QLineSeries();
        for ( int i = 0; i < read; i++ )
            series->append(i, data[i]);

        qDebug() << "notifying UI";
        thread->getParentWindow()->setChartSeries(series);
    }

    qDebug() << ( ok ? "Menos datos de los esperados" : "Ok");

    return 0;
}

void PulseAcquireThread::createExperiment()
{
    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    double nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();
    double techo = Settings::getExperimentParameter( experiment, "TEcho" ).toDouble();
    int32 nrepetitions = Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();

    taskRepetitions = new TaskRepetitions( "taskRepetitions", tr, nrepetitions );
    taskRepetitions->createTask();

    taskRFGate = new TaskRFGate( "taskRFGate", t90, 0, techo, 1 );
    taskRepetitions->createTask();

    taskAcqGate = new TaskAcquisitionGate( "taskAcqGate", t90, techo, 1 );
    taskAcqGate->createTask();

    taskRead = new TaskRead( "taskRead", 100e+03, nsamples, PulseAcquireThreadCallback );
    taskRead->createTask();

/*    taskRepetitions = taskRead = taskRFGate = taskTimer = taskAcqGate = 0;

    //qDebug() << "create taskTimer 50 MHz";
    //DAQmxCreateTask( "taskTimer", &taskTimer );
    //DAQmxCreateCOPulseChanFreq( taskTimer,"Dev1/ctr0", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, timer, 0.50 );
    //DAQmxCfgImplicitTiming( taskTimer,DAQmx_Val_ContSamps, 1000 );

    qDebug() << "create taskCounter repetitions";



    DAQmxCreateTask( "taskRepetitions", &taskRepetitions );
    DAQmxCreateCOPulseChanFreq( taskRepetitions, "Dev1/ctr1", "", DAQmx_Val_Hz, DAQmx_Val_Low, InitialDelaycount1, Freqcount1, dutycyclecount1 );
    DAQmxCfgImplicitTiming( taskRepetitions, DAQmx_Val_FiniteSamps, nrepetitions );
    // DAQmxCfgDigEdgeStartTrig( taskRepetitions, "/Dev1/Ctr0InternalOutput", DAQmx_Val_Rising );


    qDebug() << "create taskRFGate";

    dataFreq[0] = 1 / tpulse90;
    dataDC[0] = 0.9999;
    qDebug() << dataFreq[0] << " " << dataDC[0];

    DAQmxCreateTask( "taskRFGate", &taskRFGate );
    DAQmxCreateCOPulseChanFreq(taskRFGate,"Dev1/ctr2","",DAQmx_Val_Hz,DAQmx_Val_Low,InitialDelaycount2,dataFreq[0],dataDC[0]);
    DAQmxCfgImplicitTiming( taskRFGate, DAQmx_Val_FiniteSamps, 1 );
    DAQmxCfgDigEdgeStartTrig( taskRFGate,"/Dev1/Ctr1InternalOutput", DAQmx_Val_Rising );
    DAQmxSetStartTrigRetriggerable( taskRFGate, TRUE );

    qDebug() << "create task Acquisition Gate";  // ******** Importante: Enforce tacq < spacing betwen RF pulses to protect receiver

    double tacq = 1e-03;
    double InitialDelaycount3 = 0;
    double ringdowndelay = 0.5e-03;
    InitialDelaycount3 = tpulse90 + ringdowndelay;

    double Freqcount3 = 1/techo;
    double dutycyclecount3 = tacq/techo;


    DAQmxCreateTask( "taskAcqGate", &taskAcqGate );
    DAQmxCreateCOPulseChanFreq( taskAcqGate, "Dev1/ctr3", "contador3", DAQmx_Val_Hz, DAQmx_Val_Low, InitialDelaycount3, Freqcount3, dutycyclecount3 );

    DAQmxCfgImplicitTiming( taskAcqGate, DAQmx_Val_FiniteSamps, 1 );
    DAQmxCfgDigEdgeStartTrig( taskAcqGate, "/Dev1/Ctr1InternalOutput", DAQmx_Val_Rising );
    int32 status2 = DAQmxSetCOEnableInitialDelayOnRetrigger(taskAcqGate,"contador3", TRUE);
    DAQmxSetStartTrigRetriggerable( taskAcqGate, TRUE );

    qDebug() << "start task AcqGate " << status2 << " status" ;

    sampleRate1 = 100e+03;
    nsamples1 = 100;

    qDebug() << "create taskRead";
    DAQmxCreateTask( "taskRead", &taskRead );
    DAQmxCreateAIVoltageChan( taskRead ,"Dev1/ai1", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL );
    DAQmxCfgSampClkTiming( taskRead, "", sampleRate1, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, nsamples1 );
    DAQmxCfgDigEdgeStartTrig( taskRead, "/Dev1/Ctr3InternalOutput", DAQmx_Val_Rising );
    DAQmxSetStartTrigRetriggerable( taskRead, TRUE );
    DAQmxRegisterEveryNSamplesEvent(taskRead, DAQmx_Val_Acquired_Into_Buffer, nsamples1, 0, PulseAcquireThreadCallback, this );
    qDebug() << "start task read";

   // DAQmxStartTask( taskTimer );   // el timer no es la mejor forma de sincronizartareas los pulsos son muy estrechos para trigger
    DAQmxStartTask( taskRead );
    DAQmxStartTask( taskRFGate );
    DAQmxStartTask( taskAcqGate );
    DAQmxStartTask( taskRepetitions );
*/
}

int PulseAcquireThread::getProgressCount()
{
    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    int32 nrepetitions = Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();

    return tr * nrepetitions;
}

int PulseAcquireThread::getProgressTimer()
{
    return 1000 * Settings::getExperimentParameter( experiment, "TR" ).toDouble();
}
