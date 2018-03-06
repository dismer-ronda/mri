#ifndef UTILS_H
#define UTILS_H

#include <QList>
#include <QString>
#include <QWidget>
#include <QPushButton>

#include "types.h"
#include "NIDAQmx.h"

void WriteShort( BYTE * buffer, int offset, int value );
void WriteWord( BYTE * buffer, int offset, WORD value );

short ReadShort( BYTE * buffer, int offset );
WORD ReadWord( BYTE * buffer, int offset );

double getY( double x, double x0, double y0, double x1, double y1 );

double getMean( QList<double> values );

double function( double m, double n, double x );

QString formatTimeSpan( int mins, int secs );
QString getDuration( int duration );

int getSign( double a );

bool fileExists( QString path );

QString formatTimeSpan( int hours, int mins );
QString getDuration( long duration );

QString execCmd( QString command );

QString wlanFindInterface( QString binDir );
QString wlanScanNetwork( QString binDir, QString guid, QString pattern, QString & targetMac );
void wlanConnect( QString binDir, QString guid, QString ssid );
void wlanDisconnect( QString binDir, QString guid );
QString wlanPing( QString ip );
bool wlanSendManufacturingSignal( QString ip );
QString wlanGetSignalQuality( QString binDir, QString guid, QString ssid );

QString uniflashServicePackUpdate( QString binDir, QString port, QString capacity  );
QString uniflashProgram( QString binDir, QString port, QString capacity  );

QString uniflashFullProgram( QString binDir, QString port, QString capacity );

int getResX();
int getResY();
/*int getMarginX();
int getMarginY();*/
int getDialogLineHeight();
int getDialogButtonsHeight();
int getBorderThickness();

QPushButton * createButton( QWidget * parent, const QString & text );

QString readTextFile( const QString & fileName );
void writeTextFile( const QString & fileName, const QString & text );

bool presentField( QString source, QString field );
QString scanField( QString source, int &start, QString field );

QString modifyStyleSheet( QString styleSheet, QString property, QString value );

void findMaxPos( float64 * values, int size, double & max, int & pos );
double getAreaUnderMax( float64 * values, int size, double max, int pos, int & posMin, int & posMax );
double getAreaUnderRegion( float64 * values, int posMin, int posMax );

void fftw( float64 * data, int nsamples, int zeroOffsets );

#endif // UTILS_H
