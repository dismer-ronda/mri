#ifndef EXPERIMENTEXCEPTION_H
#define EXPERIMENTEXCEPTION_H

#include <QDebug>
#include <QException>

class ExperimentException : public QException
{
public:
    int code;

    ExperimentException( int code );
};

#endif // EXPERIMENTEXCEPTION_H
