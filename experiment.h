#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QString>

struct Experiment
{
    QString name;
    QString type;

    double tR;
    double tEcho;
    double t90;
    double t180;
    double nEchoes;
    double nSamples;
    int nRepetitions;
};

#endif // EXPERIMENT_H
