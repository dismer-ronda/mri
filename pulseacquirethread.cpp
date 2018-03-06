#include <stdio.h>
#include <math.h>

#include <QDebug>
#include <QValueAxis>

#include "pulseacquirethread.h"
#include "mainwindow.h"
#include "settings.h"

PulseAcquireThread::PulseAcquireThread( QString binDir, const QString & experiment, MainWindow * parent )
    : ExperimentThread( binDir, experiment, parent )
{
    qDebug() << "PulseAcquireThread";
}

void PulseAcquireThread::registerSamples( float64 * samples )
{
    QLineSeries * series1 = new QLineSeries();
    QLineSeries * series2 = new QLineSeries();
    QLineSeries * series3 = new QLineSeries();
    QLineSeries * series4 = new QLineSeries();

    for ( int i = 0; i < nsamples; i++ )
    {
        data[2 * i] += samples[2*i];
        series1->append(i, data[2 * i] );
    }

    for ( int i = 0; i < nsamples; i++ )
    {
        data[2 * i+1] += samples[2*i+1];
        series2->append(i, data[2 * i+1] );
    }

    for ( int i = 0; i < nsamples; i++ )
        series3 ->append(i, sqrt( pow(data[2 * i], 2) + pow(data[2 * i+1], 2) ) );

    fftw( samples, nsamples, zeroOffset );
    for ( int i = 0; i < nsamples; i++ )
        series4->append(i, sqrt( pow(samples[2 * i], 2) + pow(samples[2 * i+1], 2) ) );

    if ( graph1.compare( "real" ) )
        getParentWindow()->setChartSeries1( series1 );
    else if ( graph1.compare( "imag" ) )
        getParentWindow()->setChartSeries1( series2 );
    else if ( graph1.compare( "mod" ) )
        getParentWindow()->setChartSeries1( series3 );
    else if ( graph1.compare( "fft" ) )
        getParentWindow()->setChartSeries1( series4 );

    if ( graph2.compare( "real" ) )
        getParentWindow()->setChartSeries2( series1 );
    else if ( graph2.compare( "imag" ) )
        getParentWindow()->setChartSeries2( series2 );
    else if ( graph2.compare( "mod" ) )
        getParentWindow()->setChartSeries2( series3 );
    else if ( graph2.compare( "fft" ) )
        getParentWindow()->setChartSeries2( series4 );

    if ( graph3.compare( "real" ) )
        getParentWindow()->setChartSeries3( series1 );
    else if ( graph3.compare( "imag" ) )
        getParentWindow()->setChartSeries3( series2 );
    else if ( graph3.compare( "mod" ) )
        getParentWindow()->setChartSeries3( series3 );
    else if ( graph3.compare( "fft" ) )
        getParentWindow()->setChartSeries3( series4 );

    delete series1;
    delete series2;
    delete series3;
    delete series4;
}

void PulseAcquireThread::createExperiment()
{
    ExperimentThread::createExperiment();

    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
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

    taskRead = new TaskRead( "taskRead", samplingrate, nsamples, this );
    taskRead->createTask();

#ifndef LINUX_BOX
    if ( module.compare("fft" ) == 0 )
    {
        QValueAxis * axis = new QValueAxis();
        axis->setMin( -bandwidth/2 );
        axis->setMax( bandwidth/2 );

        parent->setChart3Axis( axis );
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

