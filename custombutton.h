#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QPushButton>

#include "settings.h"
#include "utils.h"

class CustomButton : public QPushButton
{
    Q_OBJECT

public:
    CustomButton( QWidget * parent );

private slots:
    void on_button_pressed();
    void on_button_released();
};

#endif // CUSTOMBUTTON_H
