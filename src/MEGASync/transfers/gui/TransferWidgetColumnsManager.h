#ifndef TRANSFERWIDGETCOLUMNSMANAGER_H
#define TRANSFERWIDGETCOLUMNSMANAGER_H

#include <QMap>
#include <QObject>
#include <QSize>
#include <QString>
#include <QWidget>

#include <optional>

class TransferWidgetColumnsManager: public QObject
{
public:
    enum class Columns
    {
        NAME,
        SIZE,
        SPEED,
        STATUS,
        TIME,
        CLEAR_CANCEL,
        PAUSE_RESUME,
    };

    TransferWidgetColumnsManager();

    using ColumnsWidget = QMap<Columns, QWidget*>;
    void addColumnsWidget(QWidget* widget, const ColumnsWidget& info);

    struct ColumnsInfo
    {
        QMap<Columns, bool> visibility;
        std::optional<Columns> columnExpanded;
        int currentTab = 0;
    };

    void setColumnVisibility(const ColumnsInfo& columnsVisibility);
    const ColumnsInfo& columnsVisibility() const;

    int getCurrentTab() const;

private slots:
    void removeColumnsWidget(QObject* widget);

private:
    QMap<QWidget*, ColumnsWidget> mColumnsByWidget;
    ColumnsInfo mColumnsVisibility;
};

#endif // TRANSFERWIDGETCOLUMNSMANAGER_H
