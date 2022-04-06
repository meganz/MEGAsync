#ifndef PASSWORDLINEEDIT_H
#define PASSWORDLINEEDIT_H
#include <QLineEdit>


class PasswordLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);

private:
    void revealPassword();
    void hidePassword();

    QAction* mHideAction;
    QAction* mShowAction;

    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
};

#endif // PASSWORDLINEEDIT_H
