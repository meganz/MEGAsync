#ifndef VERIFYEMAILMESSAGE_H
#define VERIFYEMAILMESSAGE_H

#include <QDialog>
#include <QMouseEvent>

#ifdef __APPLE__
    #include "macx/LockedPopOver.h"
    #import <objc/runtime.h>
#endif

namespace Ui {
class VerifyEmailMessage;
}

class VerifyEmailMessage : public QDialog
{
    Q_OBJECT

public:
    explicit VerifyEmailMessage(QWidget *parent = nullptr);
    ~VerifyEmailMessage();

signals:
    void logout();
    void resendEmail();

private slots:
    void on_bLogout_clicked();
    void on_bResendEmail_clicked();

protected:
    void mousePressEvent(QMouseEvent *event);
    void changeEvent(QEvent *event);

private:
    Ui::VerifyEmailMessage *m_ui;
    std::unique_ptr<LockedPopOver> m_nativeWidget{new LockedPopOver()};
#ifdef __APPLE__
    id m_popover;
#endif
};

#endif // VERIFYEMAILMESSAGE_H
