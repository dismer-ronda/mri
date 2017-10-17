#include <stdio.h>
#include <stdarg.h>

#include <QStringList>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QTcpSocket>
#include <QSerialPortInfo>

#include "utils.h"

void WriteShort( BYTE * buffer, int offset, int value )
{
    buffer[offset] = (BYTE) ((value >> 8) & 0xFF);
    buffer[offset + 1] = (BYTE) (value & 0xFF);
}

void WriteWord( BYTE * buffer, int offset, WORD value )
{
    buffer[offset] = (BYTE) ((value >> 8) & 0xFF);
    buffer[offset + 1] = (BYTE) (value & 0xFF);
}

short ReadShort( BYTE * buffer, int offset )
{
    return (((short)buffer[offset]) << 8) | buffer[offset + 1];
}

WORD ReadWord( BYTE * buffer, int offset )
{
    return (((WORD)buffer[offset]) << 8) | buffer[offset + 1];
}

double getY( double x, double x0, double y0, double x1, double y1 )
{
    double m = (y1-y0)/(x1-x0);
    double n = y0 - m * x0;

    return m * x + n;
}

double function( double m, double n, double x )
{
    return (m * (x) + n);
}

QString getDuration( int duration )
{
    int mins = duration / 60;
    int remainder = duration - mins * 60;

    int secs = remainder;

    return formatTimeSpan( mins, secs );
}

int getSign( double a )
{
    return (a > 0) - (a < 0);
}

bool fileExists( QString path )
{
    QFileInfo checkFile( path );

    return checkFile.exists() && (checkFile.isFile() || checkFile.isSymLink());
}

QString formatTimeSpan( int hours, int mins )
{
    QString ret = "";

    if ( hours > 0 )
    {
        if ( !ret.isEmpty() )
            ret += " ";
        ret += QString( "%1h" ).arg( hours, 2, 10, QChar( '0' ) );
    }

    if ( mins > 0 )
    {
        if ( !ret.isEmpty() )
            ret += " ";
        ret += QString( "%1'" ).arg( mins, 2, 10, QChar( '0' ) );
    }

    return ret;
}

QString getDuration( long duration )
{
    int hours = (int) duration / 3600;
    double remainder = (int) duration - hours * 3600;

    int mins = remainder / 60;

    return formatTimeSpan( hours, mins );
}

QString execCmd( QString command )
{
    qDebug() << "command " << command;

    QProcess process;
    process.start( command );
    process.waitForFinished(-1);
    QString ret = process.readAllStandardOutput();
    process.close();
    return ret;
}

/*
-- list wlan interfaces


wlsample ei

  There are 1 interfaces in the system.
  Interface 0:
        GUID: 69e4e0a8-4bd4-44fa-a7df-3cf7f02e4eb2
        802.11n USB Wireless LAN Card
        State: "disconnected"
  Command "ei" completed successfully.

-- connect wlan interface to wifi access point

wlsample disc 69e4e0a8-4bd4-44fa-a7df-3cf7f02e4eb2 linksys i u
  Command "disc" completed successfully.

- disconnect wlan interface

wlsample dc 69e4e0a8-4bd4-44fa-a7df-3cf7f02e4eb2 linksys i u

*/

QString wlanFindInterface( QString binDir )
{
    QString output = execCmd( binDir + "/Wlsample.exe ei" );

    int pos = output.indexOf( "GUID:" );
    QString guid = "";
    if ( pos != -1 )
    {
        pos += 6;
        while ( output.at(pos) != '\r' )
            guid += output.at(pos++);
    }

    return guid;
}

bool presentField( QString source, QString field )
{
    return source.indexOf( field ) != -1;
}

QString scanField( QString source, int &start, QString field )
{
    int pos = source.indexOf( field, start );
    QString txt = "";

    if ( pos == -1 )
    {
        start++;
        return "";
    }

    pos += field.length()+1;
    while ( source.at(pos) != '\r' && pos < source.length() )
        txt += source.at(pos++);

    start = pos;

    return txt.trimmed();
}

QString wlanGetSignalQuality( QString binDir, QString guid, QString ssid )
{
    QString output = execCmd( binDir + "/Wlsample.exe gvl " + guid );
    qDebug() << output;

    int start = 0;
    bool found = false;
    while ( !found && start < output.length() )
    {
        QString ssid1 = scanField( output, start, "SSID:" );

        if ( ssid1 == ssid )
        {
            QString signal = scanField( output, start, "Signal quality:" );

            qDebug() << "signal quality " << signal;

            return signal;
        }
    }

    return "";
}

QString wlanScanNetwork( QString binDir, QString guid, QString pattern, QString &targetMac )
{
    execCmd( binDir + "/Wlsample.exe scan " + guid );
    QThread::sleep( 5 );

    QString output = execCmd( binDir + "/Wlsample.exe gbs " + guid );
    qDebug() << output;

    int start = 0;
    bool found = false;
    while ( !found && start < output.length() )
    {
        QString mac = scanField( output, start, "MAC address:" );
        QString ssid = scanField( output, start, "SSID:" );

        if ( ssid.indexOf( pattern ) != -1 )
        {
            targetMac = mac;
            return ssid;
        }
    }

    return "";
}

void wlanConnect( QString binDir, QString guid, QString ssid )
{
    QString output = execCmd( binDir + "/Wlsample.exe disc " + guid + " " + ssid + " i u" );
    qDebug() << "connect output: " << output;
}

void wlanDisconnect( QString binDir, QString guid )
{
    QString output = execCmd( binDir + "/Wlsample.exe dc " + guid );
    qDebug() << "disconnect output: " << output;
}

QString wlanPing( QString ip )
{
    QString output = execCmd( "ping.exe -n 1 " + ip );
    qDebug() << "ping output: " << output;
    return output;
}

bool wlanSendManufacturingSignal( QString ip )
{
    QByteArray data;
    data.append("M");

    QTcpSocket * _pSocket = new QTcpSocket( NULL ); //
    //connect( _pSocket, SIGNAL(readyRead()), SLOT(readTcpData()) );

    bool found = false;
    _pSocket->connectToHost( ip, 2000);
    if ( _pSocket->waitForConnected() ) {
           _pSocket->write( data );
           _pSocket->flush();
           qDebug() << "written data: " << data;
           found=true;
    }
    _pSocket->close();
    return found;
}

QString uniflashServicePackUpdate( QString binDir, QString port, QString capacity )
{
    QString output = execCmd( binDir + "/uniflash/uniflashCLI.bat -config " + binDir + "/linde-button.usf -setOptions com=" + port + " capacity=" + capacity + " spPath=" + binDir + "/servicepack.bin -operation format servicePackUpdate" );

    qDebug() << "flash output \n" << output;

    if ( !presentField( output, "INFO: connection succeeded" ) )
        return "Error connecting to device at port " + port + "\n";

    QString ret = "";

    int start=0;
    QString error = scanField( output, start, "ERROR:" );
    if ( !error.isEmpty() )
        ret += "ERROR: " + error + "\n";

    start=0;
    QString fatal= scanField( output, start, "FATAL:" );
    if ( !fatal.isEmpty() )
        ret += "FATAL: " + fatal + "\n";

    return ret;
}

QString uniflashProgram( QString binDir, QString port, QString capacity )
{
    QString output = execCmd( binDir + "/uniflash/uniflashCLI.bat -config " + binDir + "/linde-button.usf -setOptions com=" + port + " -operation format program" );

    qDebug() << "flash output \n" << output;

    if ( !presentField( output, "INFO: connection succeeded" ) )
        return "Error connecting to device at port " + port + "\n";
    if ( !presentField( output, "INFO: erase storage succeeded" ) )
        return "Error erasing storage at port " + port + "\n";
    if ( !presentField( output, "INFO: \n\nVerification OK" ) )
        return "Error verification of programming at port " + port + "\n";

    QString ret = "";

    int start=0;
    QString error = scanField( output, start, "ERROR:" );
    if ( !error.isEmpty() )
        ret += "ERROR: " + error + "\n";

    start=0;
    QString fatal= scanField( output, start, "FATAL:" );
    if ( !fatal.isEmpty() )
        ret += "FATAL: " + fatal + "\n";

    return ret;
}

QString uniflashFullProgram( QString binDir, QString port, QString capacity )
{
    QString output = execCmd( binDir + "\\uniflash\\uniflashCLI.bat -config \"" + binDir + "\\linde-button.usf\" -setOptions com=" + port + " capacity=" + capacity + " spPath=\"" + binDir + "\\servicepack.bin\" imagePath=\"" + binDir + "\\firmware.bin\" -operation format servicePackUpdate format program" );

    qDebug() << "flash output \n" << output;

    if ( !presentField( output, "INFO: connection succeeded" ) )
        return "Error connecting to device at port " + port + "\n";
    if ( !presentField( output, "INFO: erase storage succeeded" ) )
        return "Error formatting storage at port " + port + "\n";
    if ( !presentField( output, "INFO: \n\nVerification OK" ) )
        return "Error verification of programming at port " + port + "\n";

    QString ret = "";

    int start=0;
    QString error = scanField( output, start, "ERROR:" );
    if ( !error.isEmpty() )
        ret += "ERROR: " + error + "\n";

    start=0;
    QString fatal= scanField( output, start, "FATAL:" );
    if ( !fatal.isEmpty() )
        ret += "FATAL: " + fatal + "\n";

    return ret;
}

