#ifndef TOKENIZEDICON_H
#define TOKENIZEDICON_H

#include "Preferences.h"

#include <QAbstractButton>
#include <QEvent>
#include <QIcon>
#include <QMap>
#include <QObject>

class TokenizedIcon: public QObject
{
    Q_OBJECT

public:
    static void reset(QAbstractButton* button);
    static void addPixmap(QAbstractButton* button,
                          QIcon::Mode mode,
                          QIcon::State state,
                          const QString& token);

private:
    static const char* TOKEN_ICON_PROPERTY;

    TokenizedIcon(QAbstractButton* button);

    static TokenizedIcon* getTokenizedIcon(QAbstractButton* button);

    void performReset();
    void performAddPixmap(QIcon::Mode mode, QIcon::State state, const QString& token);
    void addPixmapToIcon(const QPixmap& pix, QIcon::Mode mode, QIcon::State state);

    struct PixmapInfo
    {
        QPixmap pixm;
        QIcon::Mode mode;
        QIcon::State state;
        QString token;
    };

    QMultiMap<QString, PixmapInfo> mPixmapsByToken;

    QIcon mOriginalIcon;
    QIcon mTokenizedIcon;
    QAbstractButton* mButton;
    Preferences::ThemeAppeareance mThemeAppearance;
};

Q_DECLARE_METATYPE(TokenizedIcon*)

#endif // TOKENIZEDICON_H
