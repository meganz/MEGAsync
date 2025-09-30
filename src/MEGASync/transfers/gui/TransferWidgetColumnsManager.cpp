#include "TransferWidgetColumnsManager.h"

#include "Utilities.h"

constexpr const char* OLD_SIZE = "old_size";

TransferWidgetColumnsManager::TransferWidgetColumnsManager()
{
    for (int index = toInt(Columns::NAME); index <= toInt(Columns::PAUSE_RESUME); index++)
    {
        mColumnsVisibility.visibility.insert(static_cast<Columns>(index), true);
    }
}

void TransferWidgetColumnsManager::addHeaderWidget(const ColumnsWidget& info)
{
    mHeaderColumns = info;

    for (auto& widget: mHeaderColumns)
    {
        widget->installEventFilter(this);
    }
}

void TransferWidgetColumnsManager::addColumnsWidget(QWidget* widget, const ColumnsWidget& info)
{
    QWidget::connect(widget,
                     &QWidget::destroyed,
                     this,
                     &TransferWidgetColumnsManager::removeColumnsWidget);
    mDelegateColumns.insert(widget, info);

    for (auto columnType = info.keyBegin(), end = info.keyEnd(); columnType != end; ++columnType)
    {
        auto headerWidget = mHeaderColumns.value((*columnType));
        auto delegateWidget = info.value((*columnType));
        if (headerWidget && delegateWidget)
        {
            delegateWidget->setVisible(mColumnsVisibility.visibility.value((*columnType)));
            delegateWidget->setFixedWidth(headerWidget->width());
        }
    }
}

void TransferWidgetColumnsManager::removeColumnsWidget(QObject* object)
{
    if (auto widget = qobject_cast<QWidget*>(object))
    {
        mDelegateColumns.remove(widget);
    }
}

void TransferWidgetColumnsManager::setColumnVisibility(const ColumnsInfo& info)
{
    auto columnsVisibility(info.visibility);

    int widthSaved(0);

    // Calculate size per header
    for (auto columnType: columnsVisibility.keys())
    {
        auto headerWidget = mHeaderColumns.value(columnType);
        if (headerWidget)
        {
            bool isVisible(columnsVisibility.value(columnType));
            bool wasVisible(mColumnsVisibility.visibility.value(columnType));

            if (isVisible != wasVisible)
            {
                headerWidget->setVisible(isVisible);
                if (!isVisible)
                {
                    widthSaved += headerWidget->minimumWidth();
                }
            }

            // Reset expanded columns width
            // When the new expanded column is not the old one
            if (mColumnsVisibility.columnExpanded.has_value() &&
                mColumnsVisibility.columnExpanded.value() == columnType &&
                (!info.columnExpanded.has_value() ||
                 (mColumnsVisibility.columnExpanded.value() != info.columnExpanded.value())))
            {
                bool ok;
                auto originalWidth = headerWidget->property(OLD_SIZE).toInt(&ok);
                if (ok)
                {
                    headerWidget->setFixedWidth(originalWidth);
                }
            }

            // Set new expanded columns width
            if (widthSaved > 0 && info.columnExpanded.has_value() &&
                info.columnExpanded.value() == columnType)
            {
                headerWidget->setProperty(OLD_SIZE, headerWidget->minimumWidth());
                auto newWidth(headerWidget->minimumWidth() + widthSaved);
                headerWidget->setFixedWidth(newWidth);
            }
        }
    }

    mColumnsVisibility = info;
}

const TransferWidgetColumnsManager::ColumnsInfo&
    TransferWidgetColumnsManager::columnsVisibility() const
{
    return mColumnsVisibility;
}

int TransferWidgetColumnsManager::getCurrentTab() const
{
    return mColumnsVisibility.currentTab;
}

void TransferWidgetColumnsManager::applyProperty(QObject* object,
                                                 std::function<void(QWidget*, QWidget*)> func)
{
    auto headerWidget(dynamic_cast<QWidget*>(object));
    if (headerWidget)
    {
        auto column = mHeaderColumns.key(headerWidget);
        for (auto widgetByColumn = mDelegateColumns.cbegin(), end = mDelegateColumns.cend();
             widgetByColumn != end;
             ++widgetByColumn)
        {
            func((*widgetByColumn).value(column), headerWidget);
        }
    }
}

bool TransferWidgetColumnsManager::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Resize)
    {
        auto func = [](QWidget* delegate, QWidget* header)
        {
            delegate->setFixedWidth(header->width());
        };
        applyProperty(watched, func);
    }
    else if (event->type() == QEvent::Hide)
    {
        auto func = [](QWidget* delegate, QWidget*)
        {
            delegate->hide();
        };
        applyProperty(watched, func);
    }
    else if (event->type() == QEvent::Show)
    {
        auto func = [](QWidget* delegate, QWidget*)
        {
            delegate->show();
        };
        applyProperty(watched, func);
    }

    return QObject::eventFilter(watched, event);
}
