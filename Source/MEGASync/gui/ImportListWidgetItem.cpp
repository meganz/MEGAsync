#include "ImportListWidgetItem.h"
#include "ui_ImportListWidgetItem.h"
#include "utils/WindowsUtils.h"

#include <QFileInfo>

ImportListWidgetItem::ImportListWidgetItem(QString link, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ImportListWidgetItem)
{
	ui->setupUi(this);
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

void ImportListWidgetItem::setNode(Node *node)
{
	this->node = node;
}

void ImportListWidgetItem::setData(QString fileName, linkstatus status)
{
	this->fileName = fileName;
	this->status = status;
}

void ImportListWidgetItem::updateGui()
{
	ui->lName->setText(fileName);
	QFileInfo f(fileName);

	if(WindowsUtils::extensionIcons.contains(f.suffix().toLower()))
		ui->lImage->setPixmap(QPixmap(QString(WindowsUtils::extensionIcons[f.suffix().toLower()]).insert(0, "://images/small_")));
	else
		ui->lImage->setPixmap(QPixmap("://images/small_generic.png"));

	switch(status)
	{
	case LOADING:
		//ui->lState->setText("LOADING");
		break;
	case CORRECT:
		ui->lState->setPixmap(QPixmap("://images/import_ok_icon.png"));
		break;
	case WARNING:
		ui->lState->setPixmap(QPixmap("://images/import_warning_ico.png"));
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	default:
		ui->lState->setPixmap(QPixmap("://images/import_error_ico.png"));
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
