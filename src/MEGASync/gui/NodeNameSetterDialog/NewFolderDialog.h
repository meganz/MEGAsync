#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

#include <NodeNameSetterDialog/NodeNameSetterDialog.h>
#include <megaapi.h>

#include <memory>

class NewFolderDialog : public NodeNameSetterDialog
{
    Q_OBJECT

public:
    NewFolderDialog(std::shared_ptr<mega::MegaNode> parentNode, QWidget* parent);
    ~NewFolderDialog() = default;

    std::unique_ptr<mega::MegaNode> getNewNode();

protected:
    void onDialogAccepted() override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    QString dialogText() override;
    void title() override;

private:
    std::unique_ptr<mega::MegaNode> mNewNode;
    std::shared_ptr<mega::MegaNode> mParentNode;
};

#endif // NEWFOLDERDIALOG_H
