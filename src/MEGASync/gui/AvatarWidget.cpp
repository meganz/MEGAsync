#include "AvatarWidget.h"
#include "control/Utilities.h"
#include "MegaApplication.h"

#include <math.h>

#include <QLinearGradient>
#include <QPainter>
#include <QWindow>
#include <QMouseEvent>

static constexpr int AVATAR_DIAMETER (36);
static constexpr int AVATAR_RADIUS (AVATAR_DIAMETER / 2);

AvatarWidget::AvatarWidget(QWidget* parent) :
    QWidget(parent),
    mGradient(-AVATAR_RADIUS, AVATAR_RADIUS, AVATAR_RADIUS, -AVATAR_RADIUS),
    mLetter(),
    mLetterShadow(new QGraphicsDropShadowEffect(&mLetter))
{
    mLetterShadow->setBlurRadius(12.0);
    mLetterShadow->setOffset(0., 1.);
    mLetterShadow->setEnabled(true);
    mLetter.setGraphicsEffect(mLetterShadow);
    mLetter.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mLetter.setStyleSheet(QString::fromUtf8("QLabel {"
                                             "font-family: Lato Semibold;"
                                             "background: transparent;"
                                             "color: white;"
                                             "border: none;"
                                             "margin: 0px;"
                                             "padding: 0px;}"));
    clearData();
}

void AvatarWidget::setAvatarLetter(QChar letter, const QColor& color)
{
    mLetter.setText(letter);
    mLetterShadow->setColor(color.darker(145));
    mGradient.setColorAt(1.0, color.lighter(130));
    mGradient.setColorAt(0.0, color);
    update();
}

void AvatarWidget::setAvatarImage(const QString& pathToFile)
{
    mPathToFile = pathToFile;
    update();
}

void AvatarWidget::drawAvatarFromEmail(const QString& email)
{
    mega::MegaApi *megaApi = ((MegaApplication *)qApp)->getMegaApi();
    if (!megaApi)
    {
        return;
    }

    QString avatarsPath = Utilities::getAvatarPath(email);
    QFileInfo avatar(avatarsPath);
    if (avatar.exists())
    {
        setAvatarImage(Utilities::getAvatarPath(email));
    }
    else
    {
        QColor color (217, 0, 7);
        const char* userHandle = megaApi->getMyUserHandle();
        if (userHandle)
        {
            const char* avatarColor = megaApi->getUserAvatarColor(userHandle);
            if (avatarColor)
            {
                color = QColor(avatarColor);
                delete [] avatarColor;
            }
            delete [] userHandle;
        }

        Preferences* preferences = Preferences::instance();
        QString fullname = (preferences->firstName() + preferences->lastName()).trimmed();
        if (fullname.isEmpty())
        {
            char* apiEmail = megaApi->getMyEmail();
            if (apiEmail)
            {
                fullname = QString::fromUtf8(apiEmail);
                delete [] apiEmail;
            }
            else
            {
                fullname = preferences->email();
            }

            if (fullname.isEmpty())
            {
                fullname = QString::fromUtf8(" ");
            }
        }

        setAvatarLetter(fullname.at(0).toUpper(), color);
    }
}

void AvatarWidget::clearData()
{
    mLetter.setText(QString());
    mPathToFile = QString();
}

QSize AvatarWidget::minimumSizeHint() const
{
    return QSize(AVATAR_DIAMETER, AVATAR_DIAMETER);
}

QSize AvatarWidget::sizeHint() const
{
    return QSize(AVATAR_DIAMETER, AVATAR_DIAMETER);
}

void AvatarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (mLetter.text().isNull() && mPathToFile.isNull())
    {
        return;
    }

    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto width (this->width());
    qreal factor = width / AVATAR_DIAMETER;
    painter.translate(width / 2, height() / 2);
    QRect rect (-width / 2, -width / 2, width, width);

    if (QFileInfo::exists(mPathToFile))
    {
        //Apply avatar
        painter.drawPixmap(rect, mask_image(mPathToFile, width));
    }
    else
    {
        // Draw background
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(mGradient));
        painter.drawEllipse(rect);

        // Draw letter
        QFont font (mLetter.font());
        mGradient.setStart(-width / 2.0, width / 2.0);
        mGradient.setFinalStop(width / 2.0, -width / 2.0);
        font.setPixelSize(14 * qRound(factor));
        mLetter.setFont(font);
        mLetter.resize(width, width);
        painter.drawPixmap(rect, mLetter.grab());
    }
}

void AvatarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        ((MegaApplication *)qApp)->openSettings(SettingsDialog::ACCOUNT_TAB);
    }
}

QPixmap AvatarWidget::mask_image(const QString& pathToFile, int size)
{
    // Return a QPixmap from image loaded from pathToFile masked with a smooth circle.
    // The returned image will have a size of size × size pixels.
    // Load image and convert to 32-bit ARGB (adds an alpha channel):
    // Snipped based on Stefan scherfke code

    if (!QFileInfo::exists(pathToFile))
    {
        return QPixmap();
    }

    QPixmap pm;
    QImage image(pathToFile);
    image = image.convertToFormat(QImage::Format_ARGB32);

    // Crop image to a square:
    int imgsize = qMin(image.width(), image.height());
    QRect rect = QRect((image.width() - imgsize) / 2,
                       (image.height() - imgsize) / 2,
                       imgsize,
                       imgsize);
    image = image.copy(rect);

    // Create the output image with the same dimensions and an alpha channel
    // and make it completely transparent:
    QImage out_img = QImage(imgsize, imgsize, QImage::Format_ARGB32);
    out_img.fill(Qt::transparent);

    // Create a texture brush and paint a circle with the original image onto
    // the output image:
    QBrush brush = QBrush(image);                       // Create texture brush
    QPainter painter(&out_img);                         // Paint the output image
    painter.setPen(Qt::NoPen);                          // Don't draw an outline
    painter.setRenderHint(QPainter::Antialiasing, true);// Use AA

    painter.setBrush(brush);                            // Use the image texture brush
    painter.drawEllipse(0, 0, imgsize, imgsize);        // Actually draw the circle

    painter.end();                                      // We are done (segfault if you forget this)

    // Convert the image to a pixmap and rescale it.  Take pixel ratio into
    // account to get a sharp image on retina displays:
    qreal pr = QWindow().devicePixelRatio();
    pm = QPixmap::fromImage(out_img);
    pm.setDevicePixelRatio(pr);
    size = qRound (pr * size);
    pm = pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return pm;
}
