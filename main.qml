import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("NestorQtion")
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
    function zoomToneViewer(channel_i) {
        var viewers = [toneviewer0, toneviewer1, toneviewer2];
        var viewer_to_toggle = viewers[channel_i];
        var new_state = (viewer_to_toggle.state === "thumb" ? "full" : "thumb");
        var others_state= (new_state === "thumb" ? "thumb" : "");
        for (let viewer of viewers) {
            viewer.state = others_state;
        }
        viewer_to_toggle.state = new_state;
    }

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
                    player.stop()
                    audiofile.openClicked()
                    toneviewer0.state = "thumb"
                    toneviewer1.state = "thumb"
                    toneviewer2.state = "thumb"
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
