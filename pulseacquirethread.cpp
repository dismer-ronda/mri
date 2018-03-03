#include <stdio.h>
#include <math.h>

#include <QDebug>
#include <QValueAxis>

#include "pulseacquirethread.h"
#include "mainwindow.h"
#include "settings.h"

#include "fftw3.h"

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

void PulseAcquireThread::registerSamples( float64 * samples )
{
    QLineSeries * seriesReal = new QLineSeries();
    QLineSeries * seriesImag = new QLineSeries();
    QLineSeries * seriesMod = new QLineSeries();

    for ( int i = 0; i < nsamples; i++ )
    {
        data[2 * i] += samples[2*i];
        seriesReal->append(i, data[2 * i] );
    }

    for ( int i = 0; i < nsamples; i++ )
    {
        data[2 * i+1] += samples[2*i+1];
        seriesImag->append(i, data[2 * i+1] );
    }

#ifndef LINUX_BOX
    if ( module.compare("fft" ) == 0 )
    {
        fftw( samples, nSamples );

        for ( int i = 0; i < nsamples; i++ )
            seriesMod->append(i, sqrt( pow(samples[2 * i], 2) + pow(samples[2 * i+1], 2) ) );
    }
    else
#endif
    {
        for ( int i = 0; i < nsamples; i++ )
            seriesMod->append(i, sqrt( pow(data[2 * i], 2) + pow(data[2 * i+1], 2) ) );
    }

    getParentWindow()->setChartSeriesReal(seriesReal);
    getParentWindow()->setChartSeriesImag(seriesImag);
    getParentWindow()->setChartSeriesMod(seriesMod);
}

void PulseAcquireThread::createExperiment()
{
    nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();

    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    double techo = Settings::getExperimentParameter( experiment, "TEcho" ).toDouble();
    int32 nrepetitions = Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();
    int bandwidth = Settings::getExperimentParameter( experiment, "BandWidth" ).toInt();
    int samplingrate = Settings::getExperimentParameter( experiment, "SamplingRate" ).toInt();
    double ringdowndelay = Settings::getExperimentParameter( experiment, "RingDownDelay" ).toInt();

    module = Settings::getExperimentParameter( experiment, "Module" ).toString();

    taskRepetitions = new TaskRepetitions( "taskRepetitions", tr, nrepetitions );
    taskRepetitions->createTask();

    taskRFGate = new TaskRFGate( "taskRFGate", t90, 0, techo, 0 );
    taskRFGate->createTask();

    taskAcqGate = new TaskAcquisitionGate( "taskAcqGate", tr, t90, 0, techo, 0, nsamples, bandwidth, ringdowndelay );
    taskAcqGate->createTask();

    taskRead = new TaskRead( "taskRead", samplingrate, nsamples, this );
    taskRead->createTask();

#ifndef LINUX_BOX
    if ( module.compare("fft" ) == 0 )
    {
        QValueAxis * axis = new QValueAxis();
        axis->setMin( -bandwidth/2 );
        axis->setMax( bandwidth/2 );

        parent->setModChartAxis( axis );
    }
#endif
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

