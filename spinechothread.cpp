#include <stdio.h>

#include <QDebug>

#include "spinechothread.h"
#include "mainwindow.h"
#include "settings.h"

SpinEchoThread::SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent )
    : ExperimentThread( binDir, experiment, parent )
{
    qDebug() << "SpinEchoThread";

    seriesReal = seriesImag = seriesMod = NULL;
    data = NULL;
}

SpinEchoThread::~SpinEchoThread()
{
    if ( seriesReal != NULL )
        delete seriesReal;

    if ( seriesImag != NULL )
        delete seriesImag;

    if ( seriesMod != NULL )
        delete seriesMod;

    if ( data != NULL )
        delete data;
}


int32 CVICALLBACK SpinEchoThreadCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
    int32 read = 0;
    float64 * newdata = new float64[2 * nSamples];

    SpinEchoThread * thread = (SpinEchoThread*)callbackData;

#ifndef LINUX_BOX
    DAQmxReadAnalogF64( taskHandle, nSamples, 0.1, DAQmx_Val_GroupByScanNumber, newdata, 2 * nSamples, &read, NULL );
#endif

    bool ok = read == nSamples;

    qDebug() << "read echo " << (thread->echo+1) << (ok ? "ok" : "failed");

    if ( ok )
    {
        for ( int i = 0; i < nSamples; i++ )
        {
            thread->data[thread->echo * 3 * nSamples + 3 * i] += newdata[2*i];
            thread->data[thread->echo * 3 * nSamples + 3 * i+1] += newdata[2*i+1];
            thread->data[thread->echo * 3 * nSamples + 3 * i+2] = sqrt( pow( thread->data[thread->echo * 3 * nSamples + 3 * i], 2 ) + pow( thread->data[thread->echo * 3 * nSamples + 3 * i+1], 2 ));
        }

        for ( int i = 0; i < nSamples; i++ )
        {
            thread->seriesReal->append(thread->echo * nSamples + i, thread->data[thread->echo * 3 * nSamples + 3 * i] );
            thread->seriesImag->append(thread->echo * nSamples + i, thread->data[thread->echo * 3 * nSamples + 3 * i+1] );
            thread->seriesMod->append(thread->echo * nSamples + i, thread->data[thread->echo * 3 * nSamples + 3 * i+2] );
        }
    }

    delete newdata;

    thread->echo++;

    if ( thread->echo == thread->nechoes )
    {
        qDebug() << "updating UI";

        thread->getParentWindow()->setChartSeriesReal(thread->seriesReal);
        thread->getParentWindow()->setChartSeriesImag(thread->seriesImag);
        thread->getParentWindow()->setChartSeriesMod(thread->seriesMod);

        thread->echo = 0;

        thread->seriesReal = new QLineSeries();
        thread->seriesImag = new QLineSeries();
        thread->seriesMod = new QLineSeries();
    }

    return 0;
}

void SpinEchoThread::createExperiment()
{
    nechoes = Settings::getExperimentParameter( experiment, "nEchoes" ).toInt();

    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    double t180 = Settings::getExperimentParameter( experiment, "T180" ).toDouble();
    int nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();
    double techo = Settings::getExperimentParameter( experiment, "TEcho" ).toDouble();
    int32 nrepetitions = Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();
    int bandwidth = Settings::getExperimentParameter( experiment, "BandWidth" ).toInt();
    int samplingrate = Settings::getExperimentParameter( experiment, "SamplingRate" ).toInt();

    taskRepetitions = new TaskRepetitions( "taskRepetitions", tr, nrepetitions );
    taskRepetitions->createTask();

    taskRFGate = new TaskRFGate( "taskRFGate", t90, t180, techo, nechoes );
    taskRFGate->createTask();

    taskAcqGate = new TaskAcquisitionGate( "taskAcqGate", tr, t90, t180, techo, nechoes, nsamples, bandwidth, 0 );
    taskAcqGate->createTask();

    taskRead = new TaskRead( "taskRead", samplingrate, nsamples, SpinEchoThreadCallback, this );
    taskRead->createTask();
}

int SpinEchoThread::getProgressCount()
{
    return Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();;
}

int SpinEchoThread::getProgressTimer()
{
    return 1000 * Settings::getExperimentParameter( experiment, "TR" ).toDouble();
}

void SpinEchoThread::startExperiment()
{
    ExperimentThread::startExperiment();

    echo = 0;

    seriesReal = new QLineSeries();
    seriesImag = new QLineSeries();
    seriesMod = new QLineSeries();

    int nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();
    data = new float64[nechoes * 3 * nsamples];
    for ( int i = 0; i < nechoes * 3 * nsamples; i++)
        data[i] = 0;
}

