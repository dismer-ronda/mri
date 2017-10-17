#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class MainWindow;
class ProgrammerThread;
class CheckThread;

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QMutex>
#include <QStringListModel>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>

#include "programmerthread.h"

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
    ProgrammerThread * programmer1;
    QStringList experiments;
    QStringListModel * model;

    QLineSeries * series1;
    QLineSeries * series2;

    QChart *chart;
    QChartView *chartView;

public:
    static QString binDir;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setFinished( bool value );
    bool isFinished();

    void setChartSeries( QLineSeries * series );
    QLineSeries * getChartSeries();

    void startExperiment();

private slots:
    void on_buttonProgram_clicked();
    void on_buttonTerminate_clicked();

    void on_buttonAdd_clicked();
    void on_buttonModify_clicked();
    void on_buttonDelete_clicked();

    void on_comboExperiments_currentIndexChanged(const QString &arg1);

    void on_buttonStop_clicked();

protected:
    void timerEvent(QTimerEvent *event);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
