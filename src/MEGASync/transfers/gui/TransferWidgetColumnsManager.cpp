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

void TransferWidgetColumnsManager::addColumnsWidget(QWidget* widget, const ColumnsWidget& info)
{
    QWidget::connect(widget,
                     &QWidget::destroyed,
                     this,
                     &TransferWidgetColumnsManager::removeColumnsWidget);
    mColumnsByWidget.insert(widget, info);
}

void TransferWidgetColumnsManager::removeColumnsWidget(QObject* object)
{
    if (auto widget = qobject_cast<QWidget*>(object))
    {
        mColumnsByWidget.remove(widget);
    }
}

void TransferWidgetColumnsManager::setColumnVisibility(const ColumnsInfo& info)
{
    auto columnsVisibility(info.visibility);

    auto columnTypes(columnsVisibility.keys());

    for (auto widgetByColumn = mColumnsByWidget.cbegin(), end = mColumnsByWidget.cend();
         widgetByColumn != end;
         ++widgetByColumn)
    {
        int widthSaved(0);
        for (auto columnType: columnTypes)
        {
            bool isVisible(columnsVisibility.value(columnType));
            bool wasVisible(mColumnsVisibility.visibility.value(columnType));

            if (isVisible != wasVisible)
            {
                auto columnWidget((*widgetByColumn).value(columnType));

                columnWidget->setVisible(isVisible);
                if (!isVisible)
                {
                    widthSaved += columnWidget->minimumWidth();
                }
            }
        }

        // Reset expanded columns width
        // When the new expanded column is not the old one
        if (mColumnsVisibility.columnExpanded.has_value())
        {
            if (!info.columnExpanded.has_value() ||
                (mColumnsVisibility.columnExpanded.value() != info.columnExpanded.value()))
            {
                auto columnWidget(
                    (*widgetByColumn).value(mColumnsVisibility.columnExpanded.value()));
                if (columnWidget)
                {
                    bool ok;
                    auto originalWidth = columnWidget->property(OLD_SIZE).toInt(&ok);
                    if (ok)
                    {
                        columnWidget->setFixedWidth(originalWidth);
                    }
                }
            }
        }

        // Set new expanded columns width
        if (widthSaved > 0 && info.columnExpanded.has_value())
        {
            auto columnWidget((*widgetByColumn).value(info.columnExpanded.value()));
            if (columnWidget)
            {
                columnWidget->setProperty(OLD_SIZE, columnWidget->minimumWidth());
                auto newWidth(columnWidget->minimumWidth() + widthSaved);
                columnWidget->setFixedWidth(newWidth);
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
