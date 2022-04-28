#include "AvatarWidget.h"
#include "control/Utilities.h"
#include "MegaApplication.h"

#include <math.h>

#include <QLinearGradient>
#include <QPainter>
#include <QWindow>
#include <QMouseEvent>

static const int AVATAR_DIAMETER (60);
static const int AVATAR_RADIUS (AVATAR_DIAMETER / 2);
static const int AVATAR_LETTER_SIZE_PT_FULL (85);
static const int LETTER_PIXMAP_SIZE (150);
static const int LATO_FONT_ADJUST_SHADOW (-6);
static const int LATO_FONT_ADJUST (-8);


AvatarWidget::AvatarWidget(QWidget* parent) :
    QWidget(parent),
    mLetter(QString::fromUtf8("")),
    mPathToFile(QString::fromUtf8("")),
    mGradient(-AVATAR_RADIUS, AVATAR_RADIUS, AVATAR_RADIUS, -AVATAR_RADIUS)
{
}

void AvatarWidget::setAvatarLetter(QChar letter, const QColor& color)
{
    mLetter.append(letter);
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

        auto preferences = Preferences::instance();
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
    mLetter.clear();
    mPathToFile.clear();
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

    if (mLetter.isEmpty() && mPathToFile.isNull())
    {
        return;
    }

    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto width (this->width());
    painter.translate(width / 2, height() / 2);
    QRect rect (-width / 2, -width / 2, width, width);

    if (QFileInfo::exists(mPathToFile))
    {
        //Apply avatar
        painter.drawPixmap(rect, AvatarPixmap::maskFromImagePath(mPathToFile, width));
    }
    else
    {
        painter.drawPixmap(rect, AvatarPixmap::createFromLetter(mLetter, mGradient, width));
    }
}

void AvatarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        ((MegaApplication *)qApp)->openSettings(SettingsDialog::ACCOUNT_TAB);
    }
}

QPixmap AvatarPixmap::maskFromImagePath(const QString &pathToFile, int size)
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

QPixmap AvatarPixmap::createFromLetter(const QString& letter, QLinearGradient gradient, int size)
{
    QPixmap pm(QSize(LETTER_PIXMAP_SIZE, LETTER_PIXMAP_SIZE));
    QRect rect = pm.rect();
    pm.fill(Qt::transparent);

    // Draw background

    QPainter painter(&pm);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing );

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gradient));
    painter.drawEllipse(rect);

    QFont font = painter.font();
    font.setPointSize(AVATAR_LETTER_SIZE_PT_FULL);
    font.setFamily(QString::fromUtf8("Lato Semibold"));
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(rect.adjusted(0, LATO_FONT_ADJUST_SHADOW, 2, 0), letter, QTextOption(Qt::AlignCenter));
    painter.setPen(Qt::white);
    painter.drawText(rect.adjusted(0, LATO_FONT_ADJUST, 0, 0), letter, QTextOption(Qt::AlignCenter));



    painter.end();
    // Convert the image to a pixmap and rescale it.  Take pixel ratio into
    // account to get a sharp image on retina displays:
    qreal pr = QWindow().devicePixelRatio();
    pm.setDevicePixelRatio(pr);
    size = qRound (pr * size);
    pm = pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return pm;
}

