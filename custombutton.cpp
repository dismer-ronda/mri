#include <QDebug>

#include "custombutton.h"

CustomButton::CustomButton( QWidget * parent )
    : QPushButton( parent )
{
    connect(this, SIGNAL(pressed()), this, SLOT(on_button_pressed()));
    connect(this, SIGNAL(released()), this, SLOT(on_button_released()));
}

void CustomButton::on_button_pressed()
{
    QPushButton * button = (QPushButton *)QObject::sender();
    button->setStyleSheet( modifyStyleSheet( button->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "button.background.clicked", "FFFFFF" ) ) ) );
}

void CustomButton::on_button_released()
{
    QPushButton * button = (QPushButton *)QObject::sender();
    button->setStyleSheet( modifyStyleSheet( button->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "button.background", "FFFFFF" ) ) ) );
}

bool CustomButton::event(QEvent *event)
{
    if ( event->type() == QEvent::MouseMove )
    {
        qDebug() << "mouse move";
        return QPushButton::event(event);
    }
    else
    {
        return QPushButton::event(event);
    }
}
