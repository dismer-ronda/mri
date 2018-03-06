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
    fft = NULL;
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

    if ( fft != NULL )
        delete fft;

    if ( model != NULL )
        delete model;
}

void SpinEchoThread::registerSamples( float64 * samples )
{
    for ( int i = 0; i < nsamples; i++ )
    {
        data[echo * 2 * nsamples + 2 * i] += samples[2*i];
        data[echo * 2 * nsamples + 2 * i+1] += samples[2*i+1];

        series1->append(echo * nsamples + i, data[echo * 2 * nsamples + 2 * i] );
        series2->append(echo * nsamples + i, data[echo * 2 * nsamples + 2 * i+1] );

        series3->append(echo * nsamples + i, sqrt( pow( data[echo * 2 * nsamples + 2 * i], 2 ) + pow( data[echo * 2 * nsamples + 2 * i+1], 2 ) ) );
    }

    float64 * temp = new float64[2 * nsamples];
    float64 * temp1 = new float64[nsamples];

    for ( int i = 0; i < nsamples; i++ )
    {
        temp[2*i] = data[echo * 2 * nsamples + 2 * i];
        temp[2*i+1] = data[echo * 2 * nsamples + 2 * i+1];
    }
    fftw( temp, nsamples, zeroOffset );
    for ( int i = 0; i < nsamples; i++ )
    {
#ifdef SIMULATION
        temp1[i] = (echo == 0 ? 1 : exp( -echo * techo/0.08 ) ) * sqrt( pow( temp[2 * i], 2 ) + pow( temp[2 * i+1], 2 ) );
#else
        temp1[i] = sqrt( pow( temp[2 * i], 2 ) + pow( temp[2 * i+1], 2 ) );
#endif
        fft[echo * nsamples + i] = temp1[i];

        series4->append(echo * nsamples + i, fft[echo * nsamples + i] );
    }

    if ( echo == 0 )
    {
        float64 max;
        int pos;
        findMaxPos( temp1, nsamples, max, pos );

        model[echo] = getAreaUnderMax( temp1, nsamples, echoFactor, max, pos, posMin, posMax );
    }
    else
        model[echo] = getAreaUnderRegion( temp1, posMin, posMax );

    series5->append(echo, model[echo] );

    delete temp;
    delete temp1;

    echo++;

    if ( echo == nechoes )
    {
        qDebug() << "updating UI";

        if ( graph1.compare( "real" ) == 0 )
            getParentWindow()->setChartSeries1( series1 );
        else if ( graph1.compare( "imag" ) == 0 )
            getParentWindow()->setChartSeries1( series2 );
        else if ( graph1.compare( "mod" ) == 0 )
            getParentWindow()->setChartSeries1( series3 );
        else if ( graph1.compare( "fft" ) == 0 )
            getParentWindow()->setChartSeries1( series4 );
        else if ( graph1.compare( "model fit" ) == 0 )
            getParentWindow()->setChartSeries1( series5 );

        if ( graph2.compare( "real" ) == 0 )
            getParentWindow()->setChartSeries2( series1 );
        else if ( graph2.compare( "imag" ) == 0 )
            getParentWindow()->setChartSeries2( series2 );
        else if ( graph2.compare( "mod" ) == 0 )
            getParentWindow()->setChartSeries2( series3 );
        else if ( graph2.compare( "fft" ) == 0 )
            getParentWindow()->setChartSeries2( series4 );
        else if ( graph2.compare( "model fit" ) == 0 )
            getParentWindow()->setChartSeries2( series5 );

        if ( graph3.compare( "real" ) == 0 )
            getParentWindow()->setChartSeries3( series1 );
        else if ( graph3.compare( "imag" ) == 0 )
            getParentWindow()->setChartSeries3( series2 );
        else if ( graph3.compare( "mod" ) == 0 )
            getParentWindow()->setChartSeries3( series3 );
        else if ( graph3.compare( "fft" ) == 0 )
            getParentWindow()->setChartSeries3( series4 );
        else if ( graph3.compare( "model fit" ) == 0 )
            getParentWindow()->setChartSeries3( series5 );

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
    techo = Settings::getExperimentParameter( experiment, "TEcho" ).toDouble();
    echoFactor = Settings::getExperimentParameter( experiment, "EchoFactor" ).toDouble();

    double tr = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    double t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    double t180 = Settings::getExperimentParameter( experiment, "T180" ).toDouble();
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

    if ( graph1.compare( "real" ) == 0 )
        parent->setChart1AxisDefault();
    else if ( graph1.compare( "imag" ) == 0 )
        parent->setChart1AxisDefault();
    else if ( graph1.compare( "mod" ) == 0 )
        parent->setChart1AxisDefault();
    else if ( graph1.compare( "fft" ) == 0 )
        parent->setChart1Axis( getFftAxis() );

    if ( graph2.compare( "real" ) == 0 )
        parent->setChart2AxisDefault();
    else if ( graph2.compare( "imag" ) == 0 )
        parent->setChart2AxisDefault();
    else if ( graph2.compare( "mod" ) == 0 )
        parent->setChart2AxisDefault();
    else if ( graph2.compare( "fft" ) == 0 )
        parent->setChart2Axis( getFftAxis() );

    if ( graph3.compare( "real" ) == 0 )
        parent->setChart3AxisDefault();
    else if ( graph3.compare( "imag" ) == 0 )
        parent->setChart3AxisDefault();
    else if ( graph3.compare( "mod" ) == 0 )
        parent->setChart3AxisDefault();
    else if ( graph3.compare( "fft" ) == 0 )
        parent->setChart3Axis( getFftAxis() );
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
    data = new float64[nechoes * 2 * nsamples];
    for ( int i = 0; i < nechoes * 2 * nsamples; i++)
        data[i] = 0;

    fft = new float64[nechoes * nsamples];
    model = new float64[nechoes * nsamples];
}

