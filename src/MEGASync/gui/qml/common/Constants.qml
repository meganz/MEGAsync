pragma Singleton
import QtQuick 2.15

QtObject {

    enum MessageType {
        NONE = 0,
        ERROR,
        WARNING,
        SUCCESS,
        INFO
    }

}
