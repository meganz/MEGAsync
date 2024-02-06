#ifndef QMLCOMPONENTWRAPPER_H
#define QMLCOMPONENTWRAPPER_H

#include "QmlDialog.h"
#include "megaapi.h"

#include <QQmlComponent>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QEvent>
#include <QPointer>
#include <QDialog>
#include <QApplication>

#include <memory>

#if DEBUG
#include <iostream>
#endif

class QMLComponent : public QObject
{
public:
    QMLComponent(QObject* parent = 0);
    ~QMLComponent();

    virtual QUrl getQmlUrl() = 0;
    virtual QString contextName(){return QString();}
    virtual QVector<QQmlContext::PropertyPair> contextProperties() {return QVector<QQmlContext::PropertyPair>();};

    QQmlEngine* getEngine();
};

class QmlDialogWrapperBase : public QWidget
{
    Q_OBJECT
public:
    QmlDialogWrapperBase(QWidget *parent = 0);
    ~QmlDialogWrapperBase();

    Qt::WindowModality windowModality();
    void setWindowModality(Qt::WindowModality modality);
    Qt::WindowFlags windowFlags();
    void setWindowFlags(Qt::WindowFlags flags);
    void setWindowState(Qt::WindowState state);
    void move(const QPoint& point);
    void showMaximized();
    void showNormal();
    void setGeometry(const QRect &geometry);
    QRect geometry();
    bool isMaximized();
    bool isMinimized();
    bool isVisible();
    void hide();
    void show();
    void close();
    void activateWindow();
    QWindow* window();
    void raise();
    void removeDialog();
    int minimumWidth();
    int maximumWidth();
    int maximumHeight();
    int minimumHeight();
    QRect rect();
    void update(const QRect& rect);
    void resize(int h, int w);
    void resize(const QSize& size);
    QSize size();

    Q_INVOKABLE int result();
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();

signals:
    void finished(int result);
    void requestClose();
    void accepted();
    void rejected();

public slots:
    void onWindowFinished();

protected:
    QPointer<QmlDialog> mWindow;

private:
    QDialog::DialogCode mResult;
};


template <class Type>
class QmlDialogWrapper : public QmlDialogWrapperBase
{

public:
    QmlDialogWrapper(QWidget* parent = nullptr)
        : QmlDialogWrapperBase(parent)
    {
        Q_ASSERT((std::is_base_of<QMLComponent, Type>::value));
        QObject::connect(mWrapper->getEngine(), &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
                    for (const QQmlError& e : warnings) {
                        qDebug() << "error: " << e.toString();
                    }
                });

        mWrapper = new Type(parent);
        QQmlEngine* engine = mWrapper->getEngine();
        QQmlComponent qmlComponent(engine);
        qmlComponent.loadUrl(mWrapper->getQmlUrl());

        if (qmlComponent.isReady())
        {
            QQmlContext *context = new QQmlContext(engine->rootContext(), this);
            if(!mWrapper->contextName().isEmpty())
            {
                context->setContextProperty(mWrapper->contextName(), mWrapper);
            }
            auto propertyList = mWrapper->contextProperties();
            if(!propertyList.isEmpty())
            {
                context->setContextProperties(propertyList);
            }
            mWindow = dynamic_cast<QmlDialog*>(qmlComponent.create(context));
            Q_ASSERT(mWindow);
            connect(mWindow, &QmlDialog::finished, this, [this](){
                mWrapper->deleteLater();
                QmlDialogWrapperBase::onWindowFinished();
            });

            connect(mWindow, &QQuickWindow::screenChanged, this, [this](){
                QApplication::postEvent(this, new QEvent(QEvent::ScreenChangeInternal));
            });

            QApplication::postEvent(this, new QEvent(QEvent::ScreenChangeInternal));
        }
        else
        {
            /*
            * Errors will be printed respecting the original format (with links to source qml that fails).
            * All errors will be printed, using qDebug() some errors were hidden.
            */
            ::mega::MegaApi::log(::mega::MegaApi::LOG_LEVEL_ERROR, qmlComponent.errorString().toStdString().c_str());
        }
    }

    ~QmlDialogWrapper(){
        if(mWrapper && !mWrapper->parent())
        {
            mWrapper->deleteLater();
        }
    }

    Type* wrapper(){ return mWrapper;}

private:
    QPointer<Type> mWrapper;
};

#endif // QMLCOMPONENTWRAPPER_H
