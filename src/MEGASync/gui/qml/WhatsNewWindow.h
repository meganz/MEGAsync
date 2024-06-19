#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include "QmlDialogWrapper.h"

class WhatsNewWindow : public QMLComponent
{
    Q_OBJECT

public:
    explicit WhatsNewWindow(QObject *parent = 0);

    QUrl getQmlUrl() override;

    QString contextName() override;
};

#endif // ABOUTWINDOW_H
