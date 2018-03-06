#include <stdio.h>
#include <stdarg.h>

#include <QStringList>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QTcpSocket>

#include "utils.h"
#include "settings.h"
#include "custombutton.h"
#include "fftw3.h"

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

int getResX()
{
    return Settings::getCustom( "screenWidth", "1280" ).toInt();
}

int getResY()
{
    return Settings::getCustom( "screenHeight", "720" ).toInt();
}

int getMarginX()
{
    return Settings::getCustom( "marginX", "6" ).toInt();
}

int getMarginY()
{
    return Settings::getCustom( "marginY", "6" ).toInt();
}

int getHeadingWidth()
{
    return Settings::getCustom( "headingWidth", "40" ).toInt();
}

int getDialogLineHeight()
{
    return Settings::getCustom( "dialogLineHeight", "64" ).toInt();
}

int getDialogButtonsHeight()
{
    return Settings::getCustom( "dialogButtonsHeight", "96" ).toInt();
}


int getBorderThickness()
{
    return Settings::getCustom( "border.thickness", "6" ).toInt();
}

QPushButton * createButton( QWidget * parent, const QString & text )
{
    CustomButton * button = new CustomButton( parent );
    button->setStyleSheet( QString( "font-size: %1px; font-style: %2; font-weight: %3; background-color: #%4; color: #%5" )
                           .arg( Settings::getCustom( "button.fontSize", "24" ) )
                           .arg( Settings::getCustom( "button.fontStyle", "normal" ) )
                           .arg( Settings::getCustom( "button.fontWeight", "normal" ) )
                         .arg( Settings::getCustom( "button.background", "075B91" ) )
                         .arg( Settings::getCustom( "button.color", "FFFFFF" ) ) );
    button->setText( text );
    return button;
}


QString readTextFile( const QString & fileName )
{
    QFile file( fileName  );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    QString ret = "";
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        ret += line + "\n";
    }

    return ret;
}

void writeTextFile( const QString & fileName, const QString & text )
{
    QFile file( fileName  );
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return ;

    QTextStream out(&file);
    out << text;
    file.flush();
    file.close();
}

void writeRawDataFile( const QString & fileName, const QString &header, float64 * data, int nsamples )
{
    qDebug() << fileName;

    QFile file( fileName  );
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return ;

    QTextStream out(&file);

    out << header;
    for ( int i = 0; i < nsamples; i++ )
        out << data[2*i] << "," << data[2*i+1] << "\n";

    file.flush();
    file.close();
}

void writeFFTDataFile( const QString & fileName, const QString &header, float64 * fft, int nsamples )
{
    QFile file( fileName  );
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return ;

    QTextStream out(&file);

    out << header;
    for ( int i = 0; i < nsamples; i++ )
        out << fft[i] << "\n";

    file.flush();
    file.close();
}

void writeEchoMagnitude( const QString & fileName, const QString &header, float64 * echoMagnitude, int nechoes )
{
    QFile file( fileName  );
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return ;

    QTextStream out(&file);

    out << header;
    for ( int i = 0; i < nechoes; i++ )
        out << echoMagnitude[i] << "\n";

    file.flush();
    file.close();
}

QString scanStyleSheetField( QString source, int &start, QString & field )
{
    while ( start < source.length() && source.at(start) != ':' )
        field += source.at(start++);
    field = field.trimmed();
    start++;

    QString value="";
    while ( start < source.length() && source.at(start) != ';' )
        value += source.at(start++);
    value = value.trimmed();
    start++;

    return value;
}

QMap<QString, QString> * parseStyleSheet( QString styleSheet )
{
    QMap<QString, QString> * map = new QMap<QString, QString>();

    int start = 0;

    qDebug() << "styleSheet" << styleSheet;

    while ( start < styleSheet.length() )
    {
        QString field = "";
        QString value = scanStyleSheetField( styleSheet, start, field );

        if ( !value.isEmpty() )
            (*map)[field] = value;
    }

    return map;
}

QString generateStyleSheet( QMap<QString, QString> * map )
{
    QMap<QString, QString>::const_iterator i = map->constBegin();

    QString ret = "";

    while (i != map->constEnd()) {
        ret += i.key() + ": " + i.value() + ";";
        ++i;
    }

    return ret;
}

QString modifyStyleSheet( QString styleSheet, QString property, QString value )
{
    QMap<QString, QString> * map = parseStyleSheet(styleSheet);

    (*map)[property] = value;

    return generateStyleSheet( map );
}

void findMaxPos( float64 * values, int size, double & max, int & pos )
{
    max = values[0];
    pos = 0;

    for ( int i = 0; i < size; i++ )
    {
        if ( values[i] > max )
        {
            pos = i;
            max = values[i];
        }
    }
}

float64 getAreaUnderMax( float64 * values, int size, float64 factor, float64 max, int pos, int & posMin, int & posMax )
{
    float64 area = 0;

    int i = pos;
    while ( values[i] > max * factor && i >= 0 )
    {
        posMin = i;
        area += values[i--];
    }

    i = pos+1;
    while ( values[i] > max * factor && i < size )
    {
        posMax = i;
        area += values[i++];
    }

    return area;
}

float64 getAreaUnderRegion( float64 * values, int posMin, int posMax )
{
    float64 area = 0;

    for ( int i = posMin; i < posMax; i++ )
        area += values[i];

    return area;
}

void fftw( float64 * data, int nsamples, int zeroOffsets )
{
#ifndef LINUX_BOX
    fftw_complex *in, *out;
    fftw_plan p;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nsamples);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nsamples);

    for ( int i = 0; i < nsamples; i++ )
    {
        in[i][0] = data[2*i];
        in[i][1] = data[2*i+1];
    }

    p = fftw_plan_dft_1d(nsamples, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);

    for ( int i = 0; i < nsamples; i++ )
    {
        if ( i < zeroOffsets || i > nsamples - 1 - zeroOffsets)
        {
            data[2*i] = data[2*i+1] = 0;
        }
        else
        {
            data[2*i] = out[i][0];
            data[2*i+1] = out[i][1];
        }
    }

    for ( int i = 0; i < nsamples/2; i++ )
    {
        float64 tempr = data[2*i];
        float64 tempi = data[2*i+1];

        data[2*i] = data[nsamples + 2*i];
        data[2*i+1] = data[nsamples + 2*i+1];

        data[nsamples + 2*i] = tempr;
        data[nsamples + 2*i+1] = tempi;
    }

    fftw_free(in);
    fftw_free(out);
#endif
}

