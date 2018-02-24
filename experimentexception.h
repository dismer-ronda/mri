#ifndef EXPERIMENTEXCEPTION_H
#define EXPERIMENTEXCEPTION_H

#include <QException>

class ExperimentException : public QException
{
public:
    int code;

    ExperimentException( int code );
};

#endif // EXPERIMENTEXCEPTION_H
