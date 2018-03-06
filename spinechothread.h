#ifndef SpinEchoThread_H
#define SpinEchoThread_H

#include <QThread>
#include <QValueAxis>

#include "utils.h"
#include "mainwindow.h"
#include "NIDAQmx.h"
#include "experiment.h"

class SpinEchoThread : public ExperimentThread
{
    Q_OBJECT

public:
    QLineSeries * series1;
    QLineSeries * series2;
    QLineSeries * series3;
    QLineSeries * series4;
    QLineSeries * series5;
    QLineSeries * series6;

    int echo;
    int nechoes;
    QString outputDir;
    QString name;
    double techo;

    float64 * fft;
    float64 * model;

    int posMin, posMax;

    SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent );
    virtual ~SpinEchoThread();

    virtual void createExperiment();
    virtual void startExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();

    virtual void registerSamples( float64 * samples );

    QValueAxis * getFftAxis()
    {
        QValueAxis * axis = new QValueAxis();
        axis->setMin( 0 );
        axis->setMax( nechoes * techo );
        return axis;
    }
};

#endif // SpinEchoThread_H
