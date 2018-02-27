#include <stdio.h>
#include <math.h>

#include <QDebug>

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

#ifndef LINUX_BOX
void fftw( float64 * data, int nsamples )
{
    fftw_complex *in, *out;
    fftw_plan p;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nsamples);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nsamples);

    for ( int i = 0; i < nsamples; i++ )
    {
        in[i][0] = data[2*i];
        in[i][1] = data[2*i+1];
    }

    p = fftw_plan_dft_1d(nsamples, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);

    for ( int i = 0; i < nsamples; i++ )
    {
        data[2*i] = out[i][0];
        data[2*i+1] = out[i][1];
    }

    fftw_free(in);
    fftw_free(out);
}
#endif

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
        QLineSeries * seriesImag = new QLineSeries();
        QLineSeries * seriesMod = new QLineSeries();

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

#ifndef LINUX_BOX
        if ( thread->module.compare("fft" ) == 0 )
        {
            fftw( newdata, nSamples );

            for ( int i = 0; i < nSamples/2; i++ )
            {
                float64 tempr = newdata[2*i];
                float64 tempi = newdata[2*i+1];

                newdata[2*i] = newdata[nSamples + 2*i];
                newdata[2*i+1] = newdata[nSamples + 2*i+1];

                newdata[nSamples + 2*i] = tempr;
                newdata[nSamples + 2*i+1] = tempi;
            }

            for ( int i = 0; i < nSamples; i++ )
                seriesMod->append(i, sqrt( pow(newdata[2 * i],2) + pow(newdata[2 * i+1],2) ) );
        }
        else
#endif
        {
            for ( int i = 0; i < nSamples; i++ )
                seriesMod->append(i, sqrt( pow(thread->data[2 * i],2) + pow(thread->data[2 * i+1],2) ) );
        }

        thread->getParentWindow()->setChartSeriesReal(seriesReal);
        thread->getParentWindow()->setChartSeriesImag(seriesImag);
        thread->getParentWindow()->setChartSeriesMod(seriesMod);

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

    module = Settings::getExperimentParameter( experiment, "Module" ).toString();

    taskRepetitions = new TaskRepetitions( "taskRepetitions", tr, nrepetitions );
    taskRepetitions->createTask();

    taskRFGate = new TaskRFGate( "taskRFGate", t90, 0, techo, 0 );
    taskRFGate->createTask();

    taskAcqGate = new TaskAcquisitionGate( "taskAcqGate", tr, t90, 0, techo, 0, nsamples, bandwidth, ringdowndelay );
    taskAcqGate->createTask();

    taskRead = new TaskRead( "taskRead", samplingrate, nsamples, PulseAcquireThreadCallback, this );
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

