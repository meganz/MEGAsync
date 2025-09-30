#ifndef ICONPROPERTY_H
#define ICONPROPERTY_H

#include <QAbstractButton>
#include <QIcon>

#define DEFINE_ICON_PROPERTY() \
\
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon) \
\
public: \
    void setIcon(const QIcon& icon) \
    { \
        QAbstractButton::setIcon(icon); \
        forceUpdate(); \
    }

#endif // ICONPROPERTY_H
