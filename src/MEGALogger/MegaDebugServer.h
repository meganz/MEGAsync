#ifndef MEGADEBUGSERVER_H
#define MEGADEBUGSERVER_H

#include <QMainWindow>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QXmlStreamReader>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

struct DebugRow
{
    QString timeStamp;
    QString messageType;
    QString content;

};

namespace Ui {
class MegaDebugServer;
}

class MegaDebugServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit MegaDebugServer(QWidget *parent = 0);

    ~MegaDebugServer();

private:
    Ui::MegaDebugServer *ui;
    QLocalServer *megaServer;
    QLocalSocket *megaSyncClient;
    QXmlStreamReader *reader;
    QLocalSocket client;

    QSortFilterProxyModel *debugProxyModel;
    QStandardItemModel *debugDataModel;
    QTimer timer;

private slots:
    void clientConnected();
    void readDebugMsg();
    void startstop();
    void disconnected();
    void tryConnect();

    void filterTextRegExp();
    void filterColumn();
    void filterCaseSensitive();

    void appendDebugRow(DebugRow *);

    void saveToFile();
    void loadFromFile();
    void clearDebugWindow();

public:
    void parseReader(QXmlStreamReader *);

};

#endif // MEGADEBUGSERVER_H
