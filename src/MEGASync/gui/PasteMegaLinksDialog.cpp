#include "PasteMegaLinksDialog.h"
#include "ui_PasteMegaLinksDialog.h"

#include <QClipboard>
#include <QUrl>
#include <QMessageBox>

#include<iostream>
using namespace std;

PasteMegaLinksDialog::PasteMegaLinksDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasteMegaLinksDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

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
        if (!text.trimmed().size())
        {
            QMessageBox::warning(this, tr("Warning"), tr("Enter one or more MEGA file links"));
        }
        else
        {
            QMessageBox::warning(this, tr("Warning"), tr("No valid MEGA links found. (Folder links aren't yet supported)"));
        }
        return;
    }

    accept();
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
    QStringList tempLinks = text.split(QString::fromAscii("https://mega.co.nz/#!"), QString::KeepEmptyParts, Qt::CaseInsensitive);
    tempLinks.removeAt(0);

    QStringList tempLinks2 = text.split(QString::fromAscii("mega://#!"), QString::KeepEmptyParts, Qt::CaseInsensitive);
    tempLinks2.removeAt(0);
    tempLinks.append(tempLinks2);

    QStringList tempLinksNewSite = text.split(QString::fromAscii("https://mega.nz/#!"), QString::KeepEmptyParts, Qt::CaseInsensitive);
    tempLinksNewSite.removeAt(0);
    tempLinks.append(tempLinksNewSite);

    QStringList tempLinksHttp = text.split(QString::fromAscii("http://mega.co.nz/#!"), QString::KeepEmptyParts, Qt::CaseInsensitive);
    tempLinksHttp.removeAt(0);
    tempLinks.append(tempLinksHttp);

    QStringList tempLinksNewSiteHttp = text.split(QString::fromAscii("http://mega.nz/#!"), QString::KeepEmptyParts, Qt::CaseInsensitive);
    tempLinksNewSiteHttp.removeAt(0);
    tempLinks.append(tempLinksNewSiteHttp);

    QStringList finalLinks;
    for (int i = 0; i < tempLinks.size(); i++)
    {
        QString link = checkLink(tempLinks[i].insert(0, QString::fromAscii("https://mega.nz/#!")));
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

    if (link.length() < 70)
    {
        return QString();
    }

    link.truncate(73);
    QByteArray data = link.toUtf8();
    if (data.constData()[26] == '!')
    {
        return link;
    }

    return QString();
}
