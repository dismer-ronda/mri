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

    TaskRFGate( const QString & name, double t90, double t180, double techo, int nechoes );

    virtual void createTask();
};

#endif // TASKRFGATE_H
