#ifndef MESSAGEDLG_H
#define MESSAGEDLG_H

#include <QDialog>
#include <QPushButton>

class MessageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MessageDialog(QWidget *parent = 0);
    virtual ~MessageDialog();

    virtual void initComponents( QString message );

private slots:
    void on_buttonOk_clicked();

protected:
    QPushButton *okButton;
};

#endif // SHUTDOWNDLG_H
