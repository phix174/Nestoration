import QtQuick 2.0
import QtQuick.Controls 2.5

ScrollView {
    id: scroller
    width: parent.width
    height: parent.height - y
    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    property int tone_count: highestTone - lowestTone
    contentHeight: tone_count * noteHeight
    contentWidth: mainrow.width * itemsScale.xScale
    function reset() {
        scroller.ScrollBar.horizontal.position = 0;
        itemsScale.xScale = Qt.binding(function() { return root.width / mainrow.width });
    }

    Rectangle {
        height: parent.height
        width: Math.max(mainrow.width, root.width)
        color: "#444444"

        Column {
            anchors.fill: parent
            spacing: 1

            Repeater {
                model: tone_count
                delegate: Rectangle {
                    property int pitch_class: (highestTone - index) % 12
                    property int octave: Math.floor((highestTone - index) / 12)
                    height: noteHeight - 1
                    width: parent.width
                    color: { if ([1, 3, 6, 8, 10].includes(pitch_class)) { return "#555555" } else { return "#666666" } }
                    Text {
                        x: scroller.ScrollBar.horizontal.position * scroller.contentWidth
                        text: ["C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"][pitch_class] + octave
                        font.family: "monospace"
                        font.pointSize: 10
                        color: "#aaaaaa"
                    }
                }
            }
        }
    }

    MouseArea {
        height: parent.height
        width: Math.max(mainrow.width, root.width)
        onWheel: {
            var position_old = scroller.ScrollBar.horizontal.position;
            var scale_factor = 1.2;
            if (wheel.angleDelta.y < 0) {
                if (itemsScale.xScale / scale_factor > root.width / mainrow.width) {
                    // If zooming out wouldn't zoom out too far, go ahead and do it.
                    scale_factor = 1 / scale_factor;
                } else {
                    // Otherwise, only zoom out as much as needed to fit the whole file.
                    scale_factor = (root.width / mainrow.width) / itemsScale.xScale
                }
            } else if (itemsScale.xScale * scale_factor > 1) {
                // If zooming in would zoom in too far, only zoom in to 1:1.
                scale_factor = 1;
            }
            var mouse_x = wheel.x / (mainrow.width * itemsScale.xScale);
            var position_new = Math.max(0, position_old + (mouse_x - position_old) * (1 - 1 / scale_factor));
            itemsScale.xScale *= scale_factor;
            if (position_new + scroller.ScrollBar.horizontal.size > 1) {
                position_new = Math.max(0, 1 - scroller.ScrollBar.horizontal.size);
            }
            scroller.ScrollBar.horizontal.position = position_new;
        }
    }

    Row {
        id: mainrow
        height: 88 * noteHeight
        transform: Scale {
            id: itemsScale
            xScale: root.width / mainrow.width
        }
        Repeater {
            model: scroller.mainrepeater_model
            delegate: Rectangle {
                y: if (model.semitone_id > -999) {
                       (highestTone - model.semitone_id) * noteHeight
                   } else {
                       0
                   }
                width: model.length
                height: if (model.semitone_id > -999) {
                           noteHeight - 1;
                       } else {
                           mainrow.height;
                       }
                color: if (model.duty === 0) {
                           "#00cc00"
                       } else if (model.duty === 1) {
                           "#66cc00"
                       } else if (model.duty === 2) {
                           "#00cc66"
                       } else if (model.duty === 3) {
                           "#66cc66"
                       } else if (model.duty === 4) {
                           "#66000000"
                       } else if (model.duty === 5) {
                           "#990000"
                       }
                Rectangle {
                    y: 1
                    width: 1 / itemsScale.xScale
                    height: parent.height
                    color: parent.color
                }
            }
        }
    }
}
