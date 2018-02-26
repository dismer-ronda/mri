#ifndef TASKACQUISITION_H
#define TASKACQUISITION_H

#include "experimenttask.h"

class TaskAcquisitionGate : public ExperimentTask
{
public:
    double tr;
    double t90;
    double t180;
    double techo;
    int nechoes;
    int nsamples;
    int bandwidth;
    int ringdowndelay;

    TaskAcquisitionGate( const QString & name, double tr, double t90, double t180, double techo, int nechoes, int nsamples, int bandwidth, double ringdowndelay );

    virtual void createTask();
};

#endif // TASKACQUISITION_H
