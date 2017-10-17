#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QString>

struct Experiment
{
    QString name;

    double tR;
    double tEcho;
    double t90;
    double t180;hgfh
    double nEchoes;
    double nSamples;
};

#endif // EXPERIMENT_H
