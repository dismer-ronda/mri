#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class MainWindow;
class ExperimentThread;
class CheckThread;

#include <QMainWindow>
#include <QComboBox>
#include <QMutex>
#include <QStringListModel>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QProgressBar>
#include <QListView>
#include <QTextEdit>
#include <QLabel>
#include <QAbstractAxis>

#include "experimentthread.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    bool finished;
    QMutex mutex;
    int timerId;
    ExperimentThread * programmer1;
    QStringList experiments;
    QStringListModel * model;

    QLineSeries * series1;
    QLineSeries * series2;
    QLineSeries * series3;

    QChart *chart1;
    QChartView *chartView1;

    QChart *chart2;
    QChartView *chartView2;

    QChart *chart3;
    QChartView *chartView3;

    QWidget * tab1;
    QWidget * tab2;

    QList<QPushButton*> expButtons;

    QProgressBar * progressBar;

    QPushButton * buttonTerminate;
    QPushButton * buttonTab1;
    QPushButton * buttonTab2;
    QPushButton * buttonStop;

    QWidget * rowSelectExperiment;

    QPushButton * buttonAdd;
    QPushButton * buttonEdit;
    QPushButton * buttonDelete;

    QTextEdit * editExperiments;

    bool experimentsChanged;

    QLabel * labelHeader;

public:
    static QString binDir;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setFinished( bool value );
    bool isFinished();

    void setChartSeries( QLineSeries * seriesOrg, QLineSeries * seriesDest );

    void setChartSeries1( QLineSeries * series );
    void setChartSeries2( QLineSeries * series );
    void setChartSeries3( QLineSeries * series );

    QLineSeries * getChartSeries1();
    QLineSeries * getChartSeries2();
    QLineSeries * getChartSeries3();

    void startExperiment( const QString & name );
    void updateExperimentsButtons();

    void setChart1Axis( QAbstractAxis * axis );
    void setChart1AxisDefault();

    void setChart2Axis( QAbstractAxis * axis );
    void setChart2AxisDefault();

    void setChart3Axis( QAbstractAxis * axis );
    void setChart3AxisDefault();

private slots:
    void on_buttonTerminate_clicked();

    void on_buttonTab1_clicked();
    void on_buttonTab2_clicked();

    void on_buttonStart_clicked();
    void on_buttonStop_clicked();

    void on_experimentsChanged();

protected:
    void timerEvent(QTimerEvent *event);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
