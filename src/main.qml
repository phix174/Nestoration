import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    property string open_file_name: qsTr('')
    property int open_file_track: -1
    title: qsTr("NestorQtion") + (open_file_name ? " - " + open_file_name + (open_file_track > -1 ? " #" + (open_file_track + 1) : "") : "")
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

    Connections {
        target: audiofile
        onFileOpened: {
            controls.visible = true
            global_xScale = Qt.binding(function() { return toneviewer0.scroller_width / toneviewer0.mainrow_width });
            global_scrollbar.position = 0;
            root.open_file_name = file_name;
            root.open_file_track = file_track;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#444444"

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: tools
                Layout.fillWidth: true
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
                            player.pause();
                            audiofile.openClicked();
                        }
                    }

                    Row {
                        id: controls
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8
                        visible: false

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
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Column {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 8
                    spacing: 16

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
                ScrollBar {
                    id: global_scrollbar
                    orientation: Qt.Horizontal
                    policy: ScrollBar.AlwaysOn
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                }
                Rectangle {
                    id: info_pane
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    color: "#445566"

                    Frame {
                        anchors.fill: parent
                        anchors.margins: 8

                        Row {
                            spacing: 16
                            GridLayout {
                                columns: 2

                                Label { text: "NES Timer:"; color: "#ffffff" }
                                TextInput { id: input_nes_timer; color: "#ffffff"; readOnly: true }
                                Label { text: "Note:"; color: "#ffffff" }
                                TextInput { id: input_name; color: "#ffffff"; readOnly: true }
                                Label { text: "MIDI Note:"; color: "#ffffff" }
                                TextInput { id: input_semitone_id; color: "#ffffff" }
                            }
                            GridLayout {
                                columns: 2

                                Label { text: "Start:"; color: "#ffffff" }
                                TextInput { id: input_start; color: "#ffffff" }
                                Label { text: "Length:"; color: "#ffffff" }
                                TextInput { id: input_length; color: "#ffffff" }
                                Label { text: "Shape:"; color: "#ffffff" }
                                TextInput { id: input_shape; color: "#ffffff" }
                            }
                            GridLayout {
                                columns: 2

                                Label { text: "Volume:"; color: "#ffffff" }
                                TextInput { id: input_volume; color: "#ffffff" }
                                Label { text: "NES Timer End:"; color: "#ffffff" }
                                TextInput { id: input_nes_timer_end; color: "#ffffff"; readOnly: true }
                            }
                        }
                    }
                }
            }
        }
    }
}
