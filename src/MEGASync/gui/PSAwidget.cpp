#include "PSAwidget.h"
#include "ui_PSAwidget.h"
#include <QDesktopServices>
#include <Utilities.h>
#include <QTimer>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

PSAwidget::PSAwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSAwidget)
{
    ui->setupUi(this);

    this->reply = NULL;
    this->ready = false;
    this->shown = false;

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
    delete animationGroup;
    delete networkAccess;
    delete timer;
    delete ui;
}

void PSAwidget::setAnnounce(int id, QString title, QString desc, QString urlImage, QString textButton, QString urlClick)
{
    removeAnnounce();

    info.idPSA = id;
    info.title = title;
    info.desc = desc;
    info.urlImage = urlImage;
    info.textButton = textButton;
    info.urlClick = urlClick;

    if (Utilities::getDevicePixelRatio() >= 2)
    {
        QString imageName = QFileInfo(urlImage).fileName().split(QString::fromUtf8(".")).at(0);
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

bool PSAwidget::isPSAready()
{
    return ready;
}

bool PSAwidget::isPSAshown()
{
    return shown;
}

void PSAwidget::setPSAImage(QImage image)
{
    ui->lTitle->ensurePolished();
    int width = ui->lTitle->width();
    ui->lTitle->setText(ui->lTitle->fontMetrics().elidedText(info.title, Qt::ElideRight, width));

    ui->lDesc->setFrameStyle(QFrame::Box);
    ui->lDesc->setText(info.desc);

    if (!info.textButton.isEmpty())
    {
        ui->bMore->setText(info.textButton);
        ui->bMore->show();
    }

    if (!image.isNull())
    {
        ui->bImage->setIcon(QPixmap::fromImage(image));
        ui->bImage->setIconSize(QSize(64, 64));
        ui->wImage->show();
    }

    ready = true;
}

void PSAwidget::showPSA()
{
    if (shown || !ready)
    {
        return;
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
    shown = true;
}

PSA_info PSAwidget::getPSAdata()
{
    return info;
}

void PSAwidget::hidePSA(bool animated)
{
    if (!shown)
    {
        return;
    }

    shown = false;
    ui->pPSA->hide();
    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(120);
    maxHeightAnimation->setStartValue(120);
    minHeightAnimation->setEndValue(0);
    maxHeightAnimation->setEndValue(0);
    minHeightAnimation->setDuration(animated ? 250 : 1);
    maxHeightAnimation->setDuration(animated ? 250 : 1);
    animationGroup->start();
}

void PSAwidget::removeAnnounce()
{
    info.clear();

    if (reply)
    {
        reply->abort();
    }

    ui->bImage->setIcon(QIcon());
    ui->lTitle->setText(QString::fromUtf8(""));
    ui->lDesc->setText(QString::fromUtf8(""));
    ui->bMore->setText(QString::fromUtf8(""));
    ui->bMore->hide();
    ui->wImage->hide();
    ui->sWidget->hide();
    ui->pPSA->hide();

    if (shown)
    {
        hidePSA();
    }
    ready = false;
}

void PSAwidget::on_bMore_clicked()
{
    Utilities::openUrl(QUrl(info.urlClick));
    on_bDismiss_clicked();
}

void PSAwidget::on_bDismiss_clicked()
{
    hidePSA(true);
    emit PSAseen(info.idPSA);
    removeAnnounce();
}

void PSAwidget::onAnimationFinished()
{
    ui->pPSA->setVisible(shown);
}

void PSAwidget::onRequestImgFinished(QNetworkReply *reply)
{
    timer->stop();
    reply->deleteLater();
    this->reply = NULL;

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        setPSAImage();
        return;
    }

    QByteArray bytes = reply->readAll();
    if (bytes.isEmpty())
    {
        setPSAImage();
        return;
    }

    QImage img(64, 64, QImage::Format_ARGB32_Premultiplied);
    img.loadFromData(bytes);
    setPSAImage(img);
}

void PSAwidget::onTestTimeout()
{
    if (reply)
    {
        reply->abort();
        setPSAImage();
    }
}
