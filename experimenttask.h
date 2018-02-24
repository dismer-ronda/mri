#ifndef EXPERIMENTTASK_H
#define EXPERIMENTTASK_H

#include "NIDAQmx.h"
#include "experimentexception.h"

#define DAQmxErrChk( functionCall ) { int error; if ( DAQmxFailed( error = (functionCall) ) ) throw new ExperimentException( error ); }

class ExperimentTask
{
protected:
    TaskHandle taskId;
    QString name;

public:
    ExperimentTask( const QString & name );

    virtual void createTask();
    void startTask();
    virtual void destroyTask();

};

#endif // EXPERIMENTTASK_H
