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

        url: "ws://192.168.1.7:1234"
        active: true
        onTextMessageReceived: {
            image.imageData = message;
        }

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

        viewfinder.resolution: Qt.size(1280, 720)

        imageCapture {
            resolution: Qt.size(1280, 720)
            onImageCaptured: {
                socket.active = true;
                socket.sendTextMessage(imageConvertor.toBase64(preview, camera.imageCapture.resolution));
            }
        }
    }

    VideoOutput {
        id: viewfinder

        anchors.fill: parent

        source: camera
        autoOrientation: true

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (camera.lockStatus == Camera.Unlocked)
                    camera.searchAndLock();
                else
                    camera.unlock();
            }
        }

        RoundButton {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: width / 4

            width: parent.width / 5
            height: width

            onClicked: {
                camera.imageCapture.capture();
            }
        }
    }

    Image {
        id: image

        anchors.fill: parent

        property string imageData: ""

        source: "data:image/png;base64," + imageData

        visible: imageData.length > 0

        MouseArea {
            anchors.fill: parent
            onClicked: {
                image.imageData = "";
            }
        }
    }
}
