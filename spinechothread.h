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
    double trep;
    double t90;
    double t180;
    int32 nrepetitions;
    int bandwidth;
    int samplingrate;

    float64 * fft;
    float64 * echoMagnitude;

    int posMin, posMax;
    float64 echoFactor;
    bool simulation;

    SpinEchoThread( QString binDir, const QString & experiment, MainWindow * parent );
    virtual ~SpinEchoThread();

    virtual void createExperiment();
    virtual void startExperiment();

    virtual int getProgressCount();
    virtual int getProgressTimer();

    virtual void registerSamples( float64 * samples );

    virtual QDialog * getResultDialog();

    QValueAxis * getFftAxis()
    {
        QValueAxis * axis = new QValueAxis();
        axis->setMin( 0 );
        axis->setMax( nechoes * techo );
        return axis;
    }

    float64 calculateT2();
};

#endif // SpinEchoThread_H
