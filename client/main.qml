import QtQuick 2.12
import QtQuick.Controls 2.5
import QtWebSockets 1.0
import QtMultimedia 5.12

ApplicationWindow {
    id: root

    visible: true
    width: 360
    height: 360

    function appendMessage(message) {
        console.log(message);
    }

    WebSocket {
        id: socket

        url: "ws://77.47.223.10:1234"
        active: true
        onTextMessageReceived: appendMessage(qsTr("Client received message: %1").arg(message))

        onStatusChanged: {
            if (socket.status == WebSocket.Error) {
                appendMessage(qsTr("Client error: %1").arg(socket.errorString));
            } else if (socket.status == WebSocket.Closed) {
                appendMessage(qsTr("Client socket closed."));
            }
        }
    }

    Camera {
        id: camera
        imageCapture {
            resolution: Qt.size(1280, 720)
            onImageCaptured: {
                socket.sendTextMessage(imageConvertor.toBase64(preview, camera.imageCapture.resolution));
            }
        }
    }

    VideoOutput {
        anchors.fill: parent

        source: camera

        MouseArea {
            anchors.fill: parent
            onClicked: {
                camera.imageCapture.capture();
            }
        }
    }
}
