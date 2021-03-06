#include <stdio.h>
#include <math.h>

#include <QDebug>
#include <QValueAxis>
#include <QDateTime>

#include "spinechothread.h"
#include "mainwindow.h"
#include "settings.h"
#include "messagedlg.h"

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

    if ( echoMagnitude != NULL )
        delete echoMagnitude;
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
        if ( simulation )
            temp1[i] = (echo == 0 ? 1 : exp( -echo * techo/0.08 ) ) * sqrt( pow( temp[2 * i], 2 ) + pow( temp[2 * i+1], 2 ) );
        else
            temp1[i] = sqrt( pow( temp[2 * i], 2 ) + pow( temp[2 * i+1], 2 ) );

        fft[echo * nsamples + i] = temp1[i];

        series4->append(echo * nsamples + i, fft[echo * nsamples + i] );
    }

    if ( echo == 0 )
    {
        float64 max;
        int pos;
        findMaxPos( temp1, nsamples, max, pos );

        echoMagnitude[echo] = getAreaUnderMax( temp1, nsamples, echoFactor, max, pos, posMin, posMax );
    }
    else
        echoMagnitude[echo] = getAreaUnderRegion( temp1, posMin, posMax );

    series5->append(echo, echoMagnitude[echo] );

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
    simulation = Settings::getExperimentParameter( experiment, "Simulation" ).toBool();
    name = Settings::getExperimentParameter( experiment, "Name" ).toString();

    trep = Settings::getExperimentParameter( experiment, "TR" ).toDouble();
    t90 = Settings::getExperimentParameter( experiment, "T90" ).toDouble();
    t180 = Settings::getExperimentParameter( experiment, "T180" ).toDouble();
    nrepetitions = Settings::getExperimentParameter( experiment, "nRepetitions" ).toInt();
    bandwidth = Settings::getExperimentParameter( experiment, "BandWidth" ).toInt();
    samplingrate = Settings::getExperimentParameter( experiment, "SamplingRate" ).toInt();

    taskRepetitions = new TaskRepetitions( "taskRepetitions", trep, nrepetitions );
    taskRepetitions->createTask();

    taskRFGate = new TaskRFGate( "taskRFGate", t90, t180, techo, nechoes );
    taskRFGate->createTask();

    taskAcqGate = new TaskAcquisitionGate( "taskAcqGate", trep, t90, t180, techo, nechoes, nsamples, bandwidth, 0 );
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
    echoMagnitude = new float64[nechoes * nsamples];
}

float64 SpinEchoThread::calculateT2()
{
    float64 mx = 0;
    float64 my = 0;

    for ( int i = 0; i < nechoes; i++ )
    {
        mx += i * techo;
        my += log( echoMagnitude[i] );
    }

    mx = mx / nechoes;
    my = my / nechoes;

    float64 sum1 = 0;
    for ( int i = 0; i < nechoes; i++ )
        sum1 += (i*techo - mx) * (log(echoMagnitude[i]) - my);

    float64 sum2 = 0;
    for ( int i = 0; i < nechoes; i++ )
        sum2 += pow( i*techo - mx, 2 );

    qDebug() << "sum1=" << sum1;
    qDebug() << "sum2=" << sum2;

    float64 t2 = sum1 != 0 ? -sum2/sum1 : 0;

    return t2;
}

QDialog * SpinEchoThread::getResultDialog()
{
    if ( nechoes > 1 )
    {
        float64 t2 = calculateT2();

        QDateTime date = QDateTime::currentDateTime();

#ifdef LINUX_BOX
        QString separator = "/";
#else
        QString separator = "\\";
#endif
        QString header = "";
        header.append(QString("nechoes=%1\n").arg( nechoes ) );
        header.append(QString("techo=%1\n").arg( techo ) );
        header.append(QString("tr=%1\n").arg( trep ) );
        header.append(QString("t90=%1\n").arg( t90 ) );
        header.append(QString("t180=%1\n").arg( t180 ) );
        header.append(QString("nrepetitions=%1\n").arg( nrepetitions ) );
        header.append(QString("bandwidth=%1\n").arg( bandwidth ) );
        header.append(QString("samplingrate=%1\n").arg( samplingrate ) );
        header.append(QString("nsamples=%1\n").arg( nsamples) );
        header.append(QString("t2=%1\n").arg( t2 ) );

        writeRawDataFile( QString( "%1%2%3-raw-%4.txt" ).arg(outputDir).arg(separator).arg(name).arg( date.toString("yyyy-MM-dd-hh-mm-ss" ) ), header, data, nsamples );
        writeFFTDataFile( QString( "%1%2%3-fft-%4.txt" ).arg(outputDir).arg(separator).arg(name).arg( date.toString("yyyy-MM-dd-hh-mm-ss" ) ), header, fft, nsamples );
        writeEchoMagnitude( QString( "%1%2%3-eco-%4.txt" ).arg(outputDir).arg(separator).arg(name).arg( date.toString("yyyy-MM-dd-hh-mm-ss" ) ), header, echoMagnitude, nechoes );

        MessageDialog * dlg = new MessageDialog( parent );
        dlg->initComponents( QString( "T2 = %1" ).arg( t2 ) );
        return dlg;
    }
    else
        return NULL;
}
