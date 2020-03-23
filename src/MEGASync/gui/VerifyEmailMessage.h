#ifndef VERIFYEMAILMESSAGE_H
#define VERIFYEMAILMESSAGE_H

#include <QDialog>
#include <QMouseEvent>
#include <memory>


#ifdef __APPLE__
    #include "macx/LockedPopOver.h"
    #import <objc/runtime.h>
#else
    #include "LockedPopOver.h"
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

#ifdef __APPLE__
    std::unique_ptr<LockedPopOver> m_nativeWidget{new LockedPopOver()};
    id m_popover;
#else
    std::unique_ptr<LockedPopOver> mLockedPopOver{new LockedPopOver(this)};
#endif
};

#endif // VERIFYEMAILMESSAGE_H
