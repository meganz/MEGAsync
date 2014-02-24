#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QString>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_bSourceFolder_clicked();

    void on_cVersion_currentIndexChanged(const QString &version);

    void on_cLocation_currentIndexChanged(const QString &location);

    void on_sReports_valueChanged(int selected);

private:
    Ui::MainWindow *ui;
    QHash<QString, QHash<QString, QStringList> > reports;
    void parseCrashes(QString folder);
};

#endif // MAINWINDOW_H
