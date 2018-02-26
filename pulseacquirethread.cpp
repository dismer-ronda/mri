#include <stdio.h>

#include <QDebug>

#include "pulseacquirethread.h"
#include "mainwindow.h"
#include "settings.h"

PulseAcquireThread::PulseAcquireThread( QString binDir, const QString & experiment, MainWindow * parent )
    : ExperimentThread( binDir, experiment, parent )
{
    qDebug() << "PulseAcquireThread";
    data = NULL;
}

PulseAcquireThread::~PulseAcquireThread()
{
    if ( data != NULL )
        delete data;
}

int32 CVICALLBACK PulseAcquireThreadCallback( TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData )
{
    int32 read = 0;
    float64 * newdata = new float64[2 * nSamples];

    PulseAcquireThread * thread = (PulseAcquireThread*)callbackData;

#ifndef LINUX_BOX
    DAQmxReadAnalogF64( taskHandle, nSamples, 0.1, DAQmx_Val_GroupByScanNumber, newdata, 2 * nSamples, &read, NULL );
#endif

    bool ok = read == nSamples;

    qDebug() << "read " << (ok ? "ok" : "failed");

    if ( ok )
    {
        QLineSeries * seriesReal = new QLineSeries();
        QLineSeries *seriesImag = new QLineSeries();

        for ( int i = 0; i < nSamples; i++ )
        {
            thread->data[2 * i] += newdata[2*i];
            seriesReal->append(i, thread->data[2 * i] );
        }

        for ( int i = 0; i < nSamples; i++ )
        {
            thread->data[2 * i+1] += newdata[2*i+1];
            seriesImag->append(i, thread->data[2 * i+1] );
        }

        thread->getParentWindow()->setChartSeriesReal(seriesReal);
        thread->getParentWindow()->setChartSeriesImag(seriesImag);
    }

    delete newdata;

    return 0;
}

void PulseAcquireThread::createExperiment()
{
    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    int nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();
    double techo = Settings::getExperimentParameter( experiment, "TEcho" ).toDouble();
    int32 nrepetitions = Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();
    int bandwidth = Settings::getExperimentParameter( experiment, "BandWidth" ).toInt();
    int samplingrate = Settings::getExperimentParameter( experiment, "SamplingRate" ).toInt();
    double ringdowndelay = Settings::getExperimentParameter( experiment, "RingDownDelay" ).toInt();

    taskRepetitions = new TaskRepetitions( "taskRepetitions", tr, nrepetitions );
    taskRepetitions->createTask();

    taskRFGate = new TaskRFGate( "taskRFGate", t90, 0, techo, 0 );
    taskRFGate->createTask();

    taskAcqGate = new TaskAcquisitionGate( "taskAcqGate", tr, t90, 0, techo, 0, nsamples, bandwidth, ringdowndelay );
    taskAcqGate->createTask();

    taskRead = new TaskRead( "taskRead", samplingrate, nsamples, PulseAcquireThreadCallback, this );
    taskRead->createTask();
}

int PulseAcquireThread::getProgressCount()
{
    return Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();
}

int PulseAcquireThread::getProgressTimer()
{
    return 1000 * Settings::getExperimentParameter( experiment, "TR" ).toDouble();
}


void PulseAcquireThread::startExperiment()
{
    ExperimentThread::startExperiment();

    int nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();

    data = new float64[2 * nsamples];
    for ( int i = 0; i < 2 * nsamples; i++)
        data[i] = 0;
}

