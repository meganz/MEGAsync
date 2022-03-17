#ifndef PASSWORDLINEEDIT_H
#define PASSWORDLINEEDIT_H
#include <QLineEdit>


class PasswordLineEdit : public QLineEdit
{
public:
    explicit PasswordLineEdit(QWidget *parent = nullptr);

private slots:
    void onTextChanged();

private:
    void revealPassword();
    void hidePassword();

    QAction* mHideAction;
    QAction* mShowAction;

    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
};

#endif // PASSWORDLINEEDIT_H
