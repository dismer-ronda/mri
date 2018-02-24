#include <QDebug>

#include "experimentexception.h"

ExperimentException::ExperimentException( int code )
{
    this->code = code;

    qDebug() << "exception dispatched";
}
