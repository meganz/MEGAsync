#include "PasswordLineEdit.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
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
    setEyeImage(mShowAction, eyeRevealImage, QIcon::Normal);
}

QString PasswordLineEdit::getEyeRevealDisabledImage() const
{
    return {};
}

void PasswordLineEdit::setEyeRevealDisabledImage(const QString& eyeRevealDisabledImage)
{
    setEyeImage(mShowAction, eyeRevealDisabledImage, QIcon::Disabled);
}

QString PasswordLineEdit::getEyeClosedImage() const
{
    return {};
}

void PasswordLineEdit::setEyeClosedImage(const QString& eyeClosedImage)
{
    setEyeImage(mHideAction, eyeClosedImage, QIcon::Normal);
}

QString PasswordLineEdit::getEyeClosedDisabledImage() const
{
    return {};
}

void PasswordLineEdit::setEyeClosedDisabledImage(const QString& eyeClosedDisabledImage)
{
    setEyeImage(mHideAction, eyeClosedDisabledImage, QIcon::Disabled);
}

void PasswordLineEdit::setEyeImage(QAction* action, const QString& imagePath, QIcon::Mode mode)
{
    QIcon icon = action->icon();
    icon.addFile(imagePath, QSize(), mode);
    action->setIcon(icon);

    emit eyeImageChanged();
}
