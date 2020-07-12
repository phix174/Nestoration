import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("Scroll")
    property int noteHeight: 9
    property int noteSpacing: if (noteHeight > 6) { 1 } else { 0 }

    Rectangle {
        id: tools
        width: parent.width
        height: 50
        color: "#444444"

        Row {
            anchors.fill: parent
            spacing: 8

            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Open..."
                onClicked: {
                    toneviewer0.fudgeTimer.running = false
                    toneviewer1.fudgeTimer.running = false
                    openClicked()
                    toneviewer0.reset()
                    toneviewer1.reset()
                }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Seek 0"
                onClicked: {
                    playerSeek(0)
                }
            }

            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Play/Pause"
                onClicked: {
                    playPause()
                    toneviewer0.fudgeTimer.running = !toneviewer0.fudgeTimer.running;
                    toneviewer1.fudgeTimer.running = !toneviewer1.fudgeTimer.running;
                }
            }
        }
    }
    Rectangle {
        y: tools.height
        width: parent.width
        height: parent.height - y;
        color: "#444444";
        Column {
            anchors.fill: parent;
            spacing: 16

            ToneViewer {
                id: toneviewer0
                property variant mainrepeater_model: channel0
            }
            ToneViewer {
                id: toneviewer1
                property variant mainrepeater_model: channel1
            }
        }
    }
}
