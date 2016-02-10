#ifndef COCOAHELPBUTTON_H
#define COCOAHELPBUTTON_H

#include <QMacCocoaViewContainer>

#ifdef __OBJC__
    #define ADD_COCOA_NATIVE_REF(CocoaClass) \
        @class CocoaClass; \
        typedef CocoaClass* Native##CocoaClass##Ref
#else
    #define ADD_COCOA_NATIVE_REF(CocoaClass) typedef void* Native##CocoaClass##Ref
#endif

//ADD_COCOA_NATIVE_REF(NSButton);
class CocoaHelpButton: public QMacCocoaViewContainer
{
    Q_OBJECT
public:
    CocoaHelpButton(QWidget *pParent = 0);
    QSize sizeHint() const;
    void onClicked();
signals:
    void clicked();
private:
    void *m_pButton;
};

#endif // COCOAHELPBUTTON_H
