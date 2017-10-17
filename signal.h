#ifndef SIGNAL_H
#define SIGNAL_H

#include <QPaintEvent>
#include <QPainter>
#include <QWidget>
#include <QList>
#include <QString>
#include <QStack>
#include <QMutex>
#include <QList>

#include "types.h"

class CSignal : public QWidget
{
public:
    CSignal( QWidget * parent, QColor color );           // protected constructor used by dynamic creation

	virtual ~CSignal();
	
public:
    void initSignal( int size, bool autoscale, double min, double max, QString xunits, QString yunits, bool drawXAxis, bool drawYAxis, double deltaMin, double deltaMax );

    void setBackgroundColor( QString background );

    void setPoints( QList<QList<double>> & values );

    double getMin() { return min; }
    void setMin( double min ) { this->min = min; }

    double getMax() { return max; }
    void setMax( double max ) { this->max = max; }

    bool getAutoScale() { return autoscale; }
    void setAutoScale( bool autoscale ) { this->autoscale = autoscale; }

    void setSize( int size );

    void setXUnits( QString xunits ) { this->xunits = xunits; }
    void setYUnits( QString yunits ) { this->yunits = yunits; }

    void calculateExtremes();

protected:
    QMutex * mutex;

    QList<QList<double>> signal;

    bool autoscale;
    bool drawXAxis;
    bool drawYAxis;
    double min;
    double max;
    double deltaMin;
    double deltaMax;
    QString xunits;
    QString yunits;

    QColor color;
    QString background;

    int fontSize;
    int marginLeft;
    int marginBottom;
    int thickness;

    void paintEvent( QPaintEvent *event );
};

#endif // !defined(SIGNAL_H)
