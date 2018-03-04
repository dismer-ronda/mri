#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings
{

public:
    Settings( const QString & fileName );

    static QStringList getContents();
    static void setContents( QStringList contents );

    static QVariant getSetting( QString setting, QVariant def );

    static void loadSettings( const QString & fileName );

    static QString getSelectdExperiment();
    static void setSelectedExperiment( const QString &experiment );

    static QStringList getExperiments();
    static void setExperiments( const QStringList & experiments );

    static QVariant getExperimentParameter( const QString & experiment, const QString & parameter );
    static void setExperimentParameter( const QString & experiment, const QString & parameter, QVariant value );

    static void deleteExperiment( const QString & experiment );

    static QString getCustom( QString setup, QString def );
    static QString getText( QString id );

private:
    static Settings * instance;

    QSettings * settings;
};

#endif // SETTINGS_H
