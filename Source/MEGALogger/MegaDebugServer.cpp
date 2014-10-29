#include "MegaDebugServer.h"
#include "ui_MegaDebugServer.h"
#include <iostream>

using namespace std;

MegaDebugServer::MegaDebugServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MegaDebugServer), megaServer(0)
{
    //Setup and set up GUI
    ui->setupUi(this);

    ui->statusBar->showMessage("Ready");

    ui->filterTypeComboBox->addItem("Regular Expression", QRegExp::RegExp);
    ui->filterTypeComboBox->addItem("Wildcard", QRegExp::Wildcard);
    ui->filterTypeComboBox->addItem("Fixed string", QRegExp::FixedString);

    ui->columnComboBox->addItem("Timestamp");
    ui->columnComboBox->addItem("Message Type");
    ui->columnComboBox->addItem("Message");

    connect(ui->filterPatternLineEdit,SIGNAL(textChanged(QString)),this,SLOT(filterTextRegExp()));
    connect(ui->filterTypeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(filterTextRegExp()));
    connect(ui->columnComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(filterColumn()));
    connect(ui->caseSensitivecheckBox,SIGNAL(toggled(bool)),this, SLOT(filterCaseSensitive()));

    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveToFile()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(loadFromFile()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(clearDebugWindow()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(disconnected()));

    connect(this, SIGNAL(insertDebugRow(DebugRow *)),this, SLOT(appendDebugRow(DebugRow *)));

    debugDataModel = new QStandardItemModel(0,3,this);
    debugDataModel->setHeaderData(0,Qt::Horizontal,QString::fromUtf8("Timestamp"),Qt::DisplayRole);
    debugDataModel->setHeaderData(1,Qt::Horizontal,QString::fromUtf8("Message Type"),Qt::DisplayRole);
    debugDataModel->setHeaderData(2,Qt::Horizontal,QString::fromUtf8("Message"),Qt::DisplayRole);

    debugProxyModel = new QSortFilterProxyModel;
    debugProxyModel->setSourceModel(debugDataModel);
    ui->messagesTreeView->setModel(debugProxyModel);

    ui->messagesTreeView->sortByColumn(1,Qt::AscendingOrder);
    ui->messagesTreeView->resizeColumnToContents(0);
    ui->messagesTreeView->resizeColumnToContents(1);
    ui->messagesTreeView->resizeColumnToContents(2);

    setWindowTitle(tr("MEGAsync Debug Window"));

    megaServer = new QLocalServer(this);
    if (!megaServer->listen("MEGA_SERVER")) {
             qDebug() << "Error";
             close();
             return;
         }
    connect(megaServer,SIGNAL(newConnection()),this,SLOT(clientConnected()));

}

void MegaDebugServer::clientConnected()
{

    ui->actionSave->setEnabled(false);
    ui->actionLoad->setEnabled(false);
    ui->actionClear->setEnabled(false);

    ui->statusBar->showMessage(tr("Connected"));

    megaSyncClient = megaServer->nextPendingConnection();
    reader = new QXmlStreamReader(megaSyncClient);

    connect(megaSyncClient, SIGNAL(readyRead()), this, SLOT(readDebugMsg()));
    connect(megaSyncClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(megaSyncClient, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(disconnected()));

}

void MegaDebugServer::parseReader(QXmlStreamReader *reader)
{
    DebugRow dr;

    do
    {
        QXmlStreamReader::TokenType token = reader->readNext();

        switch(token)
        {

        case QXmlStreamReader::StartElement:
        {
            QStringRef tag = reader->name();
            if(tag == "timestamp")
            {
                dr.timeStamp = reader->readElementText();

            }else if(tag == "type")
            {
                dr.messageType = reader->readElementText();

            }else if(tag == "content")
            {
                dr.content = reader->readElementText();

            }else
            {
                continue;
            }

        }
        break;
        case QXmlStreamReader::EndElement:
        {
            QStringRef tag = reader->name();

            if(tag == "msg")
            {
                emit insertDebugRow(&dr);
            }
        }
        default:
            break;

        }

    }while(!reader->error());

}
void MegaDebugServer::readDebugMsg()
{
   parseReader(reader);
}

void MegaDebugServer::appendDebugRow(DebugRow *dr)
{
    int index = debugDataModel->rowCount();
    debugDataModel->insertRow(index);
    debugDataModel->setData(debugDataModel->index(index, 0), dr->timeStamp);
    debugDataModel->setData(debugDataModel->index(index, 1), dr->messageType);
    debugDataModel->setData(debugDataModel->index(index, 2), dr->content);

    ui->messagesTreeView->scrollToBottom();
}

void MegaDebugServer::disconnected()
{
    ui->statusBar->showMessage(tr("Disconnected"));

    megaServer->close();
    ui->actionSave->setEnabled(true);
    ui->actionLoad->setEnabled(true);
    ui->actionClear->setEnabled(true);

    if (!megaServer->listen("MEGA_SERVER"))
    {
         qDebug() << "Error";
         close();
         return;
    }
    ui->statusBar->showMessage("Ready");
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
             return;
         else {

             QFile file(fileName);
             if (!file.open(QIODevice::WriteOnly | QFile::Truncate)) {
                 QMessageBox::information(this, tr("Unable to open file"),
                     file.errorString());
                 return;
             }
             QByteArray ba;
             QXmlStreamWriter xmlWriterLog(&ba);
             QDataStream out(&file);
             out.setVersion(QDataStream::Qt_4_8);

             qint32 n(debugDataModel->rowCount());

             /* Writes a document start with the XML version number. */
             xmlWriterLog.writeStartDocument();
             xmlWriterLog.writeStartElement("msgs");

             for(int i=n-1;i>=0;i--)
             {
                 xmlWriterLog.writeStartElement("msg");
                 //Add timestamp and value
                 xmlWriterLog.writeTextElement("timestamp",debugDataModel->item(i,0)->text());
                 //Add type and value
                 xmlWriterLog.writeTextElement("type",debugDataModel->item(i,1)->text());
                 //Add content and value
                 xmlWriterLog.writeTextElement("content",debugDataModel->item(i,2)->text());
                 xmlWriterLog.writeEndElement();

             }
             //Close tags
             xmlWriterLog.writeEndElement();
             xmlWriterLog.writeEndDocument();

             out << qCompress(ba);

            file.close();
         }
}


void MegaDebugServer::loadFromFile()
{

    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Log File"), "",
             tr("Log File (*.dat);;All Files (*)"));

    if (fileName.isEmpty())
             return;
         else {
             QFile file(fileName);
             if (!file.open(QIODevice::ReadOnly)) {
                 QMessageBox::information(this, tr("Unable to open file"),
                     file.errorString());
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
}

void MegaDebugServer::clearDebugWindow()
{
    debugDataModel->removeRows(0,debugDataModel->rowCount());
}
MegaDebugServer::~MegaDebugServer()
{
    megaServer->close();
    delete megaSyncClient;
    delete megaServer;
    delete debugDataModel;
    delete debugProxyModel;
    delete reader;
    delete ui;
}
