#ifndef STALLEDISSUEACTIONTITLE_H
#define STALLEDISSUEACTIONTITLE_H

#include "megaapi.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMap>
#include <QPointer>
#include <QWidget>

#include <memory>

namespace Ui {
class StalledIssueActionTitle;
}

class StalledIssueActionTitle : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssueActionTitle(QWidget* parent = nullptr);
    ~StalledIssueActionTitle();

    void removeBackgroundColor();

    void setHTML(const QString& title, const QString& iconPath = QString());
    void setTitle(const QString& title, const QString& iconPath = QString());
    QString title() const;

    void setHyperLinkMode();

    void addActionButton(const QIcon& icon,
                         const QString& text,
                         int id,
                         bool mainButton,
                         const QString& type = QLatin1String("primary"));
    void setActionButtonVisibility(int id, bool state);

    virtual void showIcon();
    void setMessage(const QString& message,
                    const QString& pixmapName = QString(),
                    const QString& iconToken = QString(),
                    const QString& tooltip = QString());

    void setFailed(bool state, const QString& errorTooltip);
    void setDisable(bool state);

    bool isSolved() const;
    bool isFailed() const;

    void setIsCloud(bool state);

    void setInfo(const QString& newPath, mega::MegaHandle handle);
    void setHandle(mega::MegaHandle handle);

    void updateLastTimeModified(const QDateTime& time);
    void updateCreatedTime(const QDateTime& time);
    bool updateUser(const QString& user, bool show);
    bool updateVersionsCount(int versions);
    void updateSize(int64_t size);
    void updateCRC(const QString& fp);

    enum class AttributeType
    {
        LastModified = 0,
        CreatedTime,
        Size,
        User,
        Versions,
        CRC
    };
    void addExtraInfo(AttributeType type, const QString& title, const QString& info, int level);
    void hideAttribute(AttributeType type);

    void updateExtraInfoLayout();

    void updateSizeHints();

    void setIsFile(bool newIsFile);

signals:
    void actionClicked(int id);

protected:
    Ui::StalledIssueActionTitle* ui;
    bool mIsCloud;
    QString mPath;
    QString mIconPath;
    bool mIsFile;
    std::unique_ptr<mega::MegaNode> mNode;

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    bool isRawInfoVisible() const;
    void showAttribute(AttributeType type);
    void updateLabel(QLabel* label, const QString& text);
    void updateIcon();
    QMap<AttributeType, QPointer<QLabel>> mUpdateLabels;
    QMap<AttributeType, QPointer<QLabel>> mTitleLabels;
};

#endif // STALLEDISSUEACTIONTITLE_H
