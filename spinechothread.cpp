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

void SpinEchoThread::registerSamples( float64 * samples )
{
    for ( int i = 0; i < nsamples; i++ )
    {
        data[echo * 3 * nsamples + 3 * i] += samples[2*i];
        data[echo * 3 * nsamples + 3 * i+1] += samples[2*i+1];
        data[echo * 3 * nsamples + 3 * i+2] = sqrt( pow( data[echo * 3 * nsamples + 3 * i], 2 ) + pow( data[echo * 3 * nsamples + 3 * i+1], 2 ));
    }

    for ( int i = 0; i < nsamples; i++ )
    {
        seriesReal->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i] );
        seriesImag->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i+1] );
        seriesMod->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i+2] );
    }

    echo++;

    if ( echo == nechoes )
    {
        qDebug() << "updating UI";

        getParentWindow()->setChartSeriesReal(seriesReal);
        getParentWindow()->setChartSeriesImag(seriesImag);
        getParentWindow()->setChartSeriesMod(seriesMod);

        echo = 0;

        seriesReal = new QLineSeries();
        seriesImag = new QLineSeries();
        seriesMod = new QLineSeries();
    }
}

void SpinEchoThread::createExperiment()
{
    nechoes = Settings::getExperimentParameter( experiment, "nEchoes" ).toInt();
    nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();

    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    double t180 = Settings::getExperimentParameter( experiment, "T180" ).toDouble();
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

    taskRead = new TaskRead( "taskRead", samplingrate, nsamples, this );
    taskRead->createTask();

    parent->setModChartAxisDefault();
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

