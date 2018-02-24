#ifndef TASKREPETITIONS_H
#define TASKREPETITIONS_H

#include "experimenttask.h"

class TaskRepetitions : public ExperimentTask
{
public:
    double tr;
    int repetitions;

    TaskRepetitions( const QString & name, double tr, int repetitions );

    virtual void createTask();
};

#endif // TASKREPETITIONS_H
