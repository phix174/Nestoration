import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("Scroll")
    property int noteHeight: 17
    property int noteCount: 50

    Rectangle {
        anchors.fill: parent
        color: "#444444"

        Column {
            anchors.fill: parent
            spacing: 1

            Repeater {
                model: noteCount
                delegate: Rectangle {
                    width: parent.width
                    height: noteHeight - 1
                    color: { if ([1, 3, 6, 8, 10].includes((index+2) % 12)) { return "#555555" } else { return "#666666" } }
                }
            }
        }
    }

    ScrollView {
        id: scroller
        width: parent.width
        height: noteCount * noteHeight
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        contentWidth: mainrow.width * itemsScale.xScale

        MouseArea {
            anchors.fill: parent
            onWheel: {
                var outside_old = Math.max(0, (mainrow.width - scroller.width / itemsScale.xScale) / mainrow.width);
                var scroll_old = 0;
                if (outside_old > 0) {
                    scroll_old = scroller.ScrollBar.horizontal.position / outside_old;
                }
                var leftEdge_old = outside_old * scroll_old;
                var rightEdge_old = leftEdge_old + (1 - outside_old);
                var scale_factor = 1.2;
                if (wheel.angleDelta.y < 0) {
                    if (itemsScale.xScale / scale_factor > root.width / mainrow.width){
                        scale_factor = 1 / scale_factor;
                    } else {
                        scale_factor = (root.width / mainrow.width) / itemsScale.xScale
                    }
                } else if (itemsScale.xScale * scale_factor > 1) {
                    scale_factor = 1;
                }
                var mouse_x = wheel.x / (mainrow.width * itemsScale.xScale);
                var leftEdge_new = Math.max(0, leftEdge_old + (mouse_x - leftEdge_old) * (1 - 1 / scale_factor));
                var rightEdge_new = Math.min(1, rightEdge_old - (rightEdge_old - mouse_x) * (1 - 1 / scale_factor));
                var denominator = leftEdge_new + (1 - rightEdge_new)
                var scroll_new = 0
                if (denominator > 0) {
                    scroll_new = leftEdge_new / denominator;
                }
                if (scale_factor > 0) {
                    itemsScale.xScale *= scale_factor;
                    var outside_new = Math.max(0, (mainrow.width - scroller.width / itemsScale.xScale) / mainrow.width);
                    scroller.ScrollBar.horizontal.position = scroll_new * outside_new;
                }
            }
        }
        Row {
            id: mainrow
            height: noteCount * noteHeight
            transform: Scale {
                id: itemsScale
                xScale: root.width / mainrow.width
            }
            Repeater {
                model: toneList
                delegate: Rectangle {
                    y: (86 - model.semitone_id) * noteHeight
                    width: model.length
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
