#include <stdio.h>

#include <QDebug>
#include <QValueAxis>

#include "spinechothread.h"
#include "mainwindow.h"
#include "settings.h"

SpinEchoThread::SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent )
    : ExperimentThread( binDir, experiment, parent )
{
    qDebug() << "SpinEchoThread";

    series1 = series2 = series3 = series4 = series5 = series6 = NULL;
}

SpinEchoThread::~SpinEchoThread()
{
    if ( series1 != NULL )
        delete series1;

    if ( series2 != NULL )
        delete series2;

    if ( series3 != NULL )
        delete series3;

    if ( series4 != NULL )
        delete series4;

    if ( series5 != NULL )
        delete series5;

    if ( series6 != NULL )
        delete series6;
}

void SpinEchoThread::registerSamples( float64 * samples )
{
    for ( int i = 0; i < nsamples; i++ )
    {
        data[echo * 3 * nsamples + 3 * i] += samples[2*i];
        data[echo * 3 * nsamples + 3 * i+1] += samples[2*i+1];
        data[echo * 3 * nsamples + 3 * i+2] += sqrt( pow( data[echo * 3 * nsamples + 3 * i], 2 ) + pow( data[echo * 3 * nsamples + 3 * i+1], 2 ) );

        series1->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i] );
        series2->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i+1] );
        series3->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i+2] );
    }

    float64 * temp = new float64[nsamples*2];

    for ( int i = 0; i < nsamples; i++ )
    {
        temp[2*i] = data[echo * 3 * nsamples + 3 * i];
        temp[2*i+1] = data[echo * 3 * nsamples + 3 * i+1];
    }

    fftw( temp, nsamples, zeroOffset );

    for ( int i = 0; i < nsamples; i++ )
    {
        data[echo * 3 * nsamples + 3 * i+2] = sqrt( pow( temp[2 * i], 2 ) + pow( temp[2 * i+1], 2 ));
        series4->append(echo * nsamples + i, data[echo * 3 * nsamples + 3 * i+2] );
    }

    delete temp;

    echo++;

    if ( echo == nechoes )
    {
        qDebug() << "updating UI";

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

        echo = 0;

        series1->clear();
        series2->clear();
        series3->clear();
        series4->clear();
        series5->clear();
        series6->clear();
    }
}

void SpinEchoThread::createExperiment()
{
    ExperimentThread::createExperiment();

    nechoes = Settings::getExperimentParameter( experiment, "nEchoes" ).toInt();
    outputDir = Settings::getSetting( "OutputDirectory", "c:\\" ).toString();

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

    QValueAxis * axis = new QValueAxis();
    axis->setMin( 0 );
    axis->setMax( nechoes * techo );

    parent->setChart3Axis( axis );
    //parent->setModChartAxisDefault();
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

    series1 = new QLineSeries();
    series2 = new QLineSeries();
    series3 = new QLineSeries();
    series4 = new QLineSeries();
    series5 = new QLineSeries();
    series6 = new QLineSeries();

    int nsamples = Settings::getExperimentParameter( experiment, "nSamples" ).toInt();
    data = new float64[nechoes * 3 * nsamples];
    for ( int i = 0; i < nechoes * 3 * nsamples; i++)
        data[i] = 0;
}

