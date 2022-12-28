#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "ButtonIconManager.h"

#include <QFrame>
#include <QIcon>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace Ui {
class SearchLineEdit;
}

class ButtonIconManager;
class SearchLineEdit : public QFrame
{
    Q_OBJECT
public:
    explicit SearchLineEdit(QWidget *parent = nullptr);
    ~SearchLineEdit();
    void setIcon(const QIcon& icon);
    void setText(const QString& text);

protected:
    bool eventFilter(QObject* obj, QEvent* evnt) override;

signals:
    void search(const QString& text);

public slots:
    void onClearClicked();

private slots:
    void onTextChanged(const QString& text);
    void animationFinished();

private:
    void makeEffect(bool fadeIn);
    Ui::SearchLineEdit *ui;
    ButtonIconManager mButtonManager;
    QString mOldString;
};

#endif // SEARCHLINEEDIT_H
