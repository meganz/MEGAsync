#include "ButtonIconManager.h"

#include <QFileInfo>
#include <QVariant>
#include <QDir>

const char* ButtonIconManager::ICON_PREFIX = "default_icon";
const char* ButtonIconManager::HOVER_SELECTED_FLAG = "hover_selected";
const char* ButtonIconManager::CHECK_STATE = "check_state";
const char* ButtonIconManager::IGNORE_BUTTON = "ignore_button_manager";
const char* ButtonIconManager::CHANGE_LATER = "change_if_hover_on_selection";

const char* ButtonIconManager::ICON_SPACING = "icon_spacing";
const char* ButtonIconManager::BUTTON_FULL_TEXT = "button_full_text";
const char* ButtonIconManager::BUTTON_ELIDE_TEXT = "button_elide_text";
const char* ButtonIconManager::NOT_CHANGE_TEXT_COLOR = "not_change_text_color";

const QString QRC_PREFIX = QLatin1Literal("qrc");

const char* ButtonIconManager::DISABLE_UNCHECK_ON_CLICK = "disable_uncheck_on_click";

ButtonIconManager::ButtonIconManager(QObject * parent) :
    QObject(parent)
{}

void ButtonIconManager::addButton(QAbstractButton *button)
{
    if(!button->property(IGNORE_BUTTON).toBool())
    {
        button->installEventFilter(this);
        setDefaultIcon(button);
        button->setProperty(CHECK_STATE, button->isChecked());
    }
}

bool ButtonIconManager::eventFilter(QObject * watched, QEvent * event)
{
    QAbstractButton * button = qobject_cast<QAbstractButton*>(watched);
    if (!button)
    {
        return false;
    }

    if(button->isEnabled())
    {
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
        //Do not depend on checked signal as the button signals can be blocked
        else if(event->type() == QEvent::Paint)
        {
            bool checkstateHasChanged = button->isCheckable() && (button->property(CHECK_STATE).toBool() != button->isChecked());
            bool changeLater = button->property(CHANGE_LATER).isValid() ? button->property(CHANGE_LATER).toBool() : false;
            bool change(true);

            if(changeLater)
            {
               change = !button->rect().contains(button->mapFromGlobal(QCursor::pos()));
            }

            if(checkstateHasChanged && change)
            {
                setDefaultIcon(button);
                button->setProperty(CHECK_STATE, button->isChecked());
            }

            bool buttonTextHasChanged = button->property(BUTTON_ELIDE_TEXT).isValid() && button->text() != button->property(BUTTON_ELIDE_TEXT).toString();
            if(buttonTextHasChanged)
            {
                addIconSpacing(button);
                button->setProperty(BUTTON_FULL_TEXT, button->text());
                elideButtonText(button, button->property(BUTTON_FULL_TEXT).toString());
            }
        }
        else if(button->isCheckable() && event->type() == QEvent::MouseButtonPress)
        {
            auto disableUncheckOnClick = button->property(DISABLE_UNCHECK_ON_CLICK).toBool();
            if(disableUncheckOnClick)
            {
                if(button->isChecked())
                {
                    event->accept();
                    return true;
                }
            }
        }
    }

    if(event->type() == QEvent::Resize)
    {
        if(!button->text().isEmpty())
        {
            if(!button->property(BUTTON_FULL_TEXT).isValid())
            {
                addIconSpacing(button);
                button->setProperty(BUTTON_FULL_TEXT, button->text());
            }

            elideButtonText(button, button->property(BUTTON_FULL_TEXT).toString());
        }
    }

    //Update the icon if the default_icon property changes
    if(event->type() == QEvent::DynamicPropertyChange)
    {
        if(auto propertyChanged = dynamic_cast<QDynamicPropertyChangeEvent*>(event))
        {
            if(propertyChanged->propertyName() == ICON_PREFIX)
            {
                setDefaultIcon(button);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void ButtonIconManager::setDefaultIcon(QAbstractButton *button)
{
    auto iconInfo = splitIconPath(button->property(ICON_PREFIX).toString());

    if(!iconInfo.isEmpty())
    {
        QIcon newIcon;
        // The push button is not hovered by mouse
        if(button->isCheckable() && button->isChecked())
        {
            iconInfo.iconName.append(mSettings.selected_suffix);
            fillIcon(iconInfo, newIcon);
            changeButtonTextColor(button, 1.0);
        }
        else
        {
            iconInfo.iconName.append(mSettings.default_suffix);
            fillIcon(iconInfo, newIcon);
            changeButtonTextColor(button, 1.0 - mSettings.opacityGap);
        }

        button->setIcon(newIcon);
    }
}

void ButtonIconManager::setHoverIcon(QAbstractButton *button)
{
    auto iconInfo = splitIconPath(button->property(ICON_PREFIX).toString());

    if(!iconInfo.isEmpty())
    {
        auto hoverSelectedAvailable = button->property(HOVER_SELECTED_FLAG).toBool();
        QIcon newIcon;
        // The push button is hovered by mouse
        if(button->isCheckable() && button->isChecked())
        {
            if(hoverSelectedAvailable)
            {
                iconInfo.iconName.append(mSettings.hover_selected_suffix);
                fillIcon(iconInfo, newIcon);
            }
            else
            {
                iconInfo.iconName.append(mSettings.selected_suffix);
                fillIcon(iconInfo, newIcon);
            }
        }
        else
        {
            iconInfo.iconName.append(mSettings.hover_suffix);
            fillIcon(iconInfo, newIcon);
        }

        changeButtonTextColor(button, 1.0);
        button->setIcon(newIcon);
    }
}

void ButtonIconManager::setSelectedIcon(QAbstractButton *button)
{
    auto iconInfo = splitIconPath(button->property(ICON_PREFIX).toString());

    if(!iconInfo.isEmpty())
    {
        //The button is checked
        QIcon newIcon;
        if(button->isChecked())
        {
            iconInfo.iconName.append(mSettings.selected_suffix);
            fillIcon(iconInfo, newIcon);
            changeButtonTextColor(button, 1.0);
        }
        else
        {
            iconInfo.iconName.append(mSettings.hover_suffix);
            fillIcon(iconInfo, newIcon);
            changeButtonTextColor(button, 1.0 - mSettings.opacityGap);
        }

        button->setIcon(newIcon);
    }
}

void ButtonIconManager::elideButtonText(QAbstractButton* button, const QString& text)
{
    if(button->sizePolicy().horizontalPolicy() == QSizePolicy::Expanding)
    {
        auto availableSpace = button->width() - (button->contentsMargins().left() + button->contentsMargins().right());
        if(!button->icon().isNull())
        {
            availableSpace = availableSpace - (button->iconSize().width() + 5);
        }

        auto elideText = button->fontMetrics().elidedText(text, Qt::ElideMiddle, availableSpace);
        button->setText(elideText);
        button->setProperty(BUTTON_ELIDE_TEXT, elideText);
    }
}

void ButtonIconManager::changeButtonTextColor(QAbstractButton* button, double alphaValue)
{
    if(!button->text().isEmpty()
       && ((button->property(NOT_CHANGE_TEXT_COLOR).isValid() && !button->property(NOT_CHANGE_TEXT_COLOR).toBool())
            || !button->property(NOT_CHANGE_TEXT_COLOR).isValid()))
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

ButtonIconManager::IconInfo ButtonIconManager::splitIconPath(const QString &iconPath)
{
    IconInfo info;

    QFileInfo pathInfo(iconPath);
    info.extension = pathInfo.completeSuffix();
    info.iconPath = pathInfo.path();
    if(info.iconPath.startsWith(QRC_PREFIX))
    {
        info.iconPath.remove(0,QRC_PREFIX.length());
    }
    info.iconName = pathInfo.baseName();

    //Temporary code...not very clean
    if(!cleanIconName(info, QString::fromLatin1("_")))
    {
        cleanIconName(info, QString::fromLatin1("-"));
    }

    return info;
}

bool ButtonIconManager::cleanIconName(IconInfo& info, const QString& separator)
{
    auto splittedIconPath = info.iconName.split(separator);
    if(splittedIconPath.size() > 1)
    {
        splittedIconPath.removeLast();
        info.iconName = splittedIconPath.join(separator);
        return true;
    }
    else
    {
        info.iconName.clear();
        return false;
    }
}

void ButtonIconManager::addIconSpacing(QAbstractButton *button)
{
    if(button->property(ICON_SPACING).isValid())
    {
        QString spaceChar(QLatin1String(" "));
        uint spacing = button->property(ICON_SPACING).toUInt();
        uint space_size = button->fontMetrics().width(spaceChar);
        uint number_of_spaces = spacing / space_size;
        button->setText(spaceChar.repeated(number_of_spaces) + button->text());
    }
}

void ButtonIconManager::fillIcon(const IconInfo& info, QIcon& icon)
{
    QString result;
    QString separator(QString::fromLatin1("/"));
    QString pointSeparator(QString::fromLatin1("."));
    result.reserve(info.iconPath.length()
                   + separator.length()
                   + info.iconName.length()
                   + pointSeparator.length()
                   + info.extension.length());

    result.append(info.iconPath);
    result.append(separator);
    result.append(info.iconName);
    result.append(pointSeparator);
    result.append(info.extension);

    icon.addFile(QDir::toNativeSeparators(result), QSize(), QIcon::Normal, QIcon::Off);
}
