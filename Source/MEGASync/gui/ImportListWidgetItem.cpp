#include "ImportListWidgetItem.h"
#include "ui_ImportListWidgetItem.h"
#include "control/Utilities.h"

#include <QFileInfo>

ImportListWidgetItem::ImportListWidgetItem(QString link, int id, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ImportListWidgetItem)
{
	ui->setupUi(this);
	this->id = id;
	this->link = link;
    this->fileSize = 0;
	status = LOADING;
	fileName = link;
	updateGui();
}

ImportListWidgetItem::~ImportListWidgetItem()
{
	delete ui;
}

void ImportListWidgetItem::setData(QString fileName, linkstatus status, long long size)
{
	this->fileName = fileName;
	this->status = status;
	this->fileSize = size;
}

void ImportListWidgetItem::updateGui()
{
    QString name;
    if(fileSize) name = fileName + QString::fromAscii(" (") + Utilities::getSizeString(fileSize) + QString::fromAscii(")");
    else name = fileName;

    QFont f = ui->lName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lName->setText(fm.elidedText(name, Qt::ElideMiddle,ui->lName->width()));

    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapSmall(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lImage->setIcon(icon);
    ui->lImage->setIconSize(QSize(24, 24));

    QIcon icon2;
	switch(status)
	{
	case LOADING:
		//ui->lState->setText("LOADING");
		break;
	case CORRECT:
        icon2.addFile(QStringLiteral(":/images/import_ok_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
		break;
	case WARNING:
        icon2.addFile(QStringLiteral(":/images/import_warning_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	default:
        icon2.addFile(QStringLiteral(":/images/import_error_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	}
    ui->lState->setIcon(icon2);
    ui->lState->setIconSize(QSize(24, 24));
}

bool ImportListWidgetItem::isSelected()
{
	return ui->cSelected->isChecked();
}

QString ImportListWidgetItem::getLink()
{
	return link;
}

void ImportListWidgetItem::on_cSelected_stateChanged(int state)
{
	emit stateChanged(id, state);
}
