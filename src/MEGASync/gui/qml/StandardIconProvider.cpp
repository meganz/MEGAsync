#include "StandardIconProvider.h"

#include "Utilities.h"

#include <QStyle>
#include <QApplication>

StandardIconProvider::StandardIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap StandardIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    static const auto metaobject = QMetaEnum::fromType<QStyle::StandardPixmap>();
    const int value = metaobject.keyToValue(id.toLatin1());
    QIcon icon = QApplication::style()->standardIcon(static_cast<QStyle::StandardPixmap>(value));
    QPixmap pixmap = icon.pixmap(requestedSize);
    *size = pixmap.size();
    return pixmap;
}
