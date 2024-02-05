#ifndef STYLEVALUES_H
#define STYLEVALUES_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

class StyleValues : public QObject
{
    Q_OBJECT

public:
    explicit StyleValues(QQmlEngine* engine, QObject *parent = nullptr);
    ~StyleValues();
    QString getColor(QString tokenId);
    void onThemeChanged();

private:
    QQmlEngine* mEngine;
    QObject* mDarkColors;
    QObject* mLightColors;
};

class ColorStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString surface1 READ getSurface1 NOTIFY valueChanged)

public:
    explicit ColorStyle(QQmlEngine* engine, QObject *parent = nullptr):
        QObject{parent},
        mEngine{engine},
        mStyleValues{engine, parent}
    {
    }

    QString getSurface1()
    {
        return mStyleValues.getColor(QString::fromUtf8("surface1"));
    }

    void onThemeChanged()
    {
        emit valueChanged();
    }

signals:
    void valueChanged();

private:
    QQmlEngine* mEngine;
    StyleValues mStyleValues;
};


#endif // STYLEVALUES_H
