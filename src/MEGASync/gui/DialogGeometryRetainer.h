#ifndef DIALOGGEOMETRYRETAINER_H
#define DIALOGGEOMETRYRETAINER_H

#include <QString>
#include <QDialog>
#include <QPointer>
#include <QEvent>

#include <type_traits>

class DialogGeometryRetainerBase : public QObject
{
    Q_OBJECT

public:
    DialogGeometryRetainerBase(){}

protected:
    bool eventFilter(QObject* watched, QEvent *event) override
    {
        if(event->type() == QEvent::Close)
        {
             onDialogClosed();
        }

        return QObject::eventFilter(watched, event);
    }

protected:
    virtual void onDialogClosed() = 0;
};

template <class DialogClass>
class DialogGeometryRetainer : DialogGeometryRetainerBase
{
public:
    DialogGeometryRetainer() : mMaximized(false){}

    void showDialog(DialogClass* dialog)
    {
        auto isBaseOfDialog(std::is_base_of<QWidget, DialogClass>::value);
        Q_ASSERT(isBaseOfDialog);

        if(mDialog)
        {
            Q_ASSERT(mDialog == dialog);
        }

        mDialog = dialog;

        //First time this is used
        if(mDialogClassName.isEmpty() || mDialogGeometry.isEmpty())
        {
            mDialog->showNormal();
            mDialogClassName = QString::fromUtf8(typeid(DialogClass).name());
        }
        else
        {
            auto classType = QString::fromUtf8(typeid(DialogClass).name());
            Q_ASSERT(classType == mDialogClassName);

            if(mMaximized)
            {
                mDialog->showMaximized();
            }
            else
            {
                mDialog->setGeometry(mDialogGeometry);
                mDialog->showNormal();
            }
        }

        mDialog->activateWindow();
        mDialog->raise();

        mDialog->installEventFilter(this);
    }

protected:
    void onDialogClosed() override
    {
        if(mDialog)
        {
            mMaximized = mDialog->isMaximized();
            if(!mMaximized)
            {
                mDialogGeometry = mDialog->geometry();
            }
        }
    }

private:
    bool mMaximized;
    QRect mDialogGeometry;
    QString mDialogClassName;

    QPointer<DialogClass> mDialog;
};

#endif // DIALOGGEOMETRYRETAINER_H
