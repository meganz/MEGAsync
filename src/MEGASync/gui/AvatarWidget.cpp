#include "AvatarWidget.h"
#include "control/Utilities.h"
#include <QPainter>
#include <QWindow>
#include <QMouseEvent>
#include <math.h>
#include "MegaApplication.h"

AvatarWidget::AvatarWidget(QWidget *parent) :
    QWidget(parent)
{
    lastloadedwidth = 0;
    clearData();
}

void AvatarWidget::setAvatarLetter(QChar letter, QString color)
{
    this->letter = letter;
    this->color = color;
    update();
}

void AvatarWidget::setAvatarImage(QString pathToFile)
{
    this->pathToFile = pathToFile;
    update();
}

void AvatarWidget::drawAvatarFromEmail(QString email)
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
        QString color;
        const char* userHandle = megaApi->getMyUserHandle();
        const char* avatarColor = megaApi->getUserAvatarColor(userHandle);
        if (avatarColor)
        {
            color = QString::fromUtf8(avatarColor);
            delete [] avatarColor;
        }

        Preferences *preferences = Preferences::instance();
        QString fullname = (preferences->firstName() + preferences->lastName()).trimmed();
        if (fullname.isEmpty())
        {
            char *email = megaApi->getMyEmail();
            if (email)
            {
                fullname = QString::fromUtf8(email);
                delete [] email;
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
        delete [] userHandle;
    }
}

void AvatarWidget::clearData()
{
    letter = QChar();
    pathToFile = QString();
    color = QString();
}

void AvatarWidget::paintEvent(QPaintEvent *event)
{
    if (letter.isNull() && pathToFile.isNull())
    {
        return;
    }

    QPainter painter(this);

    // Draw border image
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);

    qreal factor = width() / 36.0;
    int innercirclediam = qFloor(24.0 * factor / 2) * 2;
    painter.translate(width() / 2, height() / 2);

    //Paint grey border of avatar with 4px pen width
    painter.setPen(QPen(QColor(172, 172, 172), 4.0));
    painter.drawEllipse(QRect(-innercirclediam  / 2, -innercirclediam / 2 , innercirclediam, innercirclediam));

    //Paint white border of avatar with 3.5px pen width
    painter.setPen(QPen(QColor(Qt::white),3.5));
    painter.drawEllipse(QRect(-innercirclediam  / 2, -innercirclediam / 2 , innercirclediam, innercirclediam));

    if (QFileInfo(pathToFile).exists())
    {

        QPixmap out = mask_image(pathToFile, 36.0 * factor);
        //Apply avatar
        painter.drawPixmap(QRect(-innercirclediam / 2, -innercirclediam / 2 , innercirclediam, innercirclediam), out/*QPixmap::fromImage(avatar)*/);
    }
    else
    {
        QFont font;
        font.setPixelSize(18.0 * factor);
        font.setFamily(QString::fromUtf8("Lato"));
        painter.setFont(font);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(color.size() ? color : QString::fromUtf8("#D90007"))));
        painter.drawEllipse(QRect(-innercirclediam / 2, -innercirclediam / 2, innercirclediam, innercirclediam));

        painter.setPen(QPen(QColor("#ffffff")));
        painter.drawText(QRect(-innercirclediam / 2, -innercirclediam / 2, innercirclediam, innercirclediam), Qt::AlignCenter, letter);
    }
}

void AvatarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        ((MegaApplication *)qApp)->openSettings(SettingsDialog::ACCOUNT_TAB);
    }
}

QPixmap AvatarWidget::mask_image(QString pathToFile, int size)
{
// Return a QPixmap from image loaded from pathToFile masked with a smooth circle.
// The returned image will have a size of size × size pixels.
// Load image and convert to 32-bit ARGB (adds an alpha channel):
// Snipped based on Stefan scherfke code

    if (!QFileInfo(pathToFile).exists())
    {
        return QPixmap();
    }

    QPixmap pm;
    QImage image(pathToFile);
    image.convertToFormat(QImage::Format_ARGB32);

// Crop image to a square:
    int imgsize = qMin(image.width(), image.height());
    QRect rect = QRect((image.width() - imgsize) / 2,
        (image.height() - imgsize) / 2,
        imgsize,
        imgsize
    );
    image = image.copy(rect);

// Create the output image with the same dimensions and an alpha channel
// and make it completely transparent:
    QImage out_img = QImage(imgsize, imgsize, QImage::Format_ARGB32);
    out_img.fill(Qt::transparent);

// Create a texture brush and paint a circle with the original image onto
// the output image:
    QBrush brush = QBrush(image);     // Create texture brush
    QPainter painter(&out_img);  // Paint the output image
    painter.setPen(Qt::NoPen);     // Don't draw an outline
    painter.setRenderHint(QPainter::Antialiasing, true);  // Use AA

    painter.setBrush(brush);    // Use the image texture brush
    painter.drawEllipse(0, 0, imgsize, imgsize);  // Actually draw the circle

    painter.end();               // We are done (segfault if you forget this)

// Convert the image to a pixmap and rescale it.  Take pixel ratio into
// account to get a sharp image on retina displays:
    qreal pr = QWindow().devicePixelRatio();
    pm = QPixmap::fromImage(out_img);
    pm.setDevicePixelRatio(pr);
    size *= pr;
    pm = pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return pm;
}

AvatarWidget::~AvatarWidget()
{

}
