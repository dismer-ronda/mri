#include "settings.h"
#include "experimentdialog.h"
#include "ui_experimentdialog.h"

ExperimentDialog::ExperimentDialog(Experiment * experiment, bool editName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExperimentDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    this->experiment = experiment;

    ui->editName->setText( experiment->name );
    ui->editName->setEnabled( editName );

    ui->editTR->setValue( experiment->tR );
    ui->editTEcho->setValue( experiment->tEcho );
    ui->editT90->setValue( experiment->t90 );
    ui->editT180->setValue( experiment->t180 );
    ui->editEchoes->setValue( experiment->nEchoes );
    ui->editSamples->setValue( experiment->nSamples );
    ui->editNRepetitions->setValue( experiment->nRepetitions );

    setWindowTitle( "Parámetros de experimento" );
}

ExperimentDialog::~ExperimentDialog()
{
    delete ui;
}

void ExperimentDialog::on_editName_textEdited(const QString &arg1)
{
    experiment->name = arg1;
}

void ExperimentDialog::on_editTR_valueChanged(double arg1)
{
    experiment->tR = arg1;
}

void ExperimentDialog::on_editTEcho_valueChanged(double arg1)
{
    experiment->tEcho = arg1;
}

void ExperimentDialog::on_editT90_valueChanged(double arg1)
{
    experiment->t90 = arg1;
}

void ExperimentDialog::on_editT180_valueChanged(double arg1)
{
    experiment->t180 = arg1;
}

void ExperimentDialog::on_editEchoes_valueChanged(int arg1)
{
    experiment->nEchoes = arg1;
}

void ExperimentDialog::on_editSamples_valueChanged(int arg1)
{
    experiment->nSamples = arg1;
}

void ExperimentDialog::on_editNRepetitions_valueChanged(int arg1)
{
    experiment->nRepetitions = arg1;
}
