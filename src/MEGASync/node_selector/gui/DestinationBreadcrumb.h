#ifndef DESTINATIONBREADCRUMB_H
#define DESTINATIONBREADCRUMB_H

#include "NodeSelectorBreadcrumbSegment.h"

#include <QFrame>
#include <QList>

namespace Ui
{
class DestinationBreadcrumb;
}

class DestinationBreadcrumb: public QFrame
{
    Q_OBJECT

public:
    explicit DestinationBreadcrumb(QWidget* parent = nullptr);
    ~DestinationBreadcrumb() override;

    void setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments);
    void setTitleText(const QString& text);
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption() const;

signals:
    void clearRequested();
    void refreshNeeded();

public slots:
    void onNodesRenamed(const QList<mega::MegaHandle>& handles);

private:
    void updateContentVisibility();

    Ui::DestinationBreadcrumb* ui;
    bool mShouldShowDefaultUploadOption = false;
    bool mHasClearablePath = false;
};

#endif // DESTINATIONBREADCRUMB_H
