#ifndef CATEGORYTHEME_H
#define CATEGORYTHEME_H

#include <QObject>

class QmlTheme;

class CategoryTheme : public QObject
{
    Q_OBJECT

public:
    explicit CategoryTheme(const QmlTheme* const theme, QObject *parent = nullptr);

    Q_INVOKABLE QString getValue(QString category, QString tokenId);

signals:

private:
    const QmlTheme* const mTheme;
};

#endif // CATEGORYTHEME_H
