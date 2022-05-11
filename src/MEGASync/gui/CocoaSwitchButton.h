#ifndef COCOASWITCHBUTTON_H
#define COCOASWITCHBUTTON_H

#include <QMacCocoaViewContainer>

#ifdef __OBJC__
    #define ADD_COCOA_NATIVE_REF(CocoaClass) \
        @class CocoaClass; \
        typedef CocoaClass* Native##CocoaClass##Ref
#else
    #define ADD_COCOA_NATIVE_REF(CocoaClass) typedef void* Native##CocoaClass##Ref
#endif

//ADD_COCOA_NATIVE_REF(NSWitch);
class CocoaSwitchButton: public QMacCocoaViewContainer
{
    Q_OBJECT

    Q_PROPERTY(bool checked MEMBER mChecked READ isChecked WRITE setChecked)
public:
    CocoaSwitchButton(QWidget *pParent = 0);
    QSize sizeHint() const;
    void onClicked();

    bool isChecked();
    void setChecked(bool state);

signals:
    void toggled(bool state);
private:
    void *m_pButton;
    bool mChecked;
};

#endif // COCOASWITCHBUTTON_H
