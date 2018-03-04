#include <QDebug>
#include <QLayout>
#include <QMessageBox>
#include <QStringListModel>
#include <QScreen>
#include <QPushButton>
#include <QTextEdit>
#include <QValueAxis>

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

    int maxHeight = screenGeometry.height();
    int maxWidth = screenGeometry.width();

    int height = screenGeometry.height() * 0.6;
    int width = screenGeometry.width() * 0.8;

    int border = getBorderThickness();

    int heightButtons = 56;
    int widthButtons = 220;

    setWindowFlags(Qt::FramelessWindowHint);
    setGeometry( 0, 0, maxWidth, maxHeight );

    setStyleSheet( QString( "border: %1px solid #%2; background-color: #%3" )
                   .arg( Settings::getCustom( "window.borderThickness", "1" ) )
                   .arg( Settings::getCustom( "window.borderColor", "0000FF" ) )
                   .arg( Settings::getCustom( "window.background", "FFFFFF" ) ) );

    QWidget * mainWidget = new QWidget( this );
    mainWidget->setStyleSheet( "border: none" );
    mainWidget->setGeometry( (maxWidth - width)/2, (maxHeight - height)/2, width, height );
    mainWidget->show();
    QRect crMain = mainWidget->geometry();

    QWidget * rowButtons = new QWidget( mainWidget );
    rowButtons->setGeometry( width - 3 * widthButtons, 0, 3 * widthButtons, heightButtons );
    rowButtons->show();

    buttonTab1 = createButton( rowButtons, Settings::getText( "execution" ) );
    buttonTab1->setStyleSheet( modifyStyleSheet( buttonTab1->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "tab.selected", "FFFFFF" ) ) ) );
    buttonTab1->setGeometry( 0, 0, widthButtons, heightButtons );
    buttonTab1->show();
    connect(buttonTab1, SIGNAL(clicked()), this, SLOT(on_buttonTab1_clicked()));

    buttonTab2 = createButton( rowButtons, Settings::getText( "configuration" ) );
    buttonTab2->setGeometry( widthButtons + border, 0, widthButtons, heightButtons );
    buttonTab2->show();
    connect(buttonTab2, SIGNAL(clicked()), this, SLOT(on_buttonTab2_clicked()));

    buttonTerminate = createButton( rowButtons, Settings::getText( "finish" ) );
    buttonTerminate->setGeometry( widthButtons + widthButtons + 2 * border, 0, widthButtons, heightButtons );
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
    tab1->setStyleSheet( QString( "font-size: %1px; font-weight: normal; color: #%2" )
                         .arg( Settings::getCustom( "window.fontSize", "16" ) )
                         .arg( Settings::getCustom( "window.color", "075B91" ) ) );
    tab1->setGeometry( 0, 0, crContents.right(), crContents.height() );
    tab1->show();

    QRect crTab1 = tab1->geometry();

    QWidget * rowTop = new QWidget( tab1 );
    //rowTop->setStyleSheet( "border: 1px solid red" );
    rowTop->setGeometry( 0, 0, crTab1.width(), crTab1.height() / 2 );
    rowTop->show();

    QRect crTop = tab1->geometry();

    labelHeader = new QLabel( rowTop );
    labelHeader->setStyleSheet( QString( "font-size: %1px; font-weight: bold; color: #%2" )
                         .arg( Settings::getCustom( "label.fontSize", "24" ) )
                         .arg( Settings::getCustom( "label.color", "075B91" ) ) );
    labelHeader->setText( Settings::getText( "instructions" ) );
    labelHeader->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
    labelHeader->setGeometry( 0, 0, crTab1.right(), 2 * heightButtons );

    rowSelectExperiment = new QWidget( rowTop );
    //rowSelectExperiment->setStyleSheet( "border: 1px solid black" );
    rowSelectExperiment->setGeometry( 0, 2 * heightButtons, crTop.right(), heightButtons );
    rowSelectExperiment->show();

    updateExperimentsButtons();

    QRect crSelect = rowSelectExperiment->geometry();

    buttonStop = createButton( rowTop, Settings::getText( "stop" ) );
    buttonStop->setGeometry( 0, 2 * heightButtons, widthButtons, crSelect.height() );
    buttonStop->hide();

    connect(buttonStop, SIGNAL(clicked()), this, SLOT(on_buttonStop_clicked()));

    progressBar = new QProgressBar( rowTop );
    progressBar->setGeometry( widthButtons + border, 2 * heightButtons, crTop.width() - widthButtons - 2 * border, heightButtons);
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
    chartViewReal->hide();

    chartImag = new QChart();
    chartImag->legend()->hide();
    chartImag->createDefaultAxes();
    chartViewImag = new QChartView(chartImag, rowBottom);
    chartViewImag->setRenderHint(QPainter::Antialiasing);
    chartViewImag->setGeometry(widthSignal, 0, widthSignal, cr5.height() );
    chartViewImag->hide();

    chartMod = new QChart();
    chartMod->legend()->hide();
    chartViewMod = new QChartView(chartMod, rowBottom);
    chartViewMod->setRenderHint(QPainter::Antialiasing);
    chartViewMod->setGeometry(2 * widthSignal, 0, widthSignal, cr5.height() );
    chartViewMod->hide();

    tab2 = new QWidget( tabContents );
    tab2->setStyleSheet( QString( "font-size: %1px; font-weight: bold; color: #%2" )
                         .arg( Settings::getCustom( "window.fontSize", "16" ) )
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

    seriesReal = NULL;
    seriesImag = NULL;
    seriesMod = NULL;

    experimentsChanged = false;
}

MainWindow::~MainWindow()
{
    if ( programmer1 != NULL )
        delete programmer1;
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

void MainWindow::setChartSeriesReal( QLineSeries * series )
{
    mutex.lock();
    if ( seriesReal != NULL )
        delete seriesReal;
    seriesReal = series;
    mutex.unlock();
}
void MainWindow::setChartSeriesImag( QLineSeries * series )
{
    mutex.lock();
    if ( seriesImag != NULL )
        delete seriesImag;
    seriesImag = series;
    mutex.unlock();
}

void MainWindow::setChartSeriesMod( QLineSeries * series )
{
    mutex.lock();
    if ( seriesMod != NULL )
        delete seriesMod;
    seriesMod = series;
    mutex.unlock();
}

QLineSeries * MainWindow::getChartSeriesReal()
{
    QLineSeries * ret = new QLineSeries();

    mutex.lock();
    if ( seriesReal != NULL )
    {
        ret->append( seriesReal->points() );
        delete seriesReal;
        seriesReal = NULL;
    }
    mutex.unlock();

    return ret;
}

QLineSeries * MainWindow::getChartSeriesImag()
{
    QLineSeries * ret = new QLineSeries();

    mutex.lock();
    if ( seriesImag != NULL )
    {
        ret->append( seriesImag->points() );
        delete seriesImag;
        seriesImag = NULL;
    }
    mutex.unlock();

    return ret;
}

QLineSeries * MainWindow::getChartSeriesMod()
{
    QLineSeries * ret = new QLineSeries();

    mutex.lock();
    if ( seriesMod != NULL )
    {
        ret->append( seriesMod->points() );
        delete seriesMod;
        seriesMod = NULL;
    }
    mutex.unlock();

    return ret;
}

void MainWindow::startExperiment( const QString & name )
{
    setFinished(false);

    QString type = Settings::getExperimentParameter( name, "Type" ).toString();

    qDebug() << type;

    if ( programmer1 != NULL )
        delete programmer1;

    if ( type.compare( "PulseAcquire" ) == 0 )
        programmer1 = new PulseAcquireThread( MainWindow::binDir, name, this );
    else if ( type.compare("SpinEcho") == 0 )
        programmer1 = new SpinEchoThread( MainWindow::binDir, name, this );

    chartReal->removeAllSeries();
    chartImag->removeAllSeries();
    chartMod->removeAllSeries();

    chartViewReal->update();
    chartViewImag->update();
    chartViewMod->update();

    programmer1->start();

    progressBar->setMinimum(0);
    progressBar->setMaximum( programmer1->getProgressCount() );
    progressBar->setAlignment(Qt::AlignHCenter|Qt::AlignCenter);
    progressBar->setValue(0);
    progressBar->setStyleSheet( QString( "QProgressBar { border: %1px solid #%2; font-size: %3px; font-weight: bold; color: #%4; background-color: #%5 } "
                                         "QProgressBar::chunk { background-color: #%6}" )
                                .arg( Settings::getCustom( "progress.borderThickness", "5" ) )
                                .arg( Settings::getCustom( "progress.borderColor", "075B91" ) )
                                .arg( Settings::getCustom( "progress.fontSize", "16" ) )
                                .arg( Settings::getCustom( "progress.color", "075B91" ) )
                                .arg( Settings::getCustom( "progress.background", "FFFFFF" ) )
                                .arg( Settings::getCustom( "progress.background.filled", "9A816A" ) ) );

    buttonTerminate->hide();
    rowSelectExperiment->hide();
    buttonTab1->hide();
    buttonTab2->hide();

    progressBar->show();
    buttonStop->show();

    chartViewReal->show();
    chartViewImag->show();
    chartViewMod->show();

    labelHeader->setText( Settings::getText( "stop.instructions" ) );

    timerId = startTimer( programmer1->getProgressTimer() );
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if ( !isFinished() )
    {
        int value = progressBar->value() + 1;
        int size = progressBar->maximum();
        
        progressBar->setValue( value );

        QLineSeries * series = getChartSeriesReal();

        if ( series->count() > 0 )
        {
            chartReal->removeAllSeries();
            chartReal->addSeries(series);

            chartViewReal->update();
        }

        series = getChartSeriesImag();

        if ( series->count() > 0 )
        {
            chartImag->removeAllSeries();
            chartImag->addSeries(series);

            chartViewImag->update();
        }

        series = getChartSeriesMod();

        if ( series->count() > 0 )
        {
            chartMod->removeAllSeries();
            chartMod->addSeries(series);

            chartViewMod->update();
        }

        if ( value == size )
        {
            if ( programmer1 != NULL )
                programmer1->setFinished(true);
        }

        /*MEMORYSTATUSEX memory_status;
        ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
        memory_status.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memory_status)) {
          system_info.append(
                QString("RAM: %1 MB")
                .arg(memory_status.ullTotalPhys / (1024 * 1024)));
        } else {
          system_info.append("Unknown RAM");
        }*/
    }
    else
    {
        killTimer(timerId);

        chartViewReal->hide();
        chartViewImag->hide();
        chartViewMod->hide();

        progressBar->hide();
        buttonStop->hide();

        buttonTerminate->show();
        rowSelectExperiment->show();
        buttonTab1->show();
        buttonTab2->show();

        labelHeader->setText( Settings::getText( "instructions" ) );

        if ( programmer1->errorCode != 0 )
            QMessageBox::question(this, "Error", "Error", QMessageBox::Ok);
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

    buttonTab1->setStyleSheet( modifyStyleSheet( buttonTab1->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "tab.selected", "FFFFFF" ) ) ) );
    buttonTab2->setStyleSheet( modifyStyleSheet( buttonTab2->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "button.background", "FFFFFF" ) ) ) );

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

    buttonTab2->setStyleSheet( modifyStyleSheet( buttonTab2->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "tab.selected", "FFFFFF" ) ) ) );
    buttonTab1->setStyleSheet( modifyStyleSheet( buttonTab1->styleSheet(), "background-color", QString( "#%1" ).arg( Settings::getCustom( "button.background", "FFFFFF" ) ) ) );

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

void MainWindow::setModChartAxis( QAbstractAxis * axis )
{
    chartMod->removeAxis( chartMod->axisX() );
    chartMod->addAxis( axis, Qt::AlignBottom );
}

void MainWindow::setModChartAxisDefault()
{
    chartMod->removeAxis( chartMod->axisX() );
    chartMod->createDefaultAxes();
}

