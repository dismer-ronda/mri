#ifndef TASKACQUISITION_H
#define TASKACQUISITION_H

#include "experimenttask.h"

class TaskAcquisitionGate : public ExperimentTask
{
public:
    double t90;
    double techo;
    int nechoes;

    TaskAcquisitionGate( const QString & name, double t90, double techo, int nechoes );

    virtual void createTask();
};

#endif // TASKACQUISITION_H
