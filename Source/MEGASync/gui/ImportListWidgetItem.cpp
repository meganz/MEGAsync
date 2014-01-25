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
	node = NULL;
	this->link = link;
	status = LOADING;
	fileName = link;
	updateGui();
}

ImportListWidgetItem::~ImportListWidgetItem()
{
	delete ui;
}

void ImportListWidgetItem::setNode(MegaNode *node)
{
	this->node = node;
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
    ui->lImage->setPixmap(Utilities::getExtensionPixmapSmall(fileName));

	switch(status)
	{
	case LOADING:
		//ui->lState->setText("LOADING");
		break;
	case CORRECT:
        ui->lState->setPixmap(QPixmap(QString::fromAscii("://images/import_ok_icon.png")));
		break;
	case WARNING:
        ui->lState->setPixmap(QPixmap(QString::fromAscii("://images/import_warning_ico.png")));
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	default:
        ui->lState->setPixmap(QPixmap(QString::fromAscii("://images/import_error_ico.png")));
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	}
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
