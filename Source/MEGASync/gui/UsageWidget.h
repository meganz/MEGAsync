#ifndef USAGEWIDGET_H
#define USAGEWIDGET_H

#include <QWidget>

#define BLUE   "#2ba6de"
#define GREEN  "#13e03c"
#define YELLOW "#ffd300"
#define ORANGE "#f07800"
#define GREY   "#777777"

class UsageWidget : public QWidget
{
    Q_OBJECT

public:
    UsageWidget(QWidget *parent = 0);

    void setCloudStorage(int percentage);
    void setRubbishStorage(int percentage);
    void setInShareStorage(int percentage);
    void setInboxStorage(int percentage);
    void setCloudStorageLabel(QString amount);
    void setRubbishStorageLabel(QString amount);
    void setInboxStorageLabel(QString amount);
    void setInShareStorageLabel(QString amount);
    void setUsedStorageLabel(QString amount);
    void setAvailableStorageLabel(QString amount);
    void setMaxStorage(QString amount);
    void setOverQuotaReached(bool value);

    void clearAll();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:

    bool overquotaReached;
    int pCloud;
    int pRubbish;
    int pInbox;
    int pInShare;

    QString maxStorage;
    QString cloudLabel;
    QString rubbishLabel;
    QString inboxLabel;
    QString inShareLabel;
    QString usedLabel;
    QString availableLabel;
};

#endif // USAGEWIDGET_H
