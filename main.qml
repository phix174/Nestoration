import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("Scroll")
    property int noteHeight: 17

    ScrollView {
        id: scroller
        anchors.fill: parent
        //clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        contentHeight: notesRange.length * noteHeight
        contentWidth: mainrow.width * itemsScale.xScale

        Rectangle {
            height: parent.height
            width: mainrow.width
            color: "#444444"

            Column {
                anchors.fill: parent
                spacing: 1

                Repeater {
                    model: notesRange
                    delegate: Rectangle {
                        width: parent.width
                        height: noteHeight - 1
                        color: { if ([1, 3, 6, 8, 10].includes((modelData.semitone_id) % 12)) { return "#555555" } else { return "#666666" } }
                        Text {
                            x: scroller.ScrollBar.horizontal.position * scroller.contentWidth
                            text: modelData.name
                            font.family: "monospace"
                            font.pointSize: 10
                            color: "#aaaaaa"
                        }
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
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
            height: notesRange.length * noteHeight
            transform: Scale {
                id: itemsScale
                xScale: root.width / mainrow.width
            }
            Repeater {
                model: toneList
                delegate: Rectangle {
                    y: (highNote.semitone_id - modelData.semitone_id) * noteHeight
                    width: modelData.length
                    height: noteHeight - 1
                    color: "#00cc00"
                    Rectangle {
                        width: 1 / itemsScale.xScale
                        height: noteHeight - 1
                        color: "#00cc00"
                    }
                }
            }
        }
    }
}
