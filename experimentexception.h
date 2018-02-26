#ifndef EXPERIMENTEXCEPTION_H
#define EXPERIMENTEXCEPTION_H

#include <QDebug>
#include <QException>

class ExperimentException : public QException
{
public:
    static const int ERROR_UNKNOWN                          = -300001;
    static const int ERROR_INVALID_ACQ_DURATION             = -300002;
    int code;

    ExperimentException( int code );
};

#endif // EXPERIMENTEXCEPTION_H
