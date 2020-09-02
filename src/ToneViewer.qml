import QtQuick 2.0
import QtQuick.Controls 2.5

Item {
    id: toneViewer
    visible: false
    width: parent.width - parent.padding * 2;
    property int extra_tones: 1
    property int tone_count: audiofile.highestTone - audiofile.lowestTone + 2 * extra_tones
    property int thumbNoteHeight: (parent.height - 2 * parent.spacing) / (3 * tone_count)
    property int fullNoteHeight: parent.height / tone_count
    property int noteHeight: thumbNoteHeight
    property int noteSpacing: if (noteHeight > 6) { 1 } else { 0 }
    height: noteHeight * tone_count
    property int paddedHighestTone: audiofile.highestTone + extra_tones
    property alias scroller: scroller
    property alias mainrow_width: mainrow.width
    function zoom_in(scale_factor, center) {
        if (global_xScale * scale_factor > 1) {
            // If zooming in would zoom in too far, only zoom in to 1:1.
            scale_factor = 1 / global_xScale;
        }
        finalize_zoom(scale_factor, center);
    }
    function zoom_out(scale_factor, center) {
        if (global_xScale / scale_factor > scroller.width / mainrow.width) {
            // If zooming out wouldn't zoom out too far, go ahead and do it.
            scale_factor = 1 / scale_factor;
            finalize_zoom(scale_factor, center);
        } else {
            // Otherwise, only zoom out as much as needed to fit the whole file.
            global_xScale = Qt.binding(function() { return scroller.width / mainrow.width });
        }
    }
    function finalize_zoom(scale_factor, center) {
        var position_old = scroller.ScrollBar.horizontal.position;
        var position_new = Math.max(0, position_old + (center - position_old) * (1 - 1 / scale_factor));
        global_xScale *= scale_factor;
        if (position_new + scroller.ScrollBar.horizontal.size > 1) {
            position_new = Math.max(0, 1 - scroller.ScrollBar.horizontal.size);
        }
        global_scrollbar.position = position_new;
    }

    Connections {
        target: audiofile
        onFileOpened: toneViewer.state = "";
        onTrackOpened: toneViewer.state = "thumb";
    }

    Row {
        anchors.fill: parent;
        spacing: 1

        Column {
            width: 32
            spacing: 1

            MuteButton {
                width: parent.width
                property int channel_i: toneViewer.channel_i
                text: "M"
            }
            Button {
                width: parent.width
                text: "Z"
                onClicked: zoomToneViewer(toneViewer.channel_i)
                background: Rectangle {
                    color: "#777777"
                }
            }
        }

        PianoBackground {
            hasLabels: true
            width: 32
        }
        Item {
            width: parent.width - (32 + parent.spacing) * 2
            height: noteHeight * tone_count

            PianoBackground {
                width: parent.width
            }
            ScrollView {
                id: scroller
                anchors.fill: parent;
                implicitWidth: parent.width;
                clip: true
                ScrollBar.horizontal: global_scrollbar
                contentHeight: tone_count * noteHeight
                contentWidth: mainrow.width * global_xScale

                MouseArea {
                    id: viewerMouseArea
                    height: parent.height
                    width: Math.max(mainrow.width, scroller.width)
                    onWheel: {
                        var scale_factor = 1.2;
                        var center = wheel.x / (mainrow.width * global_xScale);
                        if (wheel.angleDelta.y < 0) {
                            zoom_out(scale_factor, center);
                        } else if (wheel.angleDelta.y > 0) {
                            zoom_in(scale_factor, center);
                        }
                    }
                    onPressed: {
                        var mouse_x = mouse.x / global_xScale;
                        player.seek(mouse_x);
                    }
                }

                Item {
                    height: tone_count * noteHeight
                    width: mainrow.width
                    transform: Scale {
                        id: itemsScale
                        xScale: global_xScale
                    }

                    Rectangle {
                        property real position_ratio: (1789773.0 / 48000.0) / 4.0
                        height: parent.height
                        width: 1 / global_xScale
                        x: player.position * position_ratio
                        color: "yellow"
                        visible: true
                    }

                    Row {
                        id: mainrow
                        height: parent.height

                        Repeater {
                            model: mainrepeater_model
                            delegate: SquareTone {}
                        }
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "thumb"
            PropertyChanges {
                target: toneViewer
                noteHeight: thumbNoteHeight
                visible: true
            }
        },
        State {
            name: "full"
            PropertyChanges {
                target: toneViewer
                noteHeight: fullNoteHeight
                visible: true
            }
        }
    ]
}
