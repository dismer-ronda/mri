#ifndef EXPERIMENTDIALOG_H
#define EXPERIMENTDIALOG_H

#include <QDialog>

#include "experiment.h"

namespace Ui {
class ExperimentDialog;
}

class ExperimentDialog : public QDialog
{
    Q_OBJECT

    Experiment * experiment;

public:
    explicit ExperimentDialog(Experiment * experiment, bool editName, QWidget *parent = 0);
    ~ExperimentDialog();

private slots:
    void on_editName_textEdited(const QString &arg1);

    void on_editTR_valueChanged(double arg1);

    void on_editTEcho_valueChanged(double arg1);

    void on_editT90_valueChanged(double arg1);

    void on_editT180_valueChanged(double arg1);

    void on_editEchoes_valueChanged(int arg1);

    void on_editSamples_valueChanged(int arg1);

private:
    Ui::ExperimentDialog *ui;
};

#endif // EXPERIMENTDIALOG_H
