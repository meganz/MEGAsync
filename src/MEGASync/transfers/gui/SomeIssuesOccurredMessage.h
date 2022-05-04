#ifndef SOMEISSUESOCCURREDMESSAGE_H
#define SOMEISSUESOCCURREDMESSAGE_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class SomeIssuesOccurredMessage;
}

class StalledIssuesDialog;

class SomeIssuesOccurredMessage : public QWidget
{
    Q_OBJECT

public:
    explicit SomeIssuesOccurredMessage(QWidget *parent = nullptr);
    ~SomeIssuesOccurredMessage();

private slots:
    void on_viewIssuesButton_clicked();

private:
    Ui::SomeIssuesOccurredMessage *ui;
    QPointer<StalledIssuesDialog> mStalledIssuesDialog;
    bool mFoundStalledIssues;
};

#endif // SOMEISSUESOCCURREDMESSAGE_H
