#ifndef TYPES_H
#define TYPES_H

typedef unsigned char   BYTE;
typedef unsigned short  WORD;

class MarkedValue
{
public:
    double value;
    bool mark;

    MarkedValue()
    {
        value = 0;
        mark = false;
    }

    MarkedValue( double value, bool mark )
    {
        this->value = value;
        this->mark = mark;
    }
};


#endif // TYPES_H
