#ifndef RENAMENODEDIALOG_H
#define RENAMENODEDIALOG_H

#include <NodeNameSetterDialog/NodeNameSetterDialog.h>
#include <megaapi.h>

#include <memory>

class RenameNodeDialog : public NodeNameSetterDialog
{
    Q_OBJECT

public:
    RenameNodeDialog(QWidget* parent);

protected:
    QString dialogText() override;
    QString lineEditText() = 0;
    NodeNameSetterDialog::LineEditSelection lineEditSelection() override;

    QString enterNewFileNameText() const;
    QString enterNewFolderNameText() const;

    virtual bool isFile() = 0;


};

class RenameRemoteNodeDialog : public RenameNodeDialog
{
    Q_OBJECT

public:
    RenameRemoteNodeDialog(const QString& nodeName, QWidget* parent);
    RenameRemoteNodeDialog(std::unique_ptr<mega::MegaNode>, QWidget* parent);

protected:
    void onDialogAccepted() override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    bool isFile() override;
    QString lineEditText();

private:
    std::unique_ptr<mega::MegaNode> mNodeToRename;
    QString mNodeName;
};

class RenameLocalNodeDialog : public RenameNodeDialog
{
    Q_OBJECT

public:
    RenameLocalNodeDialog(const QString& nodePath, QWidget* parent);

protected:
    void onDialogAccepted() override;
    bool isFile() override;
    QString lineEditText();

private:
    QString errorText(const QString &newFileName) const;

    QString mNodePath;
};

#endif // RENAMENODEDIALOG_H
