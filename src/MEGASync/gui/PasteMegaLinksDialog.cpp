#include "PasteMegaLinksDialog.h"

#include "MessageDialogOpener.h"
#include "ServiceUrls.h"
#include "ui_PasteMegaLinksDialog.h"

#include <QClipboard>
#include <QRegularExpression>
#include <QUrl>

using namespace std;

PasteMegaLinksDialog::PasteMegaLinksDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PasteMegaLinksDialog)
{
    ui->setupUi(this);

    const auto text = QApplication::clipboard()->text();
    if (!extractLinks(text).isEmpty())
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
    links = QSet<QString>(links.begin(), links.end()).values();
    if (links.size() == 0)
    {
        MessageDialogInfo info;
        info.parent = this;

        if (!text.trimmed().size())
        {
            info.descriptionText = tr("Enter one or more MEGA file links");
        }
        else
        {
            info.descriptionText = tr("Invalid MEGA Link");
        }

        MessageDialogOpener::warning(info);
    }
    else
    {
        accept();
    }
}

bool PasteMegaLinksDialog::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    return QDialog::event(event);
}

QStringList PasteMegaLinksDialog::extractLinks(const QString& text)
{
    const auto supportedUrls = ServiceUrls::instance()->getSupportedLinksUrls();
    const auto splitRules = QRegularExpression(supportedUrls.join(QLatin1Char('|')));
    const auto tempLinks = text.split(splitRules, Qt::SkipEmptyParts);

    QStringList finalLinks;
    if (!tempLinks.isEmpty() && text != tempLinks.first())
    {
        foreach(auto tempLink, tempLinks)
        {
            auto link = checkLink(tempLink);
            if (!link.isEmpty())
            {
                finalLinks.append(link);
            }
        }
    }

    return finalLinks;
}

QString PasteMegaLinksDialog::checkLink(QString link)
{
    link = QUrl::fromPercentEncoding(link.toUtf8());
    link.replace(QLatin1Char(' '), QLatin1Char('+'));

    auto urlLink = ServiceUrls::instance()->getLinkBaseUrl().toString() + QLatin1Char('/');

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
    else if (rxHeaderCollectionNew.indexIn(link) != -1)
    {
        link.truncate(NEW_COLLECTION_LINK_SIZE);
        return urlLink.append(link);
    }

    return {};
}
