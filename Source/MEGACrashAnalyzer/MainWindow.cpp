#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QString>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QMap>

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
    if (path.length())
    {
        parseCrashes(path);
    }
}

void MainWindow::parseCrashes(QString folder)
{
    QDir dir(folder);
    QFileInfoList fiList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for (int i = 0; i < fiList.size(); i++)
    {
        QFile file(fiList[i].absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly))
        {
            continue;
        }

        QString fileContents = QString::fromUtf8(file.readAll());
        fileContents.replace(QString::fromUtf8("\r\n"), QString::fromUtf8("\n"));
        QStringList crashReports = fileContents.split(QString::fromUtf8("MEGAprivate ERROR DUMP"), QString::SkipEmptyParts);
        if ((crashReports.size() - 2) >= 0
                && crashReports[crashReports.size() - 2].split(QString::fromUtf8("\n")).size() < 3)
        {
            crashReports.removeLast();
        }

        for (int j = 0; j < crashReports.size() - 1; j++)
        {
            QString crashReport = crashReports[j];
            file.close();

            QStringList lines = crashReport.split(QString::fromUtf8("\n"));
            while (lines.size() && !lines.at(0).startsWith(QString::fromUtf8("Application: ")))
            {
                lines.removeAt(0);
            }

            if ((lines.size() < 5)
                    || (!lines.at(0).startsWith(QString::fromUtf8("Application: ")))
                    || (!lines.at(1).startsWith(QString::fromUtf8("Version"))))
            {
                continue;
            }

            int locationIndex = 0;
            if (lines.at(3).startsWith(QString::fromUtf8("Operating")))
            {
                locationIndex = 5;
            }
            if (lines.at(4).startsWith(QString::fromUtf8("System")))
            {
                locationIndex = 8;
            }

            if (!locationIndex || lines.size() <= locationIndex)
            {
                continue;
            }

            QHash<QString, QStringList> &version = reports[lines.at(1)];
            if (!reports.contains(lines.at(1)))
            {
                reports.insert(lines.at(1), QHash<QString, QStringList>());
            }

            if (!version.contains(lines.at(locationIndex)))
            {
                version.insert(lines.at(locationIndex), QStringList());
            }

            QStringList &fullReports = version[lines.at(locationIndex)];
            for (int i = locationIndex; i < lines.size(); i++)
            {
                if (lines.at(i).contains("------------------------------"))
                {
                    int init = i;
                    int j = i + 1;
                    while (j < lines.size() && !lines.at(j).contains("------------------------------"))
                    {
                        j++;
                    }

                    QString comment;
                    if (j != lines.size())
                    {
                        i++;
                        while (i < j)
                        {
                            comment.append(lines.at(i));
                            i++;
                        }
                        comment = comment.trimmed();
                    }

                    while (lines.size() > init)
                    {
                        lines.removeAt(lines.size() - 1);
                    }

                    if (comment.size() > 3)
                    {
                        comment.append(QString::fromUtf8("\n\nCrash report:\n"));
                        comment.append(lines.join("\n"));
                        //QMessageBox::warning(this, tr("Crash analyzer"), comment);
                        qDebug() << QString::fromUtf8("User comment: %1\n\n").arg(comment);
                    }

                    break;
                }
            }

            crashReport = lines.join(QString::fromUtf8("\n"));
            fullReports.append(crashReport);
        }
    }

    QStringList versions = reports.keys();
    ui->cVersion->clear();
    if (versions.size())
    {
        QMap<int, QString> sortedMap;
        for (int i=0; i < versions.size(); i++)
        {
            QHash<QString, QStringList> &hVersion = reports[versions.at(i)];
            QStringList crashLocations = hVersion.keys();
            int acum = 0;
            for (int j = 0; j < crashLocations.size(); j++)
            {
                acum += hVersion.value(crashLocations[j]).size();
            }
            sortedMap[acum] = versions.at(i);
        }
        QStringList sortedVersions = sortedMap.values();
        std::reverse(sortedVersions.begin(), sortedVersions.end());
        ui->cVersion->addItems(sortedVersions);
        ui->cVersion->setCurrentIndex(0);
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
    if (crashLocations.size())
    {
        QMap<int, QString> sortedMap;
        for (int i=0; i< crashLocations.size(); i++)
        {
            sortedMap[hVersion.value(crashLocations.at(i)).size()] = crashLocations.at(i);
        }
        QStringList sortedLocations = sortedMap.values();
        std::reverse(sortedLocations.begin(), sortedLocations.end());
        ui->cLocation->addItems(sortedLocations);
        ui->cLocation->setCurrentIndex(0);

        int acum = 0;
        for (int i = 0; i < crashLocations.size(); i++)
        {
            acum += hVersion.value(crashLocations[i]).size();
        }
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
    QStringList fullReports = hVersion.value(location);
    ui->eLocation->setText(QString::number(fullReports.size()));
    if (fullReports.size())
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
    if (selected > 0)
    {
        QHash<QString, QStringList> &hVersion = reports[ui->cVersion->currentText()];
        QStringList fullReports = hVersion.value(ui->cLocation->currentText());
        ui->eReport->setText(fullReports[selected-1]);
    }
    else
    {
        ui->eReport->clear();
    }
}
