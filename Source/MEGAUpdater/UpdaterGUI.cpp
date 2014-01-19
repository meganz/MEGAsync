#include "UpdaterGUI.h"
#include "ui_UpdaterGUI.h"

#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>

#include "../MEGASync/sdk/include/mega/types.h"
#include "../MEGASync/sdk/include/mega/crypto/cryptopp.h"
#include "../MEGASync/sdk/include/mega.h"

using namespace mega;

const QString UpdaterGUI::BASE_UPDATE_URL = QString::fromAscii("http://g.static.mega.co.nz/upd/wsync/");
const QString UpdaterGUI::VERSION = QString::fromAscii("112");

UpdaterGUI::UpdaterGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UpdaterGUI)
{
    ui->setupUi(this);

    dstPaths.append(QString::fromUtf8("MEGAsync.exe"));
    dstPaths.append(QString::fromUtf8("ShellExtX32.dll"));
    dstPaths.append(QString::fromUtf8("ShellExtX64.dll"));
    dstPaths.append(QString::fromUtf8("QtCore4.dll"));
    dstPaths.append(QString::fromUtf8("QtGui4.dll"));
    dstPaths.append(QString::fromUtf8("QtNetwork4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qgif4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qico4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qjpeg4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qmng4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qsvg4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qtga4.dll"));
    dstPaths.append(QString::fromUtf8("imageformats/qtiff4.dll"));

    updateFiles.append(QString::fromUtf8("MEGAsync.exe"));
        updateFiles.append(QString::fromUtf8("ShellExtX32.dll"));
        updateFiles.append(QString::fromUtf8("ShellExtX64.dll"));
        updateFiles.append(QString::fromUtf8("QtCore4.dll"));
        updateFiles.append(QString::fromUtf8("QtGui4.dll"));
        updateFiles.append(QString::fromUtf8("QtNetwork4.dll"));
        updateFiles.append(QString::fromUtf8("qgif4.dll"));
        updateFiles.append(QString::fromUtf8("qico4.dll"));
        updateFiles.append(QString::fromUtf8("qjpeg4.dll"));
       updateFiles.append(QString::fromUtf8("qmng4.dll"));
        updateFiles.append(QString::fromUtf8("qsvg4.dll"));
        updateFiles.append(QString::fromUtf8("qtga4.dll"));
        updateFiles.append(QString::fromUtf8("qtiff4.dll"));


    if(verifySourcePath(QApplication::applicationDirPath()))
        ui->eUpdateFolder->setText(QDir::toNativeSeparators(QApplication::applicationDirPath()));

    QString defaultKey = QApplication::applicationDirPath().append(QString::fromUtf8("/update.key"));
    if(readKeys(defaultKey))
        ui->eUpdateKey->setText(QDir::toNativeSeparators(defaultKey));
}

UpdaterGUI::~UpdaterGUI()
{
    delete ui;
}

void UpdaterGUI::on_bChangeFolder_clicked()
{
    QString path =  QFileDialog::getExistingDirectory(this, tr("Select update folder"),
                                                      QApplication::applicationDirPath(),
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    if(path.length())
    {
        if(verifySourcePath(path))
            ui->eUpdateFolder->setText(QDir::toNativeSeparators(path));
        else
            QMessageBox::critical(this, tr("Error"), tr("App files not found"));
    }
}

void UpdaterGUI::on_bChangeKey_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select key file"),
                                 QApplication::applicationDirPath(), QString::fromAscii("Key files (*.key)"));
    if(path.length())
    {
        if(readKeys(path))
            ui->eUpdateKey->setText(QDir::toNativeSeparators(path));
        else
            QMessageBox::critical(this, tr("Error"), tr("Invalid key file"));
    }
}

void UpdaterGUI::on_bGenerateKey_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save key file"),
                                 QApplication::applicationDirPath(), QString::fromAscii("Key files (*.key)"));
    if(!path.length()) return;

    CryptoPP::Integer pubk[AsymmCipher::PUBKEY];
    string pubks;
    string privks;

    AsymmCipher asymkey;
    asymkey.genkeypair(asymkey.key,pubk,4096);
    AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
    AsymmCipher::serializeintarray(asymkey.key,AsymmCipher::PRIVKEY,&privks);

    int len = pubks.size();
    char* pubkstr = new char[len*4/3+4];
    Base64::btoa((const byte *)pubks.data(),len,pubkstr);

    len = privks.size();
    char *privkstr = new char[len*4/3+4];
    Base64::btoa((const byte *)privks.data(),len,privkstr);

    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream out(&file);

    out << pubkstr << endl;
    out << privkstr << endl;

    file.close();
    delete[] pubkstr;
    delete[] privkstr;

    QMessageBox::information(this, tr("Success"), tr("A new key file has been generated and saved"));
    ui->eUpdateKey->setText(QDir::toNativeSeparators(path));
}

void UpdaterGUI::on_bGenerateUpdate_clicked()
{
    if(!ui->eUpdateFolder->text().size())
    {
        QMessageBox::critical(this, tr("Error"), tr("Select the source folder first"));
        return;
    }

    if(!ui->eUpdateKey->text().size())
    {
        QMessageBox::critical(this, tr("Error"), tr("Select key file first"));
        return;
    }

    QString path = ui->eUpdateFolder->text();
    if(!path.length()) return;
    path.append(QDir::separator()).append(tr("v.txt"));


    QStringList SIGNATURES;
    QString UPDATE_FILE_SIGNATURE;

    char signature[512];
    HashSignature updateFileSignatureGenerator(new Hash());
    HashSignature signatureGenerator(new Hash());
    int len = strlen(PRIVATE_KEY)/4*3+3;
    AsymmCipher privkey;
    string privks;
    privks.resize(len);
    privks.resize(Base64::atob(PRIVATE_KEY, (byte *)privks.data(), len));
    privkey.setkey(AsymmCipher::PRIVKEY,(byte*)privks.data(), privks.size());

    QString sourceFolder = ui->eUpdateFolder->text();
    if(!sourceFolder.endsWith(QDir::separator()))
        sourceFolder.append(QDir::separator());

    updateFileSignatureGenerator.add((const byte *)VERSION.toAscii().constData(), VERSION.toAscii().size());
    for(int i=0; i<finalSourcePaths.size(); i++)
    {
        signatureGenerator.get(&privkey, NULL, 0);
        QString fileName = sourceFolder + finalSourcePaths[i];
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) return;
        QByteArray blob = file.readAll();
        signatureGenerator.add((const byte *)blob.constData(), blob.length());

        signatureGenerator.get(&privkey, (byte *)signature, sizeof(signature));
        int l = (sizeof(signature)*4)/3+4;
        char *buf = new char[l];
        Base64::btoa((const byte *)signature,sizeof(signature),buf);
        SIGNATURES.append(QString::fromAscii(buf));
        delete buf;

        QString downloadURL = BASE_UPDATE_URL + QFileInfo(dstPaths[i]).fileName();
        updateFileSignatureGenerator.add((const byte *)downloadURL.toAscii().constData(), downloadURL.toAscii().size());
        QString targetPath = dstPaths[i];
        updateFileSignatureGenerator.add((const byte *)targetPath.toAscii().constData(), targetPath.toAscii().size());
        updateFileSignatureGenerator.add((const byte *)SIGNATURES[i].toAscii().constData(), SIGNATURES[i].toAscii().size());
    }

    updateFileSignatureGenerator.get(&privkey, (byte *)signature, sizeof(signature));
    int l = (sizeof(signature)*4)/3+4;
    char *buf = new char[l];
    Base64::btoa((const byte *)signature,sizeof(signature),buf);
    UPDATE_FILE_SIGNATURE = QString::fromAscii(buf);
    delete buf;


    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream out(&file);
    out << VERSION << endl;
    out << UPDATE_FILE_SIGNATURE << endl;
    for(int i=0; i<SIGNATURES.size(); i++)
    {
        out << BASE_UPDATE_URL << QFileInfo(dstPaths[i]).fileName() << endl;
        out << dstPaths[i] << endl;
        out << SIGNATURES[i] << endl;
    }

    file.close();
}

void UpdaterGUI::on_bVerifyUpdate_clicked()
{

}

bool UpdaterGUI::readKeys(QString path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in( &file );
        in >> PUBLIC_KEY;
        in >> PRIVATE_KEY;

        cout << "PUBK: " << PUBLIC_KEY << endl;
        cout << "PRIVK: " << PRIVATE_KEY << endl;

        file.close();
        return true;
    }
    return false;
}

bool UpdaterGUI::verifySourcePath(QString path)
{
    path = QDir::toNativeSeparators(path);
    if(!path.endsWith(QDir::separator()))
        path.append(QDir::separator());

    cout << "Testing: " << path.toStdString() << endl;
    for(int i=0; i<updateFiles.size(); i++)
    {
        QString fullPath = path+updateFiles[i];
        if(!QFileInfo(fullPath).exists())
        {
            cout << "File not found: " << fullPath.toStdString() << endl;
            break;
        }

        if(i==(updateFiles.size()-1))
        {
            finalSourcePaths = updateFiles;
            return true;
        }
    }

    for(int i=0; i<dstPaths.size(); i++)
    {
        QString fullPath = path+dstPaths[i];
        if(!QFileInfo(fullPath).exists())
        {
            cout << "File not found: " << fullPath.toStdString() << endl;
            break;
        }

        if(i==(dstPaths.size()-1))
        {
            finalSourcePaths = dstPaths;
            return true;
        }
    }

    return false;
}
