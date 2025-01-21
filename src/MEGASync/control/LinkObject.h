#ifndef LINKOBJECT_H
#define LINKOBJECT_H

#include "megaapi.h"
#include "SetTypes.h"

#include <QList>
#include <QObject>
#include <QString>

enum linkType { INVALID, NODE, SET };
enum linkStatus { LOADING, CORRECT, WARNING, FAILED };
const QString DEFAULT_STR = QString::fromUtf8("");

class LinkObject : public QObject
{
    Q_OBJECT

public:
    LinkObject(mega::MegaApi* megaApi = nullptr, MegaNodeSPtr node = nullptr, const QString& link = DEFAULT_STR);
    virtual ~LinkObject();

    const QString getDefaultName() const;

    virtual QString getLink() const;
    virtual void setName(const QString& name);
    virtual QString getName() const;
    virtual int64_t getSize() const;
    virtual linkType getLinkType() const;
    virtual void setLinkStatus(const linkStatus status);
    virtual linkStatus getLinkStatus() const;
    virtual MegaNodeSPtr getMegaNode() const;
    virtual void setSelected(bool selected = true);
    virtual bool isSelected() const;
    virtual void setImportNode(MegaNodeSPtr node);
    virtual MegaNodeSPtr getImportNode() const;
    virtual bool showFolderIcon() const;
    virtual bool readyForProcessing() const;
    virtual void reset();

protected:
    mega::MegaApi* mMegaApi;
    QString mLink;
    MegaNodeSPtr mNode;
    MegaNodeSPtr mImportNode;
    QString mName;
    bool mIsSelected;
    linkType mLinkType;
    linkStatus mLinkStatus;
    bool mShowFolderIcon;
};

class LinkInvalid : public LinkObject
{
public:
    LinkInvalid(mega::MegaApi* megaApi = nullptr, MegaNodeSPtr node = nullptr, const QString& link = DEFAULT_STR);
};

class LinkNode : public LinkObject
{
public:
    LinkNode(mega::MegaApi* megaApi = nullptr, MegaNodeSPtr node = nullptr, const QString& link = DEFAULT_STR);

    int64_t getSize() const override;
    bool readyForProcessing() const override;
    void setMegaNode(MegaNodeSPtr node);

private:
    void updateNodeDetails();
};

class LinkSet : public LinkObject
{
public:
    LinkSet(mega::MegaApi* megaApi, const AlbumCollection& set);

    linkStatus getLinkStatus() const override;
    bool readyForProcessing() const override;
    void reset() override;
    int64_t getSize() const override;

    AlbumCollection getSet() const;

private:
    AlbumCollection mSet;
};

#endif // LINKOBJECT_H
