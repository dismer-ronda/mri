// Signal.cpp : implementation file
//

#include <math.h>
#include <QImage>
#include <QPainter>
#include <QMainWindow>
#include <QWidgetList>
#include <QApplication>

#include "settings.h"
#include "utils.h"
#include "signal.h"

CSignal::CSignal( QWidget * parent, QColor color )
    : QWidget( parent ), color( color )
{
    mutex = new QMutex( QMutex::Recursive );

    fontSize = Settings::getCustom( "signal.fontSize", "12" ).toInt();
    marginLeft = (4 * fontSize + getMarginX());
    marginBottom = 2 * fontSize;
    thickness = Settings::getCustom( "signal.thickness", "2" ).toInt();

    setBackgroundColor( Settings::getCustom( "signal.background", "000000" ) );
}

CSignal::~CSignal()
{
    delete mutex;
}

void CSignal::initSignal( int size, bool autoscale, double min, double max, QString xunits, QString yunits, bool drawXAxis, bool drawYAxis, double deltaMin, double deltaMax )
{
    mutex->lock();

    signal.clear();

    this->autoscale = autoscale;
    this->xunits = xunits;
    this->yunits = yunits;
    this->drawXAxis = drawXAxis;
    this->drawYAxis = drawYAxis;
    this->deltaMin = deltaMin;
    this->deltaMax = deltaMax;

    if ( !autoscale )
    {
        this->min = min;
        this->max = max;
    }
    else
        this->min = this->max = 0;

    update();

    mutex->unlock();
}

void CSignal::paintEvent( QPaintEvent * )
{
    mutex->lock();

    QRect cr = this->geometry();

    QPainter painter( this );

    painter.setRenderHint(QPainter::Antialiasing);

    double deltaY = (double)(max + deltaMax - (min - deltaMin));
    if ( deltaY == 0 )
        deltaY = 1;

    double mx = (cr.width()- marginLeft)/(double)(size-1);
    double my = (cr.height()- marginBottom - getMarginY())/deltaY;

    double nx = cr.left() + marginLeft;
    double ny = cr.top() + marginBottom;

    QPen pen1 = QPen( QColor( QString( "#%1" ).arg( Settings::getCustom( "signal.color", "FFFFFF" ) ) ) );
    pen1.setWidth( thickness );
    painter.setPen( pen1 );

    QPen pen2 = QPen( QColor( QString( "#%1" ).arg( Settings::getCustom( "signal.color", "FFFFFF" ) ) ) );
    pen2.setWidth( 1 );

    QFont font=painter.font() ;
    font.setPointSize( fontSize );
    font.setBold(true);
    painter.setFont(font);

    if ( !autoscale || (autoscale && count > 0) )
    {
        QString smax = QString::number( ceil( max + deltaMax ) ) + "\n" + yunits;
        QString smin = QString::number( floor( min - deltaMin ) );

        QRect smaxRect = QRect( cr.left(), cr.top(), cr.left() + marginLeft - 2 * getMarginX(), cr.bottom() );
        painter.drawText( smaxRect, Qt::AlignRight | Qt::AlignTop, smax );

        QRect sminRect = QRect( cr.left(), cr.top(), cr.left() + marginLeft - 2 * getMarginX(), cr.bottom() - fontSize );
        painter.drawText( sminRect, Qt::AlignRight | Qt::AlignBottom, smin );
    }

    if ( drawYAxis )
        painter.drawLine( cr.left() + marginLeft - getMarginX(), cr.top(), cr.left() + marginLeft - getMarginX(), cr.bottom() - marginBottom + getMarginY() );

    if ( drawXAxis )
        painter.drawLine( cr.left() + marginLeft - getMarginX(), cr.bottom() - marginBottom + getMarginY(), cr.right() - getMarginX(), cr.bottom() - marginBottom + getMarginY() );

    if ( !autoscale || (autoscale && count > 0) )
    {
        if ( min-deltaMin < 0 && max+deltaMax > 0 )
        {
            double y = function( my, ny, -(min-deltaMin) );

            QRect zeroRect = QRect( cr.left(), cr.height() - 1 - y - fontSize, marginLeft - 2 * getMarginX(), 2 * fontSize  );
            painter.drawText( zeroRect, Qt::AlignRight | Qt::AlignVCenter, QString( "%1" ).arg( 0 ) );

            painter.drawLine( cr.left() + marginLeft, cr.height() - 1 - y, cr.right() , cr.height() - 1 - y );

            QRect unitsRect = QRect( cr.left(), cr.height() - 1 - y + getMarginY(), cr.width(), 2 * fontSize );
            painter.drawText( unitsRect, Qt::AlignRight | Qt::AlignTop, xunits );
        }
    }

    QList<SignalReference> values = references.values();
    for ( int i = 0; i < values.size(); i++ )
    {
        QPen pen2 = QPen( QColor( values.at(i).color ) );
        pen2.setWidth( thickness );
        pen2.setStyle( values.at(i).style );
        painter.setPen( pen2 );

        double y = function( my, ny, values.at(i).value - (min-deltaMin) );

        painter.drawLine( cr.left() + marginLeft, cr.height() - 1 - y, cr.right(), cr.height() - 1 - y );
    }

    if ( count > 0 )
    {
        QPen pen3 = QPen( color );
        pen3.setWidth( thickness );
        painter.setPen( pen3 );

        double x0 = function( mx, nx, 0 );
        double y0 = function( my, ny, signal[0].value - (min-deltaMin) );

        for ( int i = 0; i < count; i++ )
        {
            double x1 = function( mx, nx, i );
            double y1 = function( my, ny, signal[i].value - (min-deltaMin) );

            if ( i != index && i != index + 1 )
            {
                if ( signal[i].mark )
                {
                    painter.setPen( pen2 );
                    painter.drawLine( x1, cr.bottom() - marginBottom + getMarginY(), x1, cr.top() );
                    painter.setPen( pen3 );
                }

                painter.drawLine( x0, cr.height() - 1 - y0, x1, cr.height() - 1 - y1 );
            }

            x0 = x1;
            y0 = y1;
        }
    }

    mutex->unlock();
}

void CSignal::setBackgroundColor( QString  background )
{
    setStyleSheet( QString( "background-color: #%1;" ).arg( background ) );

    this->background = background;
    update();
}

void CSignal::calculateExtremes()
{
    mutex->lock();

    if ( count > 0 )
    {
        min = max = signal[0].value;

        for ( int i = 1; i < count; i++ )
        {
            if ( signal[i].value < min )
                min = signal[i].value;
            else if ( signal[i].value > max )
                max = signal[i].value;
        }
    }
    else
    {
        min = max = 0;
    }

    mutex->unlock();
}

void CSignal::addPoints( QList<MarkedValue> & values )
{
    mutex->lock();

    int minIndex = index;

    bool refresh = false;
    bool extremes = false;

    for ( int i = 0; i < values.size(); i++ )
    {
        double value = values.at(i).value;
        bool mark = values.at(i).mark;

        if ( count > 0 && autoscale && (signal[index].value == min || signal[index].value == max) )
            extremes = true;

        signal[index].value = value;
        signal[index].mark = mark;


        if ( count < size )
            count++;

        if ( autoscale )
        {
            if ( count == 1 )
            {
                min = max = value;
                refresh = true;
            }

            if ( value < min )
            {
                refresh = true;

                min = value;
            }
            else if ( value > max )
            {
                refresh = true;

                max = value;
            }
        }

        if ( index < size - 1 )
            index++;
        else
            index = 0;
    }

    if ( extremes )
    {
        calculateExtremes();
        update();
    }
    else if ( refresh || minIndex > index )
    {
        update();
    }
    else
    {
        QRect cr = this->geometry();

        double mx = (double)(cr.width() - marginLeft - getMarginX() )/(size);
        double nx = cr.left() + marginLeft;

        double x0 = function( mx, nx, minIndex );
        double x1 = function( mx, nx, index );

        int fromX = floor( x0 ) - 4;
        int toX = ceil( x1 ) + 4;

        if ( fromX < toX && fromX >= cr.left() && fromX <= cr.right() && toX >= cr.left() && toX <= cr.right() )
            update( fromX, cr.top(), toX - fromX, cr.height() );
    }

    mutex->unlock();
}

void CSignal::wheelChange( int )
{
}

void CSignal::focusConsumer( int status )
{
    switch( status )
    {
        case STATUS_UNSELECTED:
            background = Settings::getCustom( "signal.background", "000000" );
        break;
        case STATUS_SELECTED:
            background = Settings::getCustom( "wheel.selected.background", BACKGROUND_SELECTED );
        break;
        case STATUS_CHANGING:
            background = Settings::getCustom( "wheel.changing.background", BACKGROUND_CHANGING );
        break;
    }

    setStyleSheet( QString( "background-color: #%1;" ).arg( background ) );
}

double CSignal::getValue()
{
    return 0;
}

void CSignal::setSize( int size )
{
    mutex->lock();

    delete signal;
    signal = new MarkedValue[size];
    this->size = size;

    index = 0;
    count = 0;

    mutex->unlock();
}
