#include "PasswordLineEdit.h"

#include <QString>
#include <QIcon>
#include <QAction>
#include <QToolButton>
#include <QEvent>
#include <QApplication>
#include <QStyle>

PasswordLineEdit::PasswordLineEdit(QWidget *parent) : QLineEdit(parent)
{
    mShowAction = new QAction(this);
    mShowAction->setToolTip(tr("Show password"));
    QIcon showActionIcon;
    showActionIcon.addFile((QString::fromUtf8(":/images/password/ico_eye_reveal.png")), QSize(), QIcon::Normal);
    showActionIcon.addFile((QString::fromUtf8(":/images/password/ico_eye_reveal_disabled.png")), QSize(), QIcon::Disabled);
    mShowAction->setIcon(showActionIcon);
    addAction(mShowAction, QLineEdit::TrailingPosition);

    mHideAction = new QAction(this);
    mHideAction->setToolTip(tr("Hide password"));
    QIcon hideActionIcon;
    hideActionIcon.addFile((QString::fromUtf8(":/images/password/ico_eye_hide.png")), QSize(), QIcon::Normal);
    hideActionIcon.addFile((QString::fromUtf8(":/images/password/ico_eye_hide_disabled.png")), QSize(), QIcon::Disabled);
    mHideAction->setIcon(hideActionIcon);
    addAction(mHideAction, QLineEdit::TrailingPosition);

    mHideAction->setVisible(false);
    mHideAction->setEnabled(false);
    mShowAction->setEnabled(false);

    auto toolButtons = findChildren<QToolButton*>();
    foreach(QToolButton* toolButton, toolButtons)
    {
        toolButton->setCursor(Qt::PointingHandCursor);
    }

    connect(mShowAction, &QAction::triggered, this, &PasswordLineEdit::revealPassword);
    connect(mHideAction, &QAction::triggered, this, &PasswordLineEdit::hidePassword);
}

void PasswordLineEdit::revealPassword()
{
    setEchoMode(QLineEdit::Normal);
    mHideAction->setVisible(true);
    mShowAction->setVisible(false);
}

void PasswordLineEdit::hidePassword()
{
    setEchoMode(QLineEdit::Password);
    mHideAction->setVisible(false);
    mShowAction->setVisible(true);
}

void PasswordLineEdit::focusInEvent(QFocusEvent* e)
{
    mHideAction->setEnabled(true);
    mShowAction->setEnabled(true);
    QLineEdit::focusInEvent(e);
}

void PasswordLineEdit::focusOutEvent(QFocusEvent* e)
{
    if(text().isEmpty())
    {
        mHideAction->setEnabled(false);
        mShowAction->setEnabled(false);
    }
    QLineEdit::focusOutEvent(e);
}

