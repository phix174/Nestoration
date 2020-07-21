import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("NestorQtion")
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
    property real global_xScale: toneviewer0.scroller_width / toneviewer0.mainrow_width

    Rectangle {
        id: tools
        width: parent.width
        height: 50
        color: "#333333"

        Row {
            anchors.fill: parent
            padding: 8
            spacing: 8

            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Open..."
                onClicked: {
                    toneviewer0.fudgeTimer.running = false
                    toneviewer1.fudgeTimer.running = false
                    toneviewer2.fudgeTimer.running = false
                    player.stop()
                    audiofile.openClicked()
                    player.seek(0)
                    global_xScale = Qt.binding(function() { return toneviewer0.scroller_width / toneviewer0.mainrow_width });
                    global_scrollbar.position = 0;
                }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Seek 0"
                onClicked: {
                    player.seek(0)
                }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Play/Pause"
                onClicked: {
                    player.play_pause()
                    toneviewer0.fudgeTimer.running = !toneviewer0.fudgeTimer.running;
                    toneviewer1.fudgeTimer.running = !toneviewer1.fudgeTimer.running;
                    toneviewer2.fudgeTimer.running = !toneviewer2.fudgeTimer.running;
                }
            }
            MuteButton {
                property int channel_i: 3
                text: "Mute " + channel_i
                anchors.verticalCenter: parent.verticalCenter
            }
            MuteButton {
                property int channel_i: 4
                text: "Mute " + channel_i
                anchors.verticalCenter: parent.verticalCenter
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
            padding: 8
            topPadding: 16
            spacing: 16

            ScrollBar {
                id: global_scrollbar
                orientation: Qt.Horizontal
                policy: ScrollBar.AlwaysOn
                width: parent.width - parent.padding * 2
                height: 24
            }
            ToneViewer {
                id: toneviewer0
                property int channel_i: 0
                property variant mainrepeater_model: audiofile.channel0
            }
            ToneViewer {
                id: toneviewer1
                property int channel_i: 1
                property variant mainrepeater_model: audiofile.channel1
            }
            ToneViewer {
                id: toneviewer2
                property int channel_i: 2
                property variant mainrepeater_model: audiofile.channel2
            }
        }
    }
}
