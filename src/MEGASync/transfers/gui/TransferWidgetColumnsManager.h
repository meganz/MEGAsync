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
    Q_OBJECT

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
    void addHeaderWidget(const ColumnsWidget& info);
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

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void removeColumnsWidget(QObject* widget);

private:
    void applyProperty(QObject* object, std::function<void(QWidget*, QWidget*)> func);

    ColumnsWidget mHeaderColumns;
    QMap<QWidget*, ColumnsWidget> mDelegateColumns;
    ColumnsInfo mColumnsVisibility;
};

#endif // TRANSFERWIDGETCOLUMNSMANAGER_H
