#ifndef TASKRFGATE_H
#define TASKRFGATE_H

#include "experimenttask.h"

class TaskRFGate : public ExperimentTask
{
public:
    double t90;
    double t180;
    double techo;
    int nechoes;
    float64 dataFreq[1024];
    float64 dataDC[1024];

    TaskRFGate( const QString & name, double t90, double t180, double techo, int nechoes );

    virtual void createTask();
};

#endif // TASKRFGATE_H
