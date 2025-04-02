#ifndef CHOOSEFILE_H
#define CHOOSEFILE_H

#include <QObject>

class ChooseLocalFile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title MEMBER mTitle)

public:
    ChooseLocalFile(QObject* parent = nullptr);

    Q_INVOKABLE void openFileSelector(const QString& folderPath = QString());
    Q_INVOKABLE void openRelativeFileSelector(const QString& folderPath = QString());

signals:
    void fileChosen(QString folderPath);

private:
    void sendFileChosenSignal(const QString& file, const QString& openFromFolder = QString());

    QString mTitle;

};

#endif // CHOOSEFILE_H
