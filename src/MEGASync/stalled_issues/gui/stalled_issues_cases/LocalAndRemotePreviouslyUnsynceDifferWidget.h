#ifndef LOCALANDREMOTEPREVIOUSLYUNSYNCEDIFFERWIDGET_H
#define LOCALANDREMOTEPREVIOUSLYUNSYNCEDIFFERWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"
#include "QTMegaRequestListener.h"

#include <QWidget>
#include <memory>

namespace Ui {
class LocalAndRemotePreviouslyUnsynceDifferWidget;
}

class LocalAndRemotePreviouslyUnsynceDifferWidget : public StalledIssueBaseDelegateWidget, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit LocalAndRemotePreviouslyUnsynceDifferWidget(QWidget *parent = nullptr);
    ~LocalAndRemotePreviouslyUnsynceDifferWidget();

    void refreshUi() override;

protected slots:
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e);

private slots:
    void onLocalButtonClicked();
    void onRemoteButtonClicked();

private:
    Ui::LocalAndRemotePreviouslyUnsynceDifferWidget *ui;
    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    mega::MegaHandle mRemovedRemoteHandle;
};

#endif // LOCALANDREMOTEPREVIOUSLYUNSYNCEDIFFERWIDGET_H
