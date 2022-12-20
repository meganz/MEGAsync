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

signals:
    void search(const QString& text);

private slots:
    void onClearClicked();
    void onTextChanged(const QString& text);
    void animationFinished();
    void onEditingFinieshed();

private:
    void makeEffect(bool fadeIn);
    Ui::SearchLineEdit *ui;
    ButtonIconManager mButtonManager;
    QString mOldString;
};

#endif // SEARCHLINEEDIT_H
