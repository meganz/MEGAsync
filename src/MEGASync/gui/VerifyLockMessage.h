#ifndef VERIFYLOCKMESSAGE_H
#define VERIFYLOCKMESSAGE_H

#include "megaapi.h"

#include <QDialog>
#include <QMouseEvent>


namespace Ui {
class VerifyLockMessage;
}

class VerifyLockMessage : public QDialog
{
    Q_OBJECT

public:
    explicit VerifyLockMessage(QWidget *parent = nullptr);
    ~VerifyLockMessage();
    void regenerateUI();
    void onRequestFinish(mega::MegaRequest *request, mega::MegaError* e);

signals:
    void logout();

private slots:
    void onHelpButtonClicked();

protected:
    bool event(QEvent* event) override;

private:
    Ui::VerifyLockMessage *m_ui;
};

#endif // VERIFYLOCKMESSAGE_H
