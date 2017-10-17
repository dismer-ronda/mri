#ifndef UTILS_H
#define UTILS_H

#include <QList>
#include <QString>
#include <QWidget>

#include "types.h"

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

#endif // UTILS_H
