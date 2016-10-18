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
    this->isFolder = false;
    status = LOADING;
    fileName = tr("Please wait...");
    ui->cSelected->setChecked(false);
    ui->cSelected->setEnabled(false);
    updateGui();
}

ImportListWidgetItem::~ImportListWidgetItem()
{
    delete ui;
}

void ImportListWidgetItem::setData(QString fileName, linkstatus status, long long size, bool isFolder)
{
    this->fileName = fileName;
    this->status = status;
    this->fileSize = size;
    this->isFolder = isFolder;
}

void ImportListWidgetItem::updateGui()
{
    QString name;
    if (fileSize)
    {
        name = fileName + QString::fromAscii(" (") + Utilities::getSizeString(fileSize) + QString::fromAscii(")");
    }
    else
    {
        name = fileName;
    }

    QFont f = ui->lName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lName->setText(fm.elidedText(name, Qt::ElideMiddle,ui->lName->width()));

    QIcon typeIcon;
    typeIcon.addFile(Utilities::getExtensionPixmapSmall(isFolder ? fileName.append(QString::fromUtf8(".folder")): fileName), QSize(), QIcon::Normal, QIcon::Off);

#ifdef __APPLE__
    ui->lImage->setIcon(typeIcon);
    ui->lImage->setIconSize(QSize(24, 24));
#else
    ui->lImage->setPixmap(typeIcon.pixmap(QSize(24, 24)));
#endif

    QIcon statusIcon;
    switch(status)
    {
    case LOADING:
        break;
    case CORRECT:
        statusIcon.addFile(QString::fromUtf8(":/images/import_ok_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->cSelected->setChecked(true);
        ui->cSelected->setEnabled(true);
        break;
    case WARNING:
        statusIcon.addFile(QString::fromUtf8(":/images/import_warning_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->cSelected->setChecked(false);
        ui->cSelected->setEnabled(false);
        break;
    default:
        statusIcon.addFile(QString::fromUtf8(":/images/import_error_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->cSelected->setChecked(false);
        ui->cSelected->setEnabled(false);
        break;
    }

#ifdef __APPLE__
    ui->lState->setIcon(statusIcon);
    ui->lState->setIconSize(QSize(24, 24));
#else
    ui->lState->setPixmap(statusIcon.pixmap(QSize(24, 24)));
#endif
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
