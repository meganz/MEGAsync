#ifndef EXTERNALDIALOGOPENER_H
#define EXTERNALDIALOGOPENER_H

#include <QEvent>
#include <QWidget>
#include <QObject>
#include <QPointer>

class ExternalDialogOpener : public QObject
{
public:
    ExternalDialogOpener();
    ~ExternalDialogOpener();

private:
    QPointer<QWidget> mActivationWidget;
};

#endif // EXTERNALDIALOGOPENER_H
