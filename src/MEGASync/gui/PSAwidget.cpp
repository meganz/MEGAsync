#include "PSAwidget.h"
#include "ui_PSAwidget.h"
#include <QDesktopServices>
#include <Utilities.h>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

PSAwidget::PSAwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSAwidget)
{
    ui->setupUi(this);

    this->idPSA = 0;
    this->reply = NULL;

    networkAccess = new QNetworkAccessManager(this);
    timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTestTimeout()));
    connect(networkAccess, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onRequestImgFinished(QNetworkReply*)));

    minHeightAnimation = new QPropertyAnimation();
    maxHeightAnimation = new QPropertyAnimation();
    animationGroup = new QParallelAnimationGroup();
    animationGroup->addAnimation(minHeightAnimation);
    animationGroup->addAnimation(maxHeightAnimation);
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onAnimationFinished()));

    ui->pPSA->hide();
    ui->sWidget->hide();
    ui->wImage->hide();
    ui->bMore->hide();
}

PSAwidget::~PSAwidget()
{
    delete ui;
    delete minHeightAnimation;
    delete maxHeightAnimation;
    delete animationGroup;
    delete networkAccess;
    delete timer;
}

void PSAwidget::setAnnounce(int id, QString title, QString desc, QString urlImage, QString textButton, QString urlClick)
{
    this->idPSA = id;
    this->title = title;
    this->desc = desc;
    this->urlImage = urlImage;
    this->textButton = textButton;
    this->urlClick = urlClick;

    if (Utilities::getDevicePixelRatio() >= 2)
    {
        QString imageName = QUrl(urlImage).fileName().split(QString::fromUtf8(".")).at(0);
        if (!imageName.contains(QRegExp(QString::fromUtf8("@2x$"))))
        {
            urlImage.replace(imageName, imageName + QString::fromUtf8("@2x"));
        }
    }

    testRequest.setUrl(QUrl(urlImage));
    testRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    timer->start(5000);
    reply = networkAccess->get(testRequest);
}

int PSAwidget::isPSAshown()
{
    return idPSA;
}

bool PSAwidget::showPSA(QImage image)
{
    QFont f = ui->lTitle->font();
    QFontMetrics fm = QFontMetrics(f);
    int width = ui->lTitle->width();
    ui->lTitle->setText(fm.elidedText(title, Qt::ElideRight, width));

    ui->lDesc->setFrameStyle(QFrame::Box);
    ui->lDesc->setText(desc);

    if (!textButton.isEmpty())
    {
        ui->bMore->setText(textButton);
        ui->bMore->show();
    }

    if (!(image.isNull()))
    {
        ui->bImage->setIcon(QPixmap::fromImage(image));
        ui->bImage->setIconSize(QSize(64, 64));
        ui->wImage->show();
    }   

    ui->pPSA->hide();
    ui->sWidget->show();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(0);
    maxHeightAnimation->setStartValue(0);
    minHeightAnimation->setEndValue(120);
    maxHeightAnimation->setEndValue(120);
    minHeightAnimation->setDuration(250);
    maxHeightAnimation->setDuration(250);
    animationGroup->start();

    return true;
}

void PSAwidget::removeAnnounce()
{
    this->idPSA = 0;
    this->title = QString();
    this->desc = QString();
    this->urlImage = QString();
    this->textButton = QString();
    this->urlClick = QString();

    ui->lTitle->setText(QString::fromUtf8(""));
    ui->lDesc->setText(QString::fromUtf8(""));
    ui->bMore->setText(QString::fromUtf8(""));
    ui->bMore->hide();
    ui->wImage->hide();
}

void PSAwidget::on_bMore_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(urlClick));
    on_bDismiss_clicked();
}

void PSAwidget::on_bDismiss_clicked()
{
    ui->pPSA->hide();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(120);
    maxHeightAnimation->setStartValue(120);
    minHeightAnimation->setEndValue(0);
    maxHeightAnimation->setEndValue(0);
    minHeightAnimation->setDuration(250);
    maxHeightAnimation->setDuration(250);
    animationGroup->start();   

    emit PSAseen(idPSA);
    emit PSAhidden();

    removeAnnounce();
}

void PSAwidget::onAnimationFinished()
{
    ui->pPSA->show();
    emit PSAshown();
}

void PSAwidget::onRequestImgFinished(QNetworkReply *reply)
{
    timer->stop();
    reply->deleteLater();
    this->reply = NULL;

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        showPSA();
        return;
    }

    QByteArray bytes = reply->readAll();
    if (bytes.isEmpty())
    {
        showPSA();
        return;
    }

    QImage img(64, 64, QImage::Format_ARGB32_Premultiplied);
    img.loadFromData(bytes);
    showPSA(img);
}

void PSAwidget::onTestTimeout()
{
    if (reply)
    {
        reply->abort();
        showPSA();
    }
}
