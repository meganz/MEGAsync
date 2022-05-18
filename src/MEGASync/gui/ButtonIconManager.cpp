#include "ButtonIconManager.h"

#include <QFileInfo>
#include <QUrl>
#include <QVariant>
#include <QDir>

#include <QDebug>

const char* ButtonIconManager::ICON_PREFIX = "icon_prefix";
const char* ButtonIconManager::HOVER_SELECTED_FLAG = "hover_selected";
const char* ButtonIconManager::CHECK_STATE = "check_state";

ButtonIconManager::ButtonIconManager(QObject * parent) :
    QObject(parent)
{}

void ButtonIconManager::addButton(QAbstractButton *button)
{
    button->installEventFilter(this);
    connect(button, &QAbstractButton::toggled, this, &ButtonIconManager::onButtonChecked);

    changeButtonTextColor(button, 1.0 - mSettings.opacityGap);
    setDefaultIcon(button);

    button->setProperty(CHECK_STATE, button->isChecked());
}

bool ButtonIconManager::eventFilter(QObject * watched, QEvent * event)
{
    QAbstractButton * button = qobject_cast<QAbstractButton*>(watched);
    if (!button)
    {
        return false;
    }

    if(event->type() == QEvent::Enter || event->type() == QEvent::Leave)
    {
        if (event->type() == QEvent::Enter)
        {
            setHoverIcon(button);
        }
        else if (event->type() == QEvent::Leave)
        {
            setDefaultIcon(button);
        }
    }
    //Do not depend on checked signal
    else if(event->type() == QEvent::Paint)
    {
        if(button->isCheckable())
        {
            if(button->property(CHECK_STATE).toBool() != button->isChecked())
            {
                setDefaultIcon(button);
                button->setProperty(CHECK_STATE, button->isChecked());
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void ButtonIconManager::setDefaultIcon(QAbstractButton *button)
{
    splitIconPath(button->property(ICON_PREFIX).toUrl());

    if(!mIconBaseName.isEmpty())
    {
        auto newIcon = button->icon();
        // The push button is hovered by mouse
        // The push button is not hovered by mouse
        if(button->isCheckable() && button->isChecked())
        {
            fillIcon(newIcon, mIconBaseName.append(mSettings.selected_suffix));
            changeButtonTextColor(button, 1.0);
        }
        else
        {
            fillIcon(newIcon, mIconBaseName.append(mSettings.default_suffix));
            changeButtonTextColor(button, 1.0 - mSettings.opacityGap);
        }

        button->setIcon(newIcon);
    }
}

void ButtonIconManager::setHoverIcon(QAbstractButton *button)
{
    splitIconPath(button->property(ICON_PREFIX).toUrl());

    if(!mIconBaseName.isEmpty())
    {
        auto hoverSelectedAvailable = button->property(HOVER_SELECTED_FLAG).toBool();
        auto newIcon = button->icon();
        // The push button is hovered by mouse
        if(button->isCheckable() && button->isChecked())
        {
            if(hoverSelectedAvailable)
            {
                fillIcon(newIcon, mIconBaseName.append(mSettings.hover_selected_suffix));
            }
            else
            {
                fillIcon(newIcon, mIconBaseName.append(mSettings.hover_suffix));
            }
        }
        else
        {
            fillIcon(newIcon, mIconBaseName.append(mSettings.hover_suffix));
        }

        changeButtonTextColor(button, 1.0);
        button->setIcon(newIcon);
    }
}

void ButtonIconManager::setSelectedIcon(QAbstractButton *button)
{
    splitIconPath(button->property(ICON_PREFIX).toUrl());
    if(!mIconBaseName.isEmpty())
    {
        auto newIcon = button->icon();
        if(button->isChecked())
        {
            fillIcon(newIcon, mIconBaseName.append(mSettings.selected_suffix));
            changeButtonTextColor(button, 1.0);
        }
        else
        {
            fillIcon(newIcon, mIconBaseName.append(mSettings.hover_suffix));
            changeButtonTextColor(button, 1.0 - mSettings.opacityGap);
        }


        button->setIcon(newIcon);
    }
}

void ButtonIconManager::onButtonChecked()
{
    QAbstractButton * button = qobject_cast<QAbstractButton*>(sender());
    if (!button)
    {
        return;
    }

    setSelectedIcon(button);
}

void ButtonIconManager::changeButtonTextColor(QAbstractButton* button, double alphaValue)
{
    if(!button->text().isEmpty())
    {
        QColor textColor(button->palette().color(QPalette::ColorRole::ButtonText));
        textColor.setAlphaF(alphaValue);
        auto paletteButton = button->palette();
        paletteButton.setColor(QPalette::ButtonText, textColor);
        button->setPalette(paletteButton);

        button->setStyleSheet(QString::fromLatin1("color: rgba(%1,%2,%3,%4);")
                              .arg(QString::number(textColor.red()))
                              .arg(QString::number(textColor.green()))
                              .arg(QString::number(textColor.blue()))
                              .arg(QString::number(textColor.alpha())));
    }
}

void ButtonIconManager::splitIconPath(const QUrl &iconPath)
{
    QFileInfo info(iconPath.path());
    mExtension = info.completeSuffix();
    mIconPath = info.path();

    //Temporary code...not very clean
    if(!cleanIconName(info.baseName(), QString::fromLatin1("_")))
    {
        cleanIconName(info.baseName(), QString::fromLatin1("-"));
    }
}

bool ButtonIconManager::cleanIconName(const QString& name, const QString& separator)
{
    auto iconName = name;
    auto splittedIconPath = iconName.split(separator);
    if(splittedIconPath.size() > 1)
    {
        splittedIconPath.removeLast();
        mIconBaseName = splittedIconPath.join(separator);
        return true;
    }
    else
    {
        return false;
    }
}

void ButtonIconManager::fillIcon(QIcon &icon, const QString &iconPath)
{
    QString result;
    QString init(QString::fromLatin1(":"));
    QString separator(QString::fromLatin1("/"));
    QString pointSeparator(QString::fromLatin1("."));
    result.reserve(init.length()
                   + mIconPath.length()
                   + separator.length()
                   + iconPath.length()
                   + pointSeparator.length()
                   + mExtension.length());

    result.append(init);
    result.append(mIconPath);
    result.append(separator);
    result.append(iconPath);
    result.append(pointSeparator);
    result.append(mExtension);

    icon.addFile(QDir::toNativeSeparators(result), QSize(), QIcon::Normal, QIcon::Off);
}
