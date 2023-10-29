#ifndef DUPLICATEDUPLOADFILE_H
#define DUPLICATEDUPLOADFILE_H

#include <DuplicatedNodeDialogs/DuplicatedNodeItem.h>
#include <megaapi.h>

#include <QObject>

class DuplicatedNodeDialog;

class DuplicatedUploadBase : public QObject
{
    Q_OBJECT

public:
    DuplicatedUploadBase(){}
    virtual ~DuplicatedUploadBase(){}

    virtual void fillUi(DuplicatedNodeDialog* dialog, std::shared_ptr<DuplicatedNodeInfo> conflict) = 0;

    QString getHeader(bool isFile);
    QString getSkipText(bool isFile);

    QStringList& getCheckedNames();

signals:
    void selectionDone();

protected:
    void fillTitle();

    std::shared_ptr<DuplicatedNodeInfo> mUploadInfo;

protected slots:
    void onNodeItemSelected();

private:
    QStringList checkedNames;
};

class DuplicatedUploadFile : public DuplicatedUploadBase
{
    Q_OBJECT

public:
    explicit DuplicatedUploadFile(){}
    ~DuplicatedUploadFile(){}

    void fillUi(DuplicatedNodeDialog* dialog, std::shared_ptr<DuplicatedNodeInfo> conflict) override;
};

class DuplicatedUploadFolder : public DuplicatedUploadBase
{
    Q_OBJECT

public:
    explicit DuplicatedUploadFolder(){}
    ~DuplicatedUploadFolder(){}

    void fillUi(DuplicatedNodeDialog* dialog, std::shared_ptr<DuplicatedNodeInfo> conflict) override;
};


#endif // DUPLICATEDUPLOADFILEDIALOG_H
