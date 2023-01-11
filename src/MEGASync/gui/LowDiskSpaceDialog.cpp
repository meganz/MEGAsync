#include "LowDiskSpaceDialog.h"
#include "ui_LowDiskSpaceDialog.h"

#include "BlurredShadowEffect.h"
#include "Utilities.h"

#include <QFileIconProvider>

LowDiskSpaceDialog::LowDiskSpaceDialog(qint64 neededSize, qint64 freeSize,
                                       qint64 driveSize, const QString& driveName,
                                       QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LowDiskSpaceDialog)
{
    ui->setupUi(this);
    setupUiStyle();
    ui->lDriveIcon->setPixmap(getHardDrivePixmap());

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto message = tr("There is not enough space on %1. You need an additional %2 to download these files.");
    ui->lExplanation->setText(message.arg(driveName, toString(neededSize-freeSize)));

    ui->lDiskName->setText(driveName);
    ui->lFreeSpace->setText(tr("Free space: %1").arg(toString(freeSize)));
    ui->lTotalSize->setText(tr("Total size: %1").arg(toString(driveSize)));

    connect(ui->bTryAgain, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->bCancel, &QPushButton::clicked, this, &QDialog::reject);

    mHighDpiResize.init(this);
}

LowDiskSpaceDialog::~LowDiskSpaceDialog()
{
    delete ui;
}

QString LowDiskSpaceDialog::toString(qint64 bytes)
{
    return Utilities::getSizeString((bytes > 0) ? bytes : 0);
}

QPixmap LowDiskSpaceDialog::getHardDrivePixmap()
{
#ifdef _WIN32
    const char* iconResource = (Utilities::getDevicePixelRatio() < 2 ? ":/images/HD_win.png"
                                                                     : ":/images/HD_win@2x.png");
#else
    const char* iconResource = (Utilities::getDevicePixelRatio() < 2 ? ":/images/HD_mac.png"
                                                                     : ":/images/HD_mac@2x.png");
#endif
    return QPixmap(QString::fromLatin1(iconResource));
}

QString LowDiskSpaceDialog::getMacButtonStyle(const QString &backgroundColor, const QColor &referenceColor,
                                              const QColor &textColor)
{
    return getButtonStyle(backgroundColor, referenceColor, textColor, 14, 4, 6);
}

QString LowDiskSpaceDialog::getWinButtonStyle(const QColor& backgroundColor, const QColor &textColor)
{
    return getButtonStyle(backgroundColor.name(), backgroundColor, textColor, 20, 5, 4);
}

QString LowDiskSpaceDialog::getButtonStyle(const QString &backgroundColor, const QColor &referenceColor,
                                           const QColor &textColor, int horizontalPadding, int verticalPadding, int borderRadius)
{
    QString style = QString::fromLatin1("QPushButton { ");
    style += getStylePaddings(horizontalPadding, verticalPadding);
    style += getStyleBorders(borderRadius);
    style += QString::fromLatin1("background-color : %1;").arg(backgroundColor);
    style += QString::fromLatin1("color : %1;").arg(textColor.name());
    style += QString::fromLatin1("}");
    style += getStyleButtonStates(referenceColor);
    return style;
}

QString LowDiskSpaceDialog::getStylePaddings(int horizontal, int vertical)
{
    QString props = QString::fromLatin1("padding-top: %1px; padding-bottom: %1px; padding-left: %2px; padding-right: %2px;");
    return props.arg(vertical).arg(horizontal);
}

QString LowDiskSpaceDialog::getStyleBorders(int radius)
{
    QString props = QString::fromLatin1("border-style: solid; border-width: 1px; border-color: #d7d6d5; border-radius: %1px;");
    return props.arg(radius);
}

QString LowDiskSpaceDialog::getStyleButtonStates(const QColor& referenceColor)
{
    QString props;
    props += QString::fromLatin1("QPushButton:hover { background-color: %1; }").arg(referenceColor.lighter(120).name());
    props += QString::fromLatin1("QPushButton:pressed { background-color: %1; }").arg(referenceColor.darker(120).name());
    return props;
}

void LowDiskSpaceDialog::setupUiStyle()
{
#ifdef _WIN32
    const char* textStyle = "*{ font-family: \"Segoe UI\"; font-size: 14px; font-weight: 400; }";
#else
    const char* textStyle = "*{ font-family: \"SF Pro\"; font-size: 14px; font-weight: 400; }";
#endif
    setStyleSheet(QString::fromLatin1(textStyle));


    QColor cancelColor(252, 252, 252, 255);
#ifdef _WIN32
    ui->bTryAgain->setStyleSheet(getWinButtonStyle(QColor(0, 95, 184, 255), QColor("#FFFFFF")));
    ui->bCancel->setStyleSheet(getWinButtonStyle(cancelColor, QColor("#000000")));
#else
    const char* backgroundColor = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #4B91F7, stop:1 #367AF6)";
    ui->bTryAgain->setStyleSheet(getMacButtonStyle(QString::fromLatin1(backgroundColor),
                                                   QColor("#367AF6"),
                                                   QColor("#FFFFFF")));
    ui->bCancel->setStyleSheet(getMacButtonStyle(cancelColor.name(), cancelColor, QColor("#000000")));
    ui->bTryAgain->setGraphicsEffect(CreateBlurredShadowEffect(QColor(54, 122, 246, 64), 1.0));
    ui->bCancel->setGraphicsEffect(CreateBlurredShadowEffect(1.0));
#endif
}

