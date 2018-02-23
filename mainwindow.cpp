#include <QDebug>
#include <QLayout>
#include <QMessageBox>
#include <QStringListModel>
#include <QScreen>
#include <QPushButton>

#include "settings.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "programmerthread.h"
#include "experimentdialog.h"

QString MainWindow::binDir;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
#ifndef LINUX_BOX
    binDir = "c:/mri";
#else
    binDir = "/opt/mri";
#endif

    Settings::loadSettings( binDir + "/setup.ini" );

    QRect  screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();

    int height = screenGeometry.height();
    int width = screenGeometry.width();
    int border = getBorderThickness();
    int heightButtons = 56;
    int widthTab1 = 220;
    int widthTab2 = 220;
    int widthExit = 220;

    setWindowFlags(Qt::FramelessWindowHint);
    setGeometry(0, 0, width, height );

    QWidget * mainWidget = new QWidget( this );
    mainWidget->setGeometry( border, border, width - 2 * border, height - 2 * border );
    mainWidget->show();
    QRect crMain = mainWidget->geometry();

    buttonTab1 = createButton( mainWidget, "Experimento" );
    buttonTab1->setGeometry( 0, 0, widthTab1, heightButtons );
    buttonTab1->show();
    connect(buttonTab1, SIGNAL(clicked()), this, SLOT(on_buttonTab1_clicked()));

    buttonTab2 = createButton( mainWidget, "Configuración" );
    buttonTab2->setGeometry( widthTab1 + border, 0, widthTab2, heightButtons );
    buttonTab2->show();
    connect(buttonTab2, SIGNAL(clicked()), this, SLOT(on_buttonTab2_clicked()));

    buttonTerminate = createButton( mainWidget, "Terminar" );
    buttonTerminate->setGeometry( widthTab1 + widthTab2 + 2 * border, 0, widthExit, heightButtons );
    buttonTerminate->show();
    connect(buttonTerminate, SIGNAL(clicked()), this, SLOT(on_buttonTerminate_clicked()));

    QWidget * tabContents = new QWidget(mainWidget);
    tabContents->setStyleSheet( QString( "font-size: %1px; font-weight: bold; color: #%2" )
                         .arg( Settings::getCustom( "tab.fontSize", "24" ) )
                         .arg( Settings::getCustom( "tab.color", "075B91" ) ) );
    tabContents->setGeometry( 0, border + heightButtons, crMain.width(), crMain.height() - heightButtons - border );
    tabContents->show();
    QRect crContents = tabContents->geometry();

    tab1 = new QWidget( tabContents );
    tab1->setStyleSheet( QString( "font-size: %1px; font-weight: normal; background-color: #%2; color: #%3" )
                         .arg( Settings::getCustom( "window.fontSize", "16" ) )
                         .arg( Settings::getCustom( "window.background", "FFFFFF" ) )
                         .arg( Settings::getCustom( "window.color", "075B91" ) ) );
    tab1->setGeometry( 0, 0, crContents.right(), crContents.height() );
    tab1->show();

    tab2 = new QWidget( tabContents );
    tab2->setStyleSheet( QString( "font-size: %1px; font-weight: bold; background-color: #%2; color: #%3" )
                         .arg( Settings::getCustom( "window.fontSize", "16" ) )
                         .arg( Settings::getCustom( "window.background", "FFFFFF" ) )
                         .arg( Settings::getCustom( "window.color", "075B91" ) ) );
    tab2->setGeometry( 0, 0, crContents.right(), crContents.height() );
    tab2->hide();

    QRect crTab1 = tab1->geometry();

    QWidget * rowTop = new QWidget( tab1 );
    //rowTop->setStyleSheet( "border: 1px solid red" );
    rowTop->setGeometry( 0, 0, crTab1.width(), crTab1.height() / 2 );
    rowTop->show();

    QRect crTop = tab1->geometry();

    QLabel * labelSelect = new QLabel( rowTop );
    labelSelect->setStyleSheet( QString( "font-size: %1px; font-weight: bold; color: #%2" )
                         .arg( Settings::getCustom( "label.fontSize", "24" ) )
                         .arg( Settings::getCustom( "label.color", "075B91" ) ) );
    labelSelect->setText("Prepare la muestra y presione el botón correspondiente al experimento" );
    labelSelect->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
    labelSelect->setGeometry( 0, 0, crTab1.right(), 64 );

    rowSelectExperiment = new QWidget( rowTop );
    //rowSelectExperiment->setStyleSheet( "border: 1px solid black" );
    rowSelectExperiment->setGeometry( 0, 64, crTop.right(), heightButtons + 2 * border );
    rowSelectExperiment->show();

    experiments = Settings::getExperiments();

    QRect crSelect = rowSelectExperiment->geometry();

    int widthExperiment = crSelect.width() / experiments.size();

    for ( int i = 0; i < experiments.size(); i++ )
    {
        QString text = experiments.at(i);

        QPushButton * button = createButton( rowSelectExperiment, text );
        button->setGeometry( i * widthExperiment, 0, widthExperiment - (i < experiments.size() - 1 ? border : 0), crSelect.height() );
        button->show();

        connect(button, SIGNAL(clicked()), this, SLOT(on_buttonStart_clicked()));

        expButtons.append(button);
    }

    buttonStop = createButton( rowTop, "Detener" );
    buttonStop->setGeometry( 0, 64, widthExit, crSelect.height() );
    buttonStop->hide();

    connect(buttonStop, SIGNAL(clicked()), this, SLOT(on_buttonStop_clicked()));

    progressBar = new QProgressBar( rowTop );
    progressBar->setGeometry( widthExit + border, 64, crTop.width() - widthExit - 2 * border, heightButtons + 2 * border );
    progressBar->hide();

    QWidget * rowBottom = new QWidget( tab1 );
    rowBottom->setStyleSheet( "border: 1px solid red" );
    rowBottom->setGeometry( 0, crTab1.height() / 2, crTab1.width(), crTab1.height()/2 );
    rowBottom->show();

    QRect cr5 = rowBottom->geometry();
    int widthSignal = cr5.width() / 3;

    chartReal = new QChart();
    chartReal->legend()->hide();
    chartReal->createDefaultAxes();
    chartViewReal = new QChartView(chartReal, rowBottom);
    chartViewReal->setRenderHint(QPainter::Antialiasing);
    chartViewReal->setGeometry(0, 0, widthSignal, cr5.height() );
    chartViewReal->show();

    chartImag = new QChart();
    chartImag->legend()->hide();
    chartImag->createDefaultAxes();
    chartViewImag = new QChartView(chartImag, rowBottom);
    chartViewImag->setRenderHint(QPainter::Antialiasing);
    chartViewImag->setGeometry(widthSignal, 0, widthSignal, cr5.height() );
    chartViewImag->show();

    chartMod = new QChart();
    chartMod->legend()->hide();
    chartMod->createDefaultAxes();
    chartViewMod = new QChartView(chartMod, rowBottom);
    chartViewMod->setRenderHint(QPainter::Antialiasing);
    chartViewMod->setGeometry(2 * widthSignal, 0, widthSignal, cr5.height() );
    chartViewMod->show();

/*    QRect crRowExperiment = rowSelectExperiment->geometry();

    QLabel * labelSelect = new QLabel( rowSelectExperiment );
    labelSelect->setText("Prepare la muestra y presione el botón \"Iniciar\"" );
    labelSelect->setGeometry( border, border, crTab1.right() - 2 * border, 32 );
    labelSelect->show();

    experiments = Settings::getExperiments();
    model = new QStringListModel( experiments, NULL);

    QComboBox * comboExperiments = new QComboBox(rowSelectExperiment );
    comboExperiments->setModel( model );
    comboExperiments->addItems( experiments );
    comboExperiments->setCurrentText( Settings::getSelectdExperiment() );

*/
    /*QRect cr1 = ui->widgetProgramming->geometry();
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

    chartReal = new QChart();
    chartReal->legend()->hide();
    chartReal->createDefaultAxes();
    chartViewReal = new QChartView(chartReal, ui->signalWidget);
    chartViewReal->setRenderHint(QPainter::Antialiasing);

    chartImag = new QChart();
    chartImag->legend()->hide();
    chartImag->createDefaultAxes();
    chartViewImag = new QChartView(chartImag, ui->signalWidget);
    chartViewImag->setRenderHint(QPainter::Antialiasing);

    chartMod = new QChart();
    chartMod->legend()->hide();
    chartMod->createDefaultAxes();
    chartViewMod = new QChartView(chartMod, ui->signalWidget);
    chartViewMod->setRenderHint(QPainter::Antialiasing);

    QRect cr5 = ui->signalWidget->geometry();
    int width = cr5.width() / 3;

    chartViewReal->setGeometry(0, 0, width, cr5.height() );
    chartViewReal->show();

    chartViewImag->setGeometry(width, 0, width, cr5.height() );
    chartViewImag->show();

    chartViewMod->setGeometry(2 * width, 0, width, cr5.height() );
    chartViewMod->show();
*/
    programmer1 = NULL;

    series1 = NULL;
    series2 = NULL;
}

MainWindow::~MainWindow()
{
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

void MainWindow::startExperiment( const QString & name )
{
    setFinished(false);

    Experiment * experiment = new Experiment();

    experiment->name = name;

    experiment->tR = Settings::getExperimentParameter( experiment->name, "TR" ).toDouble();
    experiment->tEcho = Settings::getExperimentParameter( experiment->name, "TEcho" ).toDouble();
    experiment->t90 = Settings::getExperimentParameter( experiment->name, "T90" ).toDouble();
    experiment->t180 = Settings::getExperimentParameter( experiment->name, "T180" ).toDouble();
    experiment->nEchoes = Settings::getExperimentParameter( experiment->name, "nEchoes" ).toInt();
    experiment->nSamples = Settings::getExperimentParameter( experiment->name, "nSamples" ).toInt();
    experiment->nRepetitions = Settings::getExperimentParameter( experiment->name, "nRepetitions" ).toInt();

    chartReal->removeAllSeries();
    chartViewReal->update();

    programmer1 = new ProgrammerThread( MainWindow::binDir, experiment, this );
    programmer1->start();

    progressBar->setMinimum(0);
    progressBar->setMaximum( experiment->nRepetitions * (experiment->nEchoes == 0 ? 1 : experiment->nEchoes) );
    progressBar->setValue(0);

    buttonTerminate->hide();
    rowSelectExperiment->hide();
    buttonTab2->hide();

    progressBar->show();
    buttonStop->show();

    //signalWidget->show();

    //ui->tabWidget->setTabEnabled(1, false);

    timerId = startTimer(experiment->tR * 1000);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if ( !isFinished() )
    {
        int value = progressBar->value() + 1;
        int size = progressBar->maximum();
        
        progressBar->setValue( value );

        QLineSeries * series = getChartSeries();

        if ( series->count() > 0 )
        {
            chartReal->removeAllSeries();
            chartReal->addSeries(series);

            chartViewReal->update();
        }

        if ( value == size )
        {
            if ( programmer1 != NULL )
                programmer1->setFinished(true);
        }
    }
    else
    {
        killTimer(timerId);

        //label_Programming->setText("Prepare la muestra y presion el botón \"Iniciar\"");
        progressBar->hide();
        buttonStop->hide();

        buttonTerminate->show();
        rowSelectExperiment->show();
        buttonTab2->show();
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
        Settings::setExperimentParameter( experiment->name, "nRepetitions", QVariant( experiment->nRepetitions ) );
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
        experiment->nRepetitions = Settings::getExperimentParameter( experiment->name, "nRepetitions" ).toInt();

        ExperimentDialog dlg(experiment, false, this);

        if ( dlg.exec() == QDialog::Accepted )
        {
            Settings::setExperimentParameter( experiment->name, "TR", QVariant( experiment->tR ) );
            Settings::setExperimentParameter( experiment->name, "TEcho", QVariant( experiment->tEcho ) );
            Settings::setExperimentParameter( experiment->name, "T90", QVariant( experiment->t90 ) );
            Settings::setExperimentParameter( experiment->name, "T180", QVariant( experiment->t180 ) );
            Settings::setExperimentParameter( experiment->name, "nEchoes", QVariant( experiment->nEchoes ) );
            Settings::setExperimentParameter( experiment->name, "nSamples", QVariant( experiment->nSamples ) );
            Settings::setExperimentParameter( experiment->name, "nRepetitions", QVariant( experiment->nRepetitions ) );
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

void MainWindow::on_buttonTab1_clicked()
{
    tab2->hide();
    tab1->show();
}

void MainWindow::on_buttonTab2_clicked()
{
    tab1->hide();
    tab2->show();
}

void MainWindow::on_buttonStart_clicked()
{
    QPushButton * button = (QPushButton *)QObject::sender();

    qDebug() << "Start experiment" << button->text();

    startExperiment( button->text() );
}

void MainWindow::on_buttonStop_clicked()
{
    if ( programmer1 != NULL )
        programmer1->setFinished( true );
}
