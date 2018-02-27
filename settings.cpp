#include <QStringList>

#include "settings.h"

Settings * Settings::instance = NULL;

Settings::Settings( const QString & fileName )
{
    settings = new QSettings( fileName, QSettings::IniFormat );
    settings->setIniCodec("UTF-8");
}

QStringList Settings::getContents()
{
    return instance->settings->allKeys();
}

void Settings::setContents( QStringList contents )
{

}

void Settings::loadSettings( const QString & fileName )
{
    Settings::instance = new Settings( fileName );
}

QStringList Settings::getExperiments()
{
    return instance->settings->value( "experiments" ).toStringList();
}

void Settings::setExperiments( const QStringList & experiments )
{
    instance->settings->setValue( "experiments", QVariant( experiments ) );
}

QVariant Settings::getExperimentParameter( const QString & experiment, const QString & parameter )
{
    return instance->settings->value( QString( "%1/%2" ).arg( experiment ).arg( parameter ) );
}

void Settings::setExperimentParameter( const QString & experiment, const QString & parameter, QVariant value )
{
    instance->settings->setValue( QString( "%1/%2" ).arg( experiment ).arg( parameter ), value );
}

void Settings::deleteExperiment( const QString & experiment )
{
    instance->settings->remove( experiment  );
}

QString Settings::getSelectdExperiment()
{
    return instance->settings->value( "selectedExperiment" ).toString();
}

void Settings::setSelectedExperiment( const QString &experiment )
{
    instance->settings->setValue( "selectedExperiment", experiment );
}

QString Settings::getCustom( QString setup, QString def )
{
    return instance->settings->value( QString( "Customization/%1" ).arg( setup ), QVariant( def ) ).toString();
}

QVariant Settings::getSetting( QString setting, QVariant def )
{
    return instance->settings->value( QString( "%1" ).arg( setting ), QVariant( def ) );
}
