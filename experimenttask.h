#ifndef EXPERIMENTTASK_H
#define EXPERIMENTTASK_H

#include "NIDAQmx.h"
#include "experimentexception.h"

#define DAQmxErrChk( functionName, functionCall ) { int error; if ( DAQmxFailed( error = (functionCall) ) ) { qDebug() << "DAQmxErrChk" << functionName << "errorCode =" << error; throw new ExperimentException( error ); } }

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
