#include <QDebug>
#include <QLayout>
#include <QMessageBox>
#include <QStringListModel>
#include <QScreen>
#include <QPushButton>
#include <QTextEdit>

#include "settings.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "experimentthread.h"
#include "pulseacquirethread.h"
#include "spinechothread.h"

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

    updateExperimentsButtons();

    QRect crSelect = rowSelectExperiment->geometry();

    buttonStop = createButton( rowTop, "Detener" );
    buttonStop->setGeometry( 0, 64, widthExit, crSelect.height() );
    buttonStop->hide();

    connect(buttonStop, SIGNAL(clicked()), this, SLOT(on_buttonStop_clicked()));

    progressBar = new QProgressBar( rowTop );
    progressBar->setGeometry( widthExit + border, 64, crTop.width() - widthExit - 2 * border, heightButtons + 2 * border );
    progressBar->hide();

    QWidget * rowBottom = new QWidget( tab1 );
    //rowBottom->setStyleSheet( "border: 1px solid red" );
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

    tab2 = new QWidget( tabContents );
    tab2->setStyleSheet( QString( "font-size: %1px; font-weight: bold; background-color: #%2; color: #%3" )
                         .arg( Settings::getCustom( "window.fontSize", "16" ) )
                         .arg( Settings::getCustom( "window.background", "FFFFFF" ) )
                         .arg( Settings::getCustom( "window.color", "075B91" ) ) );
    tab2->setGeometry( 0, 0, crContents.right(), crContents.height() );
    tab2->hide();

    QRect crTab2 = tab2->geometry();

    editExperiments = new QTextEdit( tab2 );
    editExperiments->setStyleSheet( QString( "border: 1px solid #%1; font-size: %2px; font-weight: bold" )
                         .arg( Settings::getCustom( "window.color", "075B91" ) )
                         .arg( Settings::getCustom( "window.fontSize", "18" ) ) );
    editExperiments->setGeometry( 0, 0, crTab2.width(), crTab2.height() );
    editExperiments->show();

    programmer1 = NULL;

    series1 = NULL;
    series2 = NULL;

    experimentsChanged = false;
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

    QString type = Settings::getExperimentParameter( name, "Type" ).toString();

    qDebug() << type;

    if ( type.compare( "PulseAcquire" ) == 0 )
        programmer1 = new PulseAcquireThread( MainWindow::binDir, name, this );
    else if ( type.compare("SpinEcho") == 0 )
        programmer1 = new SpinEchoThread( MainWindow::binDir, name, this );

    chartReal->removeAllSeries();
    chartViewReal->update();

    programmer1->start();

    progressBar->setMinimum(0);
    progressBar->setMaximum( programmer1->getProgressCount() );
    progressBar->setValue(0);

    buttonTerminate->hide();
    rowSelectExperiment->hide();
    buttonTab1->hide();
    buttonTab2->hide();

    progressBar->show();
    buttonStop->show();

    timerId = startTimer( programmer1->getProgressTimer() );
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

        progressBar->hide();
        buttonStop->hide();

        buttonTerminate->show();
        rowSelectExperiment->show();
        buttonTab1->show();
        buttonTab2->show();
    }
}

void MainWindow::on_buttonTerminate_clicked()
{
    QApplication::quit();
}

void MainWindow::on_buttonTab1_clicked()
{
    tab2->hide();
    tab1->show();

    disconnect(editExperiments, SIGNAL(textChanged()));

    if ( experimentsChanged )
    {
        writeTextFile( binDir + "/setup.ini", editExperiments->toPlainText() );

        Settings::loadSettings( binDir + "/setup.ini" );

        updateExperimentsButtons();
    }
}

void MainWindow::on_buttonTab2_clicked()
{
    editExperiments->setText( readTextFile( binDir + "/setup.ini" ) );

    connect(editExperiments, SIGNAL(textChanged()), this, SLOT(on_experimentsChanged()));

    tab1->hide();
    tab2->show();
}

void MainWindow::on_buttonStart_clicked()
{
    QPushButton * button = (QPushButton *)QObject::sender();

    qDebug() << "Start experiment" << button->text();

    startExperiment( button->text() );
}

void MainWindow::on_experimentsChanged()
{
    experimentsChanged = true;
}

void MainWindow::on_buttonStop_clicked()
{
    if ( programmer1 != NULL )
        programmer1->setFinished( true );
}

void MainWindow::updateExperimentsButtons()
{
    int border = getBorderThickness();

    for ( int i = 0; i < expButtons.size(); i++ )
        delete expButtons.at(i);
    expButtons.clear();

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
}
