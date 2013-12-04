#include "PasteMegaLinksDialog.h"
#include "ui_PasteMegaLinksDialog.h"

#include <QClipboard>
#include <QUrl>

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
	if(extractLinks(text).size()!=0)
		ui->eLinks->setPlainText(text);
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
	if(links.size()==0)
	{
		return;
	}

	accept();
}

QStringList PasteMegaLinksDialog::extractLinks(QString text)
{
	QStringList tempLinks = text.split("https://mega.co.nz/#!", QString::KeepEmptyParts, Qt::CaseInsensitive);
	tempLinks.removeAt(0);
	QStringList tempLinks2 = text.split("mega://#!", QString::KeepEmptyParts, Qt::CaseInsensitive);
	tempLinks2.removeAt(0);
	tempLinks.append(tempLinks2);
	cout << "Link candidates: " << tempLinks.size() << endl;
	QStringList finalLinks;
	for(int i=0; i<tempLinks.size(); i++)
	{
		QString link = checkLink(tempLinks[i].insert(0, "https://mega.co.nz/#!"));
		if(!link.isNull()) finalLinks.append(link);
	}

	return finalLinks;
}

QString PasteMegaLinksDialog::checkLink(QString link)
{
	cout << "Checking: " << link.toStdString() << endl;
	link = QUrl::fromPercentEncoding(link.toUtf8());
	link.replace(' ', '+');

	if(link.length()<73)
	{
		cout << "Wrong lenght" << endl;
		return QString();
	}

	link.truncate(73);
	if(link.toUtf8().constData()[29]=='!')
		return link;

	cout << "NO KEY " << link.toUtf8().constData()[29] << endl;
	return QString();
}
