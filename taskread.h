#ifndef TASKREAD_H
#define TASKREAD_H

#include "experimenttask.h"

class TaskRead : public ExperimentTask
{
public:
    int sampleRate;
    int nsamples;
    DAQmxEveryNSamplesEventCallbackPtr callbackFunction;
    void * callbackData;

    TaskRead( const QString & name, int sampleRate, int nsamples, DAQmxEveryNSamplesEventCallbackPtr callbackFunction, void * callbackData );

    virtual void createTask();
};

#endif // TASKREAD_H
