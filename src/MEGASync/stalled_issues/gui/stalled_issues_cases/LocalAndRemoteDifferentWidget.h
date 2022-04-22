#ifndef LOCALANDREMOTEDIFFERENTWIDGET_H
#define LOCALANDREMOTEDIFFERENTWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"
#include "QTMegaRequestListener.h"

#include <QWidget>
#include <memory>

namespace Ui {
class LocalAndRemoteDifferentWidget;
}

class LocalAndRemoteDifferentWidget : public StalledIssueBaseDelegateWidget, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit LocalAndRemoteDifferentWidget(QWidget *parent = nullptr);
    ~LocalAndRemoteDifferentWidget();

    void refreshUi() override;

protected slots:
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e);

private slots:
    void onLocalButtonClicked();
    void onRemoteButtonClicked();

private:
    Ui::LocalAndRemoteDifferentWidget *ui;
    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    mega::MegaHandle mRemovedRemoteHandle;
};

#endif // LOCALANDREMOTEDIFFERENTWIDGET_H
