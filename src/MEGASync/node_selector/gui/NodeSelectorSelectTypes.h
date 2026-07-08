#ifndef NODESELECTORSELECTTYPES_H
#define NODESELECTORSELECTTYPES_H

#include "NodeSelectorTabTypes.h"

#include <QIcon>
#include <QMap>
#include <QModelIndex>
#include <QPushButton>
#include <QSet>
#include <QString>

#include <memory>

class NodeSelectorProxyModel;
class NodeSelectorTreeViewWidget;
class NodeSelectorTreeView;
class SelectType;
typedef std::shared_ptr<SelectType> SelectTypeSPtr;

class SelectType
{
public:
    explicit SelectType();
    virtual ~SelectType() = default;

    virtual bool areActionsAllowed()
    {
        return false;
    }

    virtual bool isCurrentFolderValidForSelection() const
    {
        return true;
    }

    virtual bool isAllowedToNavigateInside(const QModelIndex& index);
    virtual bool isDownloadAllowed() const;
    virtual void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg);
    virtual bool okButtonEnabled(const QModelIndexList& selected);
    virtual QString okButtonText() const;

    virtual bool hasNewFolderButton() const
    {
        return false;
    }

    virtual bool acceptDrops(int tabItem)
    {
        return false;
    }

    virtual bool showEmptyStateUploadButton(NodeSelectorTreeViewWidget* wdg) const;
    virtual bool showEmptyStateNewFolderButton(NodeSelectorTreeViewWidget* wdg) const;
    virtual bool showEmptyStateAddBackupButton(NodeSelectorTreeViewWidget* wdg) const;

    virtual bool flattenSearchResults() const
    {
        return false;
    }

    virtual bool isFilePicker() const
    {
        return true;
    }

    virtual bool showsDestinationBreadcrumb() const
    {
        return false;
    }

    enum ButtonId : uint
    {
        NONE = 0x0,
        UPLOAD = 0x1,
        NEW_FOLDER = 0x2,
        CLEAR_RUBBISH = 0x4,
        ADD_BACKUP = 0x8
    };
    Q_DECLARE_FLAGS(ButtonIds, ButtonId)

    virtual void updateActionButtonsText(QMap<uint, QPushButton*> buttons);
    virtual QString getCustomButtonText(uint buttonId) const;
    void checkActionButtonsVisibility(NodeSelectorTreeViewWidget* wdg,
                                      QMap<uint, QPushButton*> buttons);
    void checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                     uint buttonId,
                                     QPushButton* button);
    virtual bool checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg, uint buttonId);

    virtual TabTypes allowedTabTypes() = 0;

    struct EmptyPageInfo
    {
        QString title;
        QString description;
        QIcon icon;
        ButtonIds buttons = ButtonId::NONE;
        bool hasBorder = false;

        bool isValid()
        {
            return !title.isEmpty() && !description.isEmpty() && !icon.isNull();
        }
    };

    virtual EmptyPageInfo getEmptyFolderPageInfo() const = 0;
    virtual EmptyPageInfo getEmptyCloudDrivePage() const = 0;

    virtual bool emptyPageHasButtons() const
    {
        return false;
    }

    virtual std::shared_ptr<NodeSelectorProxyModel> createProxyModel();

    // Header sections that show no label and ignore sort clicks. Default: none (every visible
    // column is interactive). NODE is always interactive.
    virtual QSet<int> nonInteractiveColumns() const;

protected:
    bool cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg);
};

Q_DECLARE_METATYPE(SelectType::ButtonIds)
Q_DECLARE_OPERATORS_FOR_FLAGS(SelectType::ButtonIds)

class CloudDriveType: public SelectType
{
public:
    explicit CloudDriveType() = default;

    bool areActionsAllowed() override
    {
        return true;
    }

    bool isDownloadAllowed() const override;

    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    bool checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg, uint buttonId) override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool acceptDrops(int tabItem) override;

    bool showEmptyStateUploadButton(NodeSelectorTreeViewWidget* wdg) const override;
    bool showEmptyStateAddBackupButton(NodeSelectorTreeViewWidget* wdg) const override;

    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;

    bool flattenSearchResults() const override
    {
        return false;
    }

    bool isFilePicker() const override
    {
        return false;
    }

    bool emptyPageHasButtons() const override
    {
        return true;
    }

    EmptyPageInfo getEmptyFolderPageInfo() const override;
    EmptyPageInfo getEmptyCloudDrivePage() const override;

private:
    QString getCustomButtonText(uint buttonId) const override;
};

class FilePickerType: public SelectType
{
public:
    FilePickerType() = default;

    bool checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg, uint buttonId) override;

    EmptyPageInfo getEmptyFolderPageInfo() const override;
    EmptyPageInfo getEmptyCloudDrivePage() const override;

    // In the file picker only NODE has a header label and is sortable.
    QSet<int> nonInteractiveColumns() const override;
};

class DownloadType: public FilePickerType
{
public:
    explicit DownloadType() = default;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    TabTypes allowedTabTypes() override;
    bool isDownloadAllowed() const override;
    QString okButtonText() const override;
};

class SyncType: public FilePickerType
{
public:
    explicit SyncType() = default;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    QString okButtonText() const override;
    EmptyPageInfo getEmptyFolderPageInfo() const override;
    std::shared_ptr<NodeSelectorProxyModel> createProxyModel() override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool showsDestinationBreadcrumb() const override
    {
        return true;
    }

    bool emptyPageHasButtons() const override
    {
        return true;
    }
};

class StreamType: public FilePickerType
{
public:
    explicit StreamType() = default;
    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;

    bool showsDestinationBreadcrumb() const override
    {
        return false;
    }

    bool isCurrentFolderValidForSelection() const override
    {
        return false;
    }
};

class UploadType: public FilePickerType
{
public:
    explicit UploadType() = default;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    QString okButtonText() const override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool showsDestinationBreadcrumb() const override
    {
        return true;
    }
};

class MoveBackupType: public FilePickerType
{
public:
    explicit MoveBackupType() = default;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    TabTypes allowedTabTypes() override;
    QString okButtonText() const override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool showsDestinationBreadcrumb() const override
    {
        return true;
    }
};

#endif // NODESELECTORSELECTTYPES_H
