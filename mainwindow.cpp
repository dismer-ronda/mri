#include <QDebug>
#include <QLayout>
#include <QMessageBox>
#include <QStringListModel>

#include "settings.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "programmerthread.h"
#include "experimentdialog.h"

QString MainWindow::binDir;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    binDir = "c:/mri";

    Settings::loadSettings( binDir + "/setup.ini" );

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    window()->showFullScreen();

    QRect cr = window()->geometry();

    QRect cr4 = ui->buttonTerminate->geometry();
    ui->buttonTerminate->setGeometry( cr.right() - cr4.width() - 32, 8, cr4.width(), 56 );

    ui->tabWidget->setGeometry(32, 32, cr.right() - 64, cr.bottom() - 64);
    cr = ui->tabWidget->geometry();

    ui->tabWidget->setCurrentIndex(0);

    QRect cr1 = ui->widgetProgramming->geometry();
    ui->widgetProgramming->setGeometry( (cr.width() - cr1.width())/2, (cr.height() - cr1.height())/2, cr1.width(), cr1.height());

    QCoreApplication::addLibraryPath( binDir );

    ui->progressBar->hide();
    ui->buttonProgram->show();
    ui->buttonStop->hide();
    ui->rowSelectExperiment->setEnabled(true);
    ui->signalWidget->hide();

    experiments = Settings::getExperiments();
    model = new QStringListModel( experiments, NULL);
    ui->listExperiments->setModel( model );

    ui->comboExperiments->addItems( experiments );
    ui->comboExperiments->setCurrentText( Settings::getSelectdExperiment() );

    chart = new QChart();
    chart->legend()->hide();

    chart->createDefaultAxes();
    //chart->setTitle("Simple line chart example");

    chartView = new QChartView(chart, ui->signalWidget);
    chartView->setRenderHint(QPainter::Antialiasing);

    QRect cr5 = ui->signalWidget->geometry();
    chartView->setGeometry(0, 0, cr5.width(), cr5.height() );

    chartView->show();

    programmer1 = NULL;

    series1 = NULL;
    series2 = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setFinished( bool value )
{
    mutex.lock();
    finished = value;
    mutex.unlock();
}

bool MainWindow::isFinished()
{
    mutex.lock();
    bool ret = finished;
    mutex.unlock();

    return ret;
}

void MainWindow::setChartSeries( QLineSeries * series )
{
    mutex.lock();
    series1 = series;
    mutex.unlock();
}

QLineSeries * MainWindow::getChartSeries()
{
    QLineSeries * ret = new QLineSeries();

    mutex.lock();
    if ( series1 != NULL )
    {
        ret->append( series1->points() );
        series1 = NULL;
    }
    mutex.unlock();

    return ret;
}

void MainWindow::startExperiment()
{
    setFinished(false);

    Experiment * experiment = new Experiment();

    experiment->name = Settings::getSelectdExperiment();

    experiment->tR = Settings::getExperimentParameter( experiment->name, "TR" ).toDouble();
    experiment->tEcho = Settings::getExperimentParameter( experiment->name, "TEcho" ).toDouble();
    experiment->t90 = Settings::getExperimentParameter( experiment->name, "T90" ).toDouble();
    experiment->t180 = Settings::getExperimentParameter( experiment->name, "T180" ).toDouble();
    experiment->nEchoes = Settings::getExperimentParameter( experiment->name, "nEchoes" ).toInt();
    experiment->nSamples = Settings::getExperimentParameter( experiment->name, "nSamples" ).toInt();

    chart->removeAllSeries();
    chartView->update();

    programmer1 = new ProgrammerThread( MainWindow::binDir, experiment, this );
    programmer1->start();

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(30);
    ui->progressBar->setValue(0);

    ui->buttonProgram->hide();
    ui->progressBar->show();
    ui->rowSelectExperiment->setEnabled(false);
    ui->label_Programming->hide();
    ui->buttonTerminate->hide();
    ui->buttonStop->show();
    ui->signalWidget->show();

    ui->tabWidget->setTabEnabled(1, false);

    timerId = startTimer(1000);
}

void MainWindow::on_buttonProgram_clicked()
{
    startExperiment();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if ( !isFinished() )
    {
        int value = ui->progressBar->value() + 1;
        int size = ui->progressBar->maximum();
        ui->progressBar->setValue( value == size + 1 ? 0 : value );

        QLineSeries * series = getChartSeries();

        if ( series->count() > 0 )
        {
            chart->removeAllSeries();
            chart->addSeries(series);

            chartView->update();
        }
    }
    else
    {
        killTimer(timerId);

        ui->label_Programming->setText("Prepare la muestra y presion el botón \"Iniciar\"");
        ui->buttonProgram->show();
        ui->progressBar->hide();
        ui->rowSelectExperiment->setEnabled(true);
        ui->label_Programming->show();
        ui->tabWidget->setTabEnabled(1, true);
        ui->buttonTerminate->show();
        ui->buttonStop->hide();
        ui->signalWidget->hide();

        ui->tabWidget->setTabEnabled(1, true);
        ui->tabWidget->setTabEnabled(2, true);
    }
}

void MainWindow::on_buttonTerminate_clicked()
{
    QApplication::quit();
}

void MainWindow::on_buttonAdd_clicked()
{
    Experiment * experiment = new Experiment();

    ExperimentDialog dlg(experiment, true, this);

    if ( dlg.exec() == QDialog::Accepted )
    {
        experiments.append( experiment->name );
        model->setStringList( experiments );
        Settings::setExperiments( experiments );

        ui->comboExperiments->clear();
        ui->comboExperiments->addItems(experiments);

        Settings::setExperimentParameter( experiment->name, "TR", QVariant( experiment->tR ) );
        Settings::setExperimentParameter( experiment->name, "TEcho", QVariant( experiment->tEcho ) );
        Settings::setExperimentParameter( experiment->name, "T90", QVariant( experiment->t90 ) );
        Settings::setExperimentParameter( experiment->name, "T180", QVariant( experiment->t180 ) );
        Settings::setExperimentParameter( experiment->name, "nEchoes", QVariant( experiment->nEchoes ) );
        Settings::setExperimentParameter( experiment->name, "nSamples", QVariant( experiment->nSamples ) );
    }
}

void MainWindow::on_buttonModify_clicked()
{
    int index = ui->listExperiments->currentIndex().row();

    if ( index != -1 )
    {
        Experiment * experiment = new Experiment();

        experiment->name = experiments.at(index);

        experiment->tR = Settings::getExperimentParameter( experiment->name, "TR" ).toDouble();
        experiment->tEcho = Settings::getExperimentParameter( experiment->name, "TEcho" ).toDouble();
        experiment->t90 = Settings::getExperimentParameter( experiment->name, "T90" ).toDouble();
        experiment->t180 = Settings::getExperimentParameter( experiment->name, "T180" ).toDouble();
        experiment->nEchoes = Settings::getExperimentParameter( experiment->name, "nEchoes" ).toInt();
        experiment->nSamples = Settings::getExperimentParameter( experiment->name, "nSamples" ).toInt();

        ExperimentDialog dlg(experiment, false, this);

        if ( dlg.exec() == QDialog::Accepted )
        {
            Settings::setExperimentParameter( experiment->name, "TR", QVariant( experiment->tR ) );
            Settings::setExperimentParameter( experiment->name, "TEcho", QVariant( experiment->tEcho ) );
            Settings::setExperimentParameter( experiment->name, "T90", QVariant( experiment->t90 ) );
            Settings::setExperimentParameter( experiment->name, "T180", QVariant( experiment->t180 ) );
            Settings::setExperimentParameter( experiment->name, "nEchoes", QVariant( experiment->nEchoes ) );
            Settings::setExperimentParameter( experiment->name, "nSamples", QVariant( experiment->nSamples ) );
        }
    }
}

void MainWindow::on_buttonDelete_clicked()
{
    int index = ui->listExperiments->currentIndex().row();

    if ( index != -1 )
    {
        QMessageBox::StandardButton reply = QMessageBox::question( this, "Confirmar", "¿Confirma que desea eliminar el experimento?",
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            Settings::deleteExperiment( experiments.at( index ) );

            experiments.removeAt( index );
            model->setStringList( experiments );
            Settings::setExperiments( experiments );

            ui->comboExperiments->clear();
            ui->comboExperiments->addItems(experiments);
        }
    }
}

void MainWindow::on_comboExperiments_currentIndexChanged(const QString &arg1)
{
    qDebug() << "selectedExperiment " << arg1;
    Settings::setSelectedExperiment(arg1);
}

void MainWindow::on_buttonStop_clicked()
{
    if ( programmer1 != NULL )
        programmer1->setFinished(true);
}
