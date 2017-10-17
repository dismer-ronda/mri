#include <stdio.h>

#include <QDebug>

#include "programmerthread.h"
#include "mainwindow.h"
#include "settings.h"

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);

ProgrammerThread::ProgrammerThread( QString binDir, Experiment * experiment, MainWindow * parent )
{
    this->binDir = binDir;
    this->parent = parent;
    this->experiment = experiment;

    /*QLineSeries * series = new QLineSeries();
    for ( int i = 0; i < 100; i++ )
        series->append(i, i * i);
    getParentWindow()->setChartSeries(series);*/
}

bool ProgrammerThread::isFinished()
{
    bool ret;
    mutex.lock();
    ret = finished;
    mutex.unlock();
    return ret;
}

void ProgrammerThread::setFinished( bool value )
{
    mutex.lock();
    finished = value;
    mutex.unlock();
}

void ProgrammerThread::run()
{
    qDebug() << "experiment thread started";

    setFinished(false);

    SpinEcho();

    while ( !isFinished() )
        QThread::msleep(100);

    if (taskRead != 0)
    {
       qDebug() << "stop task read";
       DAQmxStopTask (taskRead);
       DAQmxClearTask (taskRead);
    }

    if (taskTimer != 0)
    {
       qDebug() << "stop task timer";
       DAQmxStopTask (taskTimer);
       DAQmxClearTask (taskTimer);
    }

    if (taskRepetitions != 0)
    {
       qDebug() << "stop task Repetitions";
       DAQmxStopTask (taskRepetitions);
       DAQmxClearTask (taskRepetitions);
    }

    if (taskRFGate != 0)
    {
       qDebug() << "stop task RFGate";
       DAQmxStopTask (taskRFGate );
       DAQmxClearTask (taskRFGate );
    }

    if (taskAcqGate != 0)
    {
       qDebug() << "stop task AcqGate";
       DAQmxStopTask (taskAcqGate );
       DAQmxClearTask (taskAcqGate );
    }

    //delete dataDC;
    //delete dataFreq;

    parent->setFinished( true );

    qDebug() << "experiment thread finished";
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
    int32       read=0;
    float64     * data  = new float64[nSamples];

    ProgrammerThread * thread = (ProgrammerThread*)callbackData;

    qDebug() << "reading " << nSamples << " samples";
    DAQmxReadAnalogF64( taskHandle, nSamples, 10.0, DAQmx_Val_GroupByScanNumber, data, nSamples, &read, NULL );

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

void ProgrammerThread::SpinEcho()
{
    double tr = experiment->tR;
    double tpulse90 = experiment->t90;
    double tpulse180 = experiment->t180;
    int nexperiments = experiment->nEchoes;
    double nsamples1 = experiment->nSamples;
    double techo = experiment->tEcho;
    double timer = 50.0e+06;
    double sampleRate1 = 1024;

    double InitialDelaycount1 = 0;
    double InitialDelaycount2 = 0;
    double Freqcount1 = 1/tr;
    double dutycyclecount1 = 5e-03/tr;

    double dutyCycle180 = tpulse180/techo;
    double dutyCycle90 = tpulse90/tr;

    qDebug() << "tr = " << tr;
    qDebug() << "tpuse90 = " << tpulse90;
    qDebug() << "tpulse180 = " << tpulse180;
    qDebug() << "techo = " << techo;
    qDebug() << "nexperiments = " << nexperiments;
    qDebug() << "sampleRate = " << sampleRate1;
    qDebug() << "nsamples = " << nsamples1;
    qDebug() << "duty90 = " << dutyCycle90;
    qDebug() << "duty180 = " << dutyCycle180;

    taskRepetitions = taskRead = taskRFGate = taskTimer = taskAcqGate = 0;

   /* qDebug() << "create taskTimer 50 MHz";
    DAQmxCreateTask( "taskTimer", &taskTimer );
    DAQmxCreateCOPulseChanFreq( taskTimer,"Dev1/ctr0", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, timer, 0.50 );
    DAQmxCfgImplicitTiming( taskTimer,DAQmx_Val_ContSamps, 1000 ); */

    qDebug() << "create taskCounter repetitions";

    int32 Nrepetitions = 30;

    DAQmxCreateTask( "taskRepetitions", &taskRepetitions );
    DAQmxCreateCOPulseChanFreq( taskRepetitions, "Dev1/ctr1", "", DAQmx_Val_Hz, DAQmx_Val_Low, InitialDelaycount1, Freqcount1, dutycyclecount1 );
    DAQmxCfgImplicitTiming( taskRepetitions, DAQmx_Val_FiniteSamps, Nrepetitions );
    /* DAQmxCfgDigEdgeStartTrig( taskRepetitions, "/Dev1/Ctr0InternalOutput", DAQmx_Val_Rising ); */



    qDebug() << "create taskRFGate";

    dataFreq[0] = 1 / tpulse90;
    dataDC[0] = 0.999;
    qDebug() << dataFreq[0] << " " << dataDC[0];

    if ( nexperiments > 0 )   // si nexperiments es 0, entonces es un solo pulso (pulse-acquire)
    {
        dataFreq[1] = 1/ ((tpulse180/2)+(techo/2)-(tpulse90/2));
        dataDC[1] = tpulse180 / ((tpulse180/2)+(techo/2)-(tpulse90/2));
        qDebug() << dataFreq[1] << " " << dataDC[1];
    }



    int32 temp = nexperiments + 1;

    for( int i = 2; i < temp; i++ )
    {
        dataFreq[i] = 1 / (techo);
        dataDC[i] = tpulse180 / (techo);
        qDebug() << dataFreq[i] << " " << dataDC[i];
    }

    DAQmxCreateTask( "taskRFGate", &taskRFGate );
    DAQmxCreateCOPulseChanFreq(taskRFGate,"Dev1/ctr2","",DAQmx_Val_Hz,DAQmx_Val_Low,InitialDelaycount2,dataFreq[0],dataDC[0]);
    DAQmxCfgImplicitTiming( taskRFGate, DAQmx_Val_FiniteSamps, temp );
    DAQmxCfgDigEdgeStartTrig( taskRFGate,"/Dev1/Ctr1InternalOutput", DAQmx_Val_Rising );
    DAQmxSetStartTrigRetriggerable( taskRFGate, TRUE );

    int32 status = 1;  // If nexperiments = 0 (i.e. pulse acquire or GE), then no data is written to buffer, default pulse generated and status=1
    if ( nexperiments > 0 )
    {
        status = DAQmxWriteCtrFreq(taskRFGate,temp,1,10.0,DAQmx_Val_GroupByChannel,dataFreq,dataDC,NULL,NULL);
    }


    qDebug() << "start task RFGate " << temp << " written " << status << " status" ;



    qDebug() << "create task Acquisition Gate";  //******** Importante: Enforce tacq < spacing betwen RF pulses to protect receiver

    double tacq = 1e-03;
    double InitialDelaycount3 = 0;
    int32 nReadouts;
    if (nexperiments > 0)
    {
        InitialDelaycount3 = tpulse90/2 + techo - (tacq/2);
        nReadouts = nexperiments;
    }
    else
    {
        double ringdowndelay = 0.5e-03;
        InitialDelaycount3 = tpulse90 + ringdowndelay;
        nReadouts = 1;
    }


    double Freqcount3 = 1/techo;
    double dutycyclecount3 = tacq/techo;


    DAQmxCreateTask( "taskAcqGate", &taskAcqGate );
    DAQmxCreateCOPulseChanFreq( taskAcqGate, "Dev1/ctr3", "contador3", DAQmx_Val_Hz, DAQmx_Val_Low, InitialDelaycount3, Freqcount3, dutycyclecount3 );

    DAQmxCfgImplicitTiming( taskAcqGate, DAQmx_Val_FiniteSamps, nReadouts );
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
    DAQmxRegisterEveryNSamplesEvent(taskRead, DAQmx_Val_Acquired_Into_Buffer, nsamples1, 0, EveryNCallback, this );
    qDebug() << "start task read";

   /* DAQmxStartTask( taskTimer );*/   // el timer no es la mejor forma de sincronizartareas los pulsos son muy estrechos para trigger
    DAQmxStartTask( taskRead );
    DAQmxStartTask( taskRFGate );
    DAQmxStartTask( taskAcqGate );
    DAQmxStartTask( taskRepetitions );
}

MainWindow * ProgrammerThread::getParentWindow()
{
    return parent;
}
