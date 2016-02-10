#include "MegaDebugServer.h"
#include "ui_MegaDebugServer.h"
#include <iostream>

#define MEGA_LOGGER "MEGA_LOGGER"
#define ENABLE_MEGASYNC_LOGS "MEGA_ENABLE_LOGS"
#define MAX_LOG_MESSAGES 16384

using namespace std;

MegaDebugServer::MegaDebugServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MegaDebugServer)
{
    //Setup and set up GUI
    ui->setupUi(this);

    ui->statusBar->showMessage("Ready");
    megaSyncClient = NULL;
    megaServer = NULL;
    debugDataModel = NULL;
    debugProxyModel = NULL;
    reader = NULL;

    ui->filterTypeComboBox->addItem("Regular Expression", QRegExp::RegExp);
    ui->filterTypeComboBox->addItem("Wildcard", QRegExp::Wildcard);
    ui->filterTypeComboBox->addItem("Fixed string", QRegExp::FixedString);

    ui->columnComboBox->addItem("Timestamp");
    ui->columnComboBox->addItem("Message Type");
    ui->columnComboBox->addItem("Message");

    connect(ui->filterPatternLineEdit, SIGNAL(textChanged(QString)), this, SLOT(filterTextRegExp()));
    connect(ui->filterTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(filterTextRegExp()));
    connect(ui->columnComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(filterColumn()));
    connect(ui->caseSensitivecheckBox, SIGNAL(toggled(bool)), this, SLOT(filterCaseSensitive()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(tryConnect()));

    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveToFile()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(loadFromFile()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(clearDebugWindow()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(startstop()));

    debugDataModel = new QStandardItemModel(0, 3, this);
    debugDataModel->setHeaderData(0, Qt::Horizontal,QString::fromUtf8("Timestamp"), Qt::DisplayRole);
    debugDataModel->setHeaderData(1, Qt::Horizontal,QString::fromUtf8("Message Type"), Qt::DisplayRole);
    debugDataModel->setHeaderData(2, Qt::Horizontal,QString::fromUtf8("Message"), Qt::DisplayRole);

    debugProxyModel = new QSortFilterProxyModel;
    debugProxyModel->setSourceModel(debugDataModel);
    ui->messagesTreeView->setModel(debugProxyModel);

    ui->messagesTreeView->sortByColumn(1, Qt::AscendingOrder);
    ui->messagesTreeView->resizeColumnToContents(0);
    ui->messagesTreeView->resizeColumnToContents(1);
    ui->messagesTreeView->resizeColumnToContents(2);

    setWindowTitle(tr("MEGAsync Debug Window"));
    startstop();
}

void MegaDebugServer::clientConnected()
{
    timer.stop();

    ui->actionSave->setEnabled(false);
    ui->actionLoad->setEnabled(false);
    ui->statusBar->showMessage(tr("Connected"));

    for (;;)
    {
        megaSyncClient = megaServer->nextPendingConnection();
        if (!megaServer->hasPendingConnections())
        {
            break;
        }

        megaSyncClient->disconnectFromServer();
        megaSyncClient->deleteLater();
    }
    reader = new QXmlStreamReader(megaSyncClient);

    connect(megaSyncClient, SIGNAL(readyRead()), this, SLOT(readDebugMsg()));
    connect(megaSyncClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(megaSyncClient, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));
}

void MegaDebugServer::parseReader(QXmlStreamReader *reader)
{    
    do
    {
        QXmlStreamReader::TokenType token = reader->readNext();
        if (token == QXmlStreamReader::StartElement && reader->name() == "log")
        {
            QXmlStreamAttributes attr = reader->attributes();
            DebugRow dr;
            dr.timeStamp = attr.value(QString::fromUtf8("timestamp")).toString();
            dr.messageType = attr.value(QString::fromUtf8("type")).toString();
            dr.content = attr.value(QString::fromUtf8("content")).toString();
            appendDebugRow(&dr);
        }
    } while (!reader->error());
}

void MegaDebugServer::readDebugMsg()
{
    parseReader(reader);
}

void MegaDebugServer::appendDebugRow(DebugRow *dr)
{
    int index = debugDataModel->rowCount();

    if (index >= MAX_LOG_MESSAGES)
    {
        QList<QStandardItem*> items = debugDataModel->takeRow(0);
        for (int i = 0; i < items.size(); i++)
        {
            delete items[i];
        }
        index--;
    }

    QStandardItem *ts = new QStandardItem(dr->timeStamp);
    QStandardItem *mt = new QStandardItem(dr->messageType);
    QStandardItem *c = new QStandardItem(dr->content);

    debugDataModel->setItem(index, 0, ts);
    debugDataModel->setItem(index, 1, mt);
    debugDataModel->setItem(index, 2, c);

    ui->messagesTreeView->scrollToBottom();
}

void MegaDebugServer::startstop()
{
    if (!megaServer)
    {
        QLocalServer::removeServer(MEGA_LOGGER);
        megaServer = new QLocalServer();
        if (!megaServer->listen(MEGA_LOGGER))
        {
            ui->statusBar->showMessage("Error starting server");
            megaServer->deleteLater();
            megaServer = NULL;
            return;
        }

        connect(megaServer,SIGNAL(newConnection()),this,SLOT(clientConnected()));
        ui->actionSave->setEnabled(false);
        ui->actionLoad->setEnabled(false);
        ui->statusBar->showMessage("Ready");
        ui->actionStop->setText(tr("Stop"));

        timer.setSingleShot(false);
        timer.setInterval(1000);
        timer.start();
        tryConnect();
    }
    else
    {
        disconnected();
    }
}

void MegaDebugServer::disconnected()
{
    if (megaServer)
    {
        delete reader;
        megaServer->deleteLater();
        reader = NULL;
        megaServer = NULL;
        megaSyncClient = NULL;
        ui->actionSave->setEnabled(true);
        ui->actionLoad->setEnabled(true);
        ui->statusBar->showMessage(tr("Disconnected"));
        ui->actionStop->setText(tr("Start"));
        timer.stop();
    }
}

void MegaDebugServer::tryConnect()
{
    client.connectToServer(ENABLE_MEGASYNC_LOGS);
}

void MegaDebugServer::filterTextRegExp()
{
    qDebug() << ui->filterTypeComboBox->itemData(ui->filterTypeComboBox->currentIndex()).toInt();
    QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(ui->filterTypeComboBox->itemData(ui->filterTypeComboBox->currentIndex()).toInt());
    Qt::CaseSensitivity caseSensitivity = ui->caseSensitivecheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegExp regExp(ui->filterPatternLineEdit->text(), caseSensitivity, syntax);
    qDebug() << regExp;
    debugProxyModel->setFilterRegExp(regExp);
}

void MegaDebugServer::filterColumn()
{
    debugProxyModel->setFilterKeyColumn(ui->columnComboBox->currentIndex());
}

void MegaDebugServer::filterCaseSensitive()
{
    debugProxyModel->setSortCaseSensitivity(ui->caseSensitivecheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void MegaDebugServer::saveToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
             tr("Save Debug Log"), "",
             tr("Log File (*.dat);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Truncate))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QByteArray ba;
    QXmlStreamWriter xmlWriterLog(&ba);
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_8);
    qint32 n(debugDataModel->rowCount());

    /* Writes a document start with the XML version number. */
    xmlWriterLog.writeStartDocument();
    xmlWriterLog.writeStartElement("MEGA");
    for (int i = 0; i < n; i++)
    {
        xmlWriterLog.writeStartElement("log");
        //Add timestamp and value
        xmlWriterLog.writeAttribute("timestamp", debugDataModel->item(i,0)->text());
        //Add type and value
        xmlWriterLog.writeAttribute("type", debugDataModel->item(i,1)->text());
        //Add content and value
        xmlWriterLog.writeAttribute("content", debugDataModel->item(i,2)->text());
        xmlWriterLog.writeEndElement();
    }

    //Close tags
    xmlWriterLog.writeEndElement();
    xmlWriterLog.writeEndDocument();
    out << qCompress(ba);
    file.close();
}


void MegaDebugServer::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Log File"), "",
             tr("Log File (*.dat);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    clearDebugWindow();

    QDataStream in(&file);
    QByteArray ba;
    in >> ba;

    QXmlStreamReader xmlLoad(qUncompress(ba));
    parseReader(&xmlLoad);
    file.close();
}

void MegaDebugServer::clearDebugWindow()
{
    debugDataModel->removeRows(0,debugDataModel->rowCount());
}
MegaDebugServer::~MegaDebugServer()
{
    disconnected();
    delete debugDataModel;
    delete debugProxyModel;
    delete ui;
}
