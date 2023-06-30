#include "PasteMegaLinksDialog.h"
#include "ui_PasteMegaLinksDialog.h"

#include <QClipboard>
#include <QUrl>
#include "QMegaMessageBox.h"

#include<iostream>
using namespace std;

PasteMegaLinksDialog::PasteMegaLinksDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasteMegaLinksDialog)
{
    ui->setupUi(this);

    const QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if (extractLinks(text).size() != 0)
    {
        ui->eLinks->setPlainText(text);
    }

    ui->bSubmit->setDefault(true);
}

PasteMegaLinksDialog::~PasteMegaLinksDialog()
{
    delete ui;
}

QStringList PasteMegaLinksDialog::getLinks()
{
    return links;
}

void PasteMegaLinksDialog::on_bSubmit_clicked()
{
    QString text = ui->eLinks->toPlainText();
    links = extractLinks(text);
    links = links.toSet().toList();
    if (links.size() == 0)
    {
        QMegaMessageBox::MessageBoxInfo info;
        info.parent = this;
        info.title = QMegaMessageBox::warningTitle();

        if (!text.trimmed().size())
        {
            info.text = tr("Enter one or more MEGA file links");
        }
        else
        {
            info.text =  tr("Invalid MEGA Link");
        }

        QMegaMessageBox::warning(info);
    }
    else
    {
        accept();
    }
}

void PasteMegaLinksDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QDialog::changeEvent(event);
}

QStringList PasteMegaLinksDialog::extractLinks(QString text)
{
    QStringList finalLinks;
    QString separator;
    separator.append(QString::fromAscii("mega://").append(QString::fromUtf8("|")));
    separator.append(QString::fromAscii("https://mega.co.nz/").append(QString::fromUtf8("|")));
    separator.append(QString::fromAscii("https://mega.nz/").append(QString::fromUtf8("|")));
    separator.append(QString::fromAscii("http://mega.co.nz/").append(QString::fromUtf8("|")));
    separator.append(QString::fromAscii("http//mega.nz/"));

    QStringList tempLinks = text.split(QRegExp(separator));
    tempLinks.removeAt(0);

    for (int i = 0; i < tempLinks.size(); i++)
    {
        QString link = checkLink(tempLinks[i]);
        if (!link.isNull())
        {
            finalLinks.append(link);
        }
    }

    return finalLinks;
}

QString PasteMegaLinksDialog::checkLink(QString link)
{
    link = QUrl::fromPercentEncoding(link.toUtf8());
    link.replace(QChar::fromAscii(' '), QChar::fromAscii('+'));

    QString urlLink = QString::fromUtf8("https://mega.nz/");

    if (rxHeaderFolderSubfolder.indexIn(link) != -1)
    {
        link.truncate(FOLDER_LINK_WITH_SUBFOLDER_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFolderSubfolderNew.indexIn(link) != -1)
    {
        link.truncate(NEW_FOLDER_LINK_WITH_SUBFOLDER_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFolderFile.indexIn(link) != -1)
    {
        link.truncate(FOLDER_LINK_WITH_FILE_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFolderFileNew.indexIn(link) != -1)
    {
        link.truncate(NEW_FOLDER_LINK_WITH_FILE_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFile.indexIn(link) != -1)
    {
        link.truncate(FILE_LINK_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFileNew.indexIn(link) != -1)
    {
        link.truncate(NEW_FILE_LINK_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFolder.indexIn(link) != -1)
    {
        link.truncate(FOLDER_LINK_SIZE);
        return urlLink.append(link);
    }
    else if (rxHeaderFolderNew.indexIn(link) != -1)
    {
        link.truncate(NEW_FOLDER_LINK_SIZE);
        return urlLink.append(link);
    }


    return QString();
}
