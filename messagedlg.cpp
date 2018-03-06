#include <QString>
#include <QLabel>
#include <QGuiApplication>
#include <QScreen>

#include "utils.h"
#include "settings.h"
#include "messagedlg.h"

MessageDialog::MessageDialog(QWidget *parent) :
    QDialog(parent)
{
    //setStyleSheet( QString( "border: 1px solid black; color: #%1; background-color: #%2; font-size: %3px;" ).arg( Settings::getCustom( "dialog.color", "FFFFFF" ) ).arg( Settings::getCustom( "dialog.background", "4B5C68" ) ).arg( Settings::getCustom( "dialog.fontSize", "30" ) ) );

    setWindowModality( Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint);
}

MessageDialog::~MessageDialog()
{
}

void MessageDialog::initComponents( QString message )
{
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();

    int maxHeight = screenGeometry.height();
    int maxWidth = screenGeometry.width();

    int width = 640;
    int height = 320 ;

    int heightButtons = 56;
    int widthButtons = 220;

    int border = getBorderThickness();

    setGeometry( (maxWidth - width)/2, (maxHeight - height)/2, width, height );

    setStyleSheet( QString( "border: %1px solid #%2; background-color: #%3" )
                   .arg( Settings::getCustom( "window.borderThickness", "1" ) )
                   .arg( Settings::getCustom( "window.borderColor", "0000FF" ) )
                   .arg( Settings::getCustom( "window.background", "FFFFFF" ) ) );

    QWidget * mainWidget = new QWidget( this );
    mainWidget->setStyleSheet( "border: none" );
    mainWidget->setGeometry( border, border, width - 2 * border, height - heightButtons - 2 * border);
    mainWidget->show();

    QRect crMain = mainWidget->geometry();

    QLabel * labelMsg = new QLabel( mainWidget );
    labelMsg->setStyleSheet( QString( "font-size: %1px; font-weight: bold; color: #%2" )
                         .arg( Settings::getCustom( "label.fontSize", "24" ) )
                         .arg( Settings::getCustom( "label.color", "075B91" ) ) );
    labelMsg->setText( message );
    labelMsg->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
    labelMsg->setGeometry( 0, 0, crMain.right(), crMain.height() );


    okButton = createButton( this, Settings::getText( "ok" ) );
    okButton->setStyleSheet( modifyStyleSheet( okButton->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "tab.selected", "FFFFFF" ) ) ) );
    okButton->setStyleSheet( modifyStyleSheet( okButton->styleSheet(), "border", "none" ) );
    okButton->setGeometry( width - widthButtons - 2 * border, height - heightButtons - 2 * border, widthButtons, heightButtons );
    okButton->show();
    connect(okButton, SIGNAL(clicked()), this, SLOT(on_buttonOk_clicked()));
}

void MessageDialog::on_buttonOk_clicked()
{
    done(1);
}

