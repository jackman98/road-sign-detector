import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtWebSockets 1.0
import QtMultimedia 5.12

ApplicationWindow {
    id: root

    property var detectedSigns: []

    visible: true
    width: 360
    height: 640

    function appendMessage(message) {
        console.log(message);
    }

    WebSocket {
        id: socket

        url: "ws://10.42.0.1:1234"
        active: true
        onTextMessageReceived: {
            container.visible = true;

            if (message.search("signs:") !== -1) {
                var detectedSignsStr = message.replace("signs:", "");
                if (detectedSignsStr.length !== 0) {
                    detectedSigns = detectedSignsStr.split(",");
                }
            }
            else {
                image.imageData = message;
            }
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

    Item {
        id: container

        anchors.fill: parent

        visible: false

        Image {
            id: image

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }

            property string imageData: ""

            source: "data:image/png;base64," + imageData

            visible: imageData.length > 0

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    image.imageData = "";
                    container.visible = false;
                }
            }
        }


        ListView {
            id: listView

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            orientation: ListView.Horizontal

            height: detectedSigns.length > 0 ? (parent.width / 5) : 0

            model: detectedSigns

            delegate: Image {
                height: parent.height
                width: height

                source: "/road_signs/" + detectedSigns[index] + ".png"

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        listView.currentIndex = index;
                        contentDialog.open();
                    }
                }
            }
        }

        Dialog {
            id: contentDialog

            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: Math.min(parent.width, parent.height) / 4 * 3
            height: Math.max(parent.width, parent.height) / 4 * 3
            parent: Overlay.overlay

            modal: true

            Flickable {
                id: flickable
                clip: true
                anchors.fill: parent
                contentHeight: column.height

                Column {
                    id: column
                    spacing: 20
                    width: parent.width

                    Image {
                        id: logo
                        width: parent.width / 2
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        source: "/road_signs/" + detectedSigns[listView.currentIndex] + ".png"
                    }

                    Label {
                        width: parent.width
                        text: "Забороняється проїзд без зупинки перед розміткою 1.12 (стоп-лінія), а якщо вона відсутня — перед знаком. Необхідно дати дорогу транспортним засобам, що рухаються дорогою, яка перетинається, а за наявності таблички  — транспортним засобам, що рухаються головною дорогою, а також праворуч рівнозначною дорогою.

Знак встановлюється безпосередньо перед перехрестям або вузькою ділянкою дороги.

Якщо безпосередньо перед перехрестям встановлено знак, то йому повинен передувати знак  з додатковою табличкою .

Якщо знак встановлений перед залізничним переїздом, що не охороняється та не обладнаний світлофорною сигналізацією, водій повинен зупинитися перед стоп-лінією, а за її відсутності — перед цим знаком.
"
                        wrapMode: Label.Wrap
                    }
                }

                ScrollIndicator.vertical: ScrollIndicator {
                    parent: contentDialog.contentItem
                    anchors.top: flickable.top
                    anchors.bottom: flickable.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: -contentDialog.rightPadding + 1
                }
            }
        }
    }
}
