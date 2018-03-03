#ifndef SpinEchoThread_H
#define SpinEchoThread_H

#include <QThread>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experiment.h"

class SpinEchoThread : public ExperimentThread
{
    Q_OBJECT

public:
    float64 * data;

    QLineSeries * seriesReal;
    QLineSeries * seriesImag;
    QLineSeries * seriesMod;

    int echo;
    int nechoes;
    int nsamples;

    SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent );
    virtual ~SpinEchoThread();

    virtual void createExperiment();
    virtual void startExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();

    virtual void registerSamples( float64 * samples );
};

#endif // SpinEchoThread_H
