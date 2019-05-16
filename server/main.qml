import QtQuick 2.12
import QtQuick.Controls 2.5
import QtWebSockets 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Server")

    function appendMessage(message) {
        messageBox.text += "\n" + message
    }

    Image {
        id: image

        anchors.fill: parent

        property string imageData: ""

        source: "data:image/png;base64," + imageData

        visible: imageData.length > 0
    }

    WebSocketServer {
        id: server
        listen: true
        host: "192.168.1.7"
        port: 1234
        onClientConnected: {
            webSocket.onTextMessageReceived.connect(function(message) {
                image.imageData = message;
            });
        }
        onErrorStringChanged: {
            appendMessage(qsTr("Server error: %1").arg(errorString));
        }
    }


    Text {
        id: messageBox
        text: qsTr("Click to send a message!")
        anchors.fill: parent
    }
}
