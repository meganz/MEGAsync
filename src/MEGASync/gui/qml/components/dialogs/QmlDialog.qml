import QtQml 2.12
import QtQuick.Window 2.12
import com.qmldialog 1.0 as Cpp

Cpp.QmlDialog {
    id: dialog

    function accept(){
        Wrapper.accept()
    }

    function reject(){
        Wrapper.reject()
    }
}

