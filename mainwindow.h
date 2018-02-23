#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class MainWindow;
class ProgrammerThread;
class CheckThread;

#include <QMainWindow>
#include <QComboBox>
#include <QMutex>
#include <QStringListModel>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QProgressBar>
#include <QListView>

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

    QChart *chartReal;
    QChartView *chartViewReal;

    QChart *chartImag;
    QChartView *chartViewImag;

    QChart *chartMod;
    QChartView *chartViewMod;

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

    QListView * listExperiments;

public:
    static QString binDir;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setFinished( bool value );
    bool isFinished();

    void setChartSeries( QLineSeries * series );
    QLineSeries * getChartSeries();

    void startExperiment( const QString & name );
    void updateExperimentsButtons();

private slots:
    void on_buttonTerminate_clicked();
    void on_buttonTab1_clicked();
    void on_buttonTab2_clicked();
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();

    void on_buttonAdd_clicked();
    void on_buttonModify_clicked();
    void on_buttonDelete_clicked();

    void on_comboExperiments_currentIndexChanged(const QString &arg1);

protected:
    void timerEvent(QTimerEvent *event);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
