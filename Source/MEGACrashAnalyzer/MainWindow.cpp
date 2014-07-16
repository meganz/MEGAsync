#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QString>
#include <QDesktopServices>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_bSourceFolder_clicked()
{
    #if QT_VERSION < 0x050000
        QString defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    #else
        QString defaultPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
    #endif

    QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
                                                      defaultPath,
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    if(path.length())
        parseCrashes(path);
}

void MainWindow::parseCrashes(QString folder)
{
    QDir dir(folder);
    QFileInfoList fiList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for(int i=0; i<fiList.size(); i++)
    {
        QFile file(fiList[i].absoluteFilePath());
        if(file.size()>16384)
            continue;

        if(!file.open(QIODevice::ReadOnly))
            continue;

        QString fileContents = QString::fromUtf8(file.readAll());
        QStringList crashReports = fileContents.split(QString::fromAscii("------------------------------\n"), QString::SkipEmptyParts);
        if(crashReports.size()-2 >= 0 && crashReports[crashReports.size()-2].split(QString::fromAscii("\n")).size()<3)
                crashReports.removeLast();

        for(int j=0; j<crashReports.size()-1; j++)
        {
            QString crashReport = crashReports[j];
            file.close();

            QStringList lines = crashReport.split(QString::fromAscii("\n"));
            if((lines.size()<3) ||
             (lines.at(0) != QString::fromAscii("MEGAprivate ERROR DUMP")) ||
             (!lines.at(1).startsWith(QString::fromAscii("Application: "))) ||
             (!lines.at(2).startsWith(QString::fromAscii("Version"))))
            {
                continue;
            }

            if(!reports.contains(lines.at(2)))
                reports.insert(lines.at(2), QHash<QString, QStringList>());

            QHash<QString, QStringList> &version = reports[lines.at(2)];
            int locationIndex = 5;
            if(lines.at(4).startsWith(QString::fromUtf8("Operating")))
                locationIndex = 6;
            if(lines.at(5).startsWith(QString::fromUtf8("System")))
                locationIndex = 9;
            if(!version.contains(lines.at(locationIndex)))
                version.insert(lines.at(locationIndex), QStringList());

            QStringList &fullReports = version[lines.at(locationIndex)];
            crashReport.insert(0, tr("File: ") + fiList[i].fileName() + QString::fromAscii("\n\n"));
            if(crashReports[crashReports.size()-1].size())
                crashReport.insert(0, tr("User comment: ") + crashReports[crashReports.size()-1]);
            fullReports.append(crashReport);
        }
    }

    QStringList versions = reports.keys();
    ui->cVersion->clear();
    if(versions.size())
    {
        versions.sort();
        ui->cVersion->addItems(versions);
        ui->cVersion->setCurrentIndex(versions.size()-1);
    }
    else
    {
        ui->cVersion->addItem(tr("Version code"));
    }
}

void MainWindow::on_cVersion_currentIndexChanged(const QString &version)
{
    QHash<QString, QStringList> &hVersion = reports[version];
    QStringList crashLocations = hVersion.keys();
    ui->cLocation->clear();
    if(crashLocations.size())
    {
        crashLocations.sort();
        ui->cLocation->addItems(crashLocations);
        ui->cLocation->setCurrentIndex(0);

        int acum = 0;
        QHash<QString, QStringList> &hVersion = reports[ui->cVersion->currentText()];
        for(int i=0; i<crashLocations.size(); i++)
            acum += hVersion[crashLocations[i]].size();

        ui->eVersion->setText(QString::number(acum));
    }
    else
    {
        ui->eVersion->setText(QString::number(0));
        ui->cLocation->addItem(tr("Crash location"));
    }
}

void MainWindow::on_cLocation_currentIndexChanged(const QString &location)
{
    QHash<QString, QStringList> &hVersion = reports[ui->cVersion->currentText()];
    QStringList &fullReports = hVersion[location];
    ui->eLocation->setText(QString::number(fullReports.size()));
    if(fullReports.size())
    {
        ui->sReports->setMinimum(1);
        ui->sReports->setMaximum(fullReports.size());
        ui->sReports->setValue(1);
        ui->eReport->setText(fullReports[0]);
    }
    else
    {
        ui->sReports->setMinimum(0);
        ui->sReports->setMaximum(0);
        ui->sReports->setValue(0);
        ui->eReport->clear();
    }
}

void MainWindow::on_sReports_valueChanged(int selected)
{
    if(selected > 0)
    {
        QHash<QString, QStringList> &hVersion = reports[ui->cVersion->currentText()];
        QStringList &fullReports = hVersion[ui->cLocation->currentText()];
        ui->eReport->setText(fullReports[selected-1]);
    }
    else ui->eReport->clear();
}
