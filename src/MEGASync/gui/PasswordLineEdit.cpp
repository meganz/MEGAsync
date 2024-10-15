#include "PasswordLineEdit.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QIcon>
#include <QString>
#include <QStyle>
#include <QToolButton>

PasswordLineEdit::PasswordLineEdit(QWidget* parent):
    QLineEdit(parent)
{
    mShowAction = new QAction(this);
    mShowAction->setToolTip(tr("Show password"));
    addAction(mShowAction, QLineEdit::TrailingPosition);

    mHideAction = new QAction(this);
    mHideAction->setToolTip(tr("Hide password"));
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
    if (text().isEmpty())
    {
        mHideAction->setEnabled(false);
        mShowAction->setEnabled(false);
    }
    QLineEdit::focusOutEvent(e);
}

QString PasswordLineEdit::getEyeRevealImage() const
{
    return {};
}

void PasswordLineEdit::setEyeRevealImage(const QString& eyeRevealImage)
{
    QIcon showActionIcon = mShowAction->icon();
    showActionIcon.addFile(eyeRevealImage, QSize(), QIcon::Normal);
    mShowAction->setIcon(showActionIcon);

    emit eyeImageChanged();
}

QString PasswordLineEdit::getEyeRevealDisabledImage() const
{
    return {};
}

void PasswordLineEdit::setEyeRevealDisabledImage(const QString& eyeRevealDisabledImage)
{
    QIcon showActionIcon = mShowAction->icon();
    showActionIcon.addFile(eyeRevealDisabledImage, QSize(), QIcon::Disabled);
    mShowAction->setIcon(showActionIcon);

    emit eyeImageChanged();
}

QString PasswordLineEdit::getEyeClosedImage() const
{
    return {};
}

void PasswordLineEdit::setEyeClosedImage(const QString& eyeClosedImage)
{
    QIcon hideActionIcon = mHideAction->icon();
    hideActionIcon.addFile(eyeClosedImage, QSize(), QIcon::Normal);
    mHideAction->setIcon(hideActionIcon);

    emit eyeImageChanged();
}

QString PasswordLineEdit::getEyeClosedDisabledImage() const
{
    return {};
}

void PasswordLineEdit::setEyeClosedDisabledImage(const QString& eyeClosedDisabledImage)
{
    QIcon hideActionIcon = mHideAction->icon();
    hideActionIcon.addFile(eyeClosedDisabledImage, QSize(), QIcon::Disabled);
    mHideAction->setIcon(hideActionIcon);

    emit eyeImageChanged();
}
