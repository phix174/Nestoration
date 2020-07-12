import QtQuick 2.0
import QtQuick.Controls 2.5

Item {
    id: toneViewer
    width: parent.width;
    height: noteHeight * tone_count
    property int extra_tones: 3
    property int tone_count: highestTone - lowestTone + 2 * extra_tones
    property int paddedHighestTone: highestTone + extra_tones
    property alias fudgeTimer: fudgeTimer
    function reset() {
        scroller.ScrollBar.horizontal.position = 0;
        itemsScale.xScale = Qt.binding(function() { return scroller.width / mainrow.width });
    }

    Row {
        anchors.fill: parent;
        spacing: 1

        PianoBackground {
            hasLabels: true
            width: 32
        }
        Item {
            width: parent.width - 32 - parent.spacing
            height: noteHeight * tone_count

            PianoBackground {
                width: parent.width
            }
            ScrollView {
                id: scroller
                anchors.fill: parent;
                implicitWidth: parent.width;
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                contentHeight: tone_count * noteHeight
                contentWidth: mainrow.width * itemsScale.xScale

                MouseArea {
                    height: parent.height
                    width: Math.max(mainrow.width, scroller.width)
                    onWheel: {
                        var position_old = scroller.ScrollBar.horizontal.position;
                        var scale_factor = 1.2;
                        var bound_again = false;
                        if (wheel.angleDelta.y < 0) {
                            if (itemsScale.xScale / scale_factor > scroller.width / mainrow.width) {
                                // If zooming out wouldn't zoom out too far, go ahead and do it.
                                scale_factor = 1 / scale_factor;
                            } else {
                                // Otherwise, only zoom out as much as needed to fit the whole file.
                                toneViewer.reset();
                                bound_again = true;
                            }
                        } else if (itemsScale.xScale * scale_factor > 1) {
                            // If zooming in would zoom in too far, only zoom in to 1:1.
                            scale_factor = 1 / itemsScale.xScale;
                        }
                        if (!bound_again) {
                            var mouse_x = wheel.x / (mainrow.width * itemsScale.xScale);
                            var position_new = Math.max(0, position_old + (mouse_x - position_old) * (1 - 1 / scale_factor));
                            itemsScale.xScale *= scale_factor;
                            if (position_new + scroller.ScrollBar.horizontal.size > 1) {
                                position_new = Math.max(0, 1 - scroller.ScrollBar.horizontal.size);
                            }
                            scroller.ScrollBar.horizontal.position = position_new;
                        }
                    }
                    onClicked: {
                        var mouse_x = mouse.x / itemsScale.xScale;
                        playerSeek(mouse_x);
                    }
                }

                Item {
                    height: tone_count * noteHeight
                    width: mainrow.width
                    transform: Scale {
                        id: itemsScale
                        xScale: scroller.width / mainrow.width
                    }

                    Rectangle {
                        property real fudge: 0
                        height: parent.height
                        width: 1 / itemsScale.xScale
                        x: playerPosition * (1789773.0 / 48000.0) / 4.0 - 152727.0 + fudge
                        color: "yellow"
                        visible: true
                        readonly property int myPosition: playerPosition
                        onMyPositionChanged: fudge = 0

                        Timer {
                            id: fudgeTimer
                            repeat: true
                            running: false
                            interval: 1000.0 / 50.0
                            onTriggered: parent.fudge += 1789773.0 / 50.0
                        }
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
}
