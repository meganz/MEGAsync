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


signals:

private slots:
    void onClearClicked();
    void onTextChanged(const QString& text);
    void animationFinished();

private:
    void makeEffect(bool fadeIn);
    Ui::SearchLineEdit *ui;
    //QGraphicsOpacityEffect * mEffect;
    //QPropertyAnimation *mAnimation;
    ButtonIconManager mButtonManager;
};

#endif // SEARCHLINEEDIT_H
