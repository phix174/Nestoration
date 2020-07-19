import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("Scroll")
    property int noteHeight: 5
    property int noteSpacing: if (noteHeight > 6) { 1 } else { 0 }
    readonly property int sHAPE_NONE: 0
    readonly property int sHAPE_SQUARE_EIGHTH: 1
    readonly property int sHAPE_SQUARE_QUARTER: 2
    readonly property int sHAPE_SQUARE_HALF: 3
    readonly property int sHAPE_SQUARE_THREEQUARTERS: 4
    readonly property int sHAPE_TRIANGLE: 5
    readonly property int sHAPE_IRREGULAR: 6
    readonly property int sHAPE_FIXED: 7
    function shape_is_square(shape) {
        return (
            shape === sHAPE_SQUARE_EIGHTH ||
            shape === sHAPE_SQUARE_QUARTER ||
            shape === sHAPE_SQUARE_HALF ||
            shape === sHAPE_SQUARE_THREEQUARTERS
        )
    }

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
                    toneviewer2.fudgeTimer.running = false
                    openClicked()
                    toneviewer0.reset()
                    toneviewer1.reset()
                    toneviewer2.reset()
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
                    toneviewer2.fudgeTimer.running = !toneviewer2.fudgeTimer.running;
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
            ToneViewer {
                id: toneviewer2
                property variant mainrepeater_model: channel2
            }
        }
    }
}
