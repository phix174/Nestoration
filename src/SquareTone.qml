import QtQuick 2.0
import QtQuick.Shapes 1.12

Item {
    id: note_item
    y: if (audiofile.lowestTone <= model.semitone_id && model.semitone_id <= audiofile.highestTone && model.shape !== sHAPE_NONE) {
           (paddedHighestTone - model.semitone_id) * noteHeight
       } else {
           0
       }
    property real y2: if (y) {
        (paddedHighestTone - model.semitone_id_end) * noteHeight - y
    } else {
        0
    }
    z: if (model.shape === sHAPE_IRREGULAR || model.shape === sHAPE_FIXED) { 1 } else { 0 }
    width: model.length
    height: if (model.semitone_id > -999 && model.shape !== sHAPE_NONE) {
               noteHeight - noteSpacing;
           } else {
               mainrow.height;
           }
    property color shape_color: if (model.shape === sHAPE_NONE) {
                                    "#000000"
                                } else if (model.shape === sHAPE_SQUARE_EIGHTH) {
                                    "#00cc00"
                                } else if (model.shape === sHAPE_SQUARE_QUARTER) {
                                    "#66cc00"
                                } else if (model.shape === sHAPE_SQUARE_HALF) {
                                    "#00cc66"
                                } else if (model.shape === sHAPE_SQUARE_THREEQUARTERS) {
                                    "#66cc66"
                                } else if (model.shape === sHAPE_TRIANGLE) {
                                    "#00cc00"
                                } else if (model.shape === sHAPE_IRREGULAR) {
                                    "#cc0000"
                                } else if (model.shape === sHAPE_FIXED) {
                                    "#ff9900"
                                }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            input_nes_timer.text = model.nes_timer
            input_name.text = model.name
            input_semitone_id.text = Math.round(model.semitone_id * 1000) / 1000
            input_start.text = model.start
            input_length.text = model.length
            input_volume.text = model.volume
            input_nes_timer_end.text = model.nes_timer_end
            input_shape.text = [
                    "Flat",
                    "Square 1/8",
                    "Square 1/4",
                    "Square 1/2",
                    "Square 3/4",
                    "Triangle",
                    "Irregular",
                    "Fixed"][model.shape]
        }
    }
    Rectangle {
        width: 1 / itemsScale.xScale
        height: parent.height
        color: shape_color
        visible: if (model.shape === sHAPE_NONE) { false } else { true }
        opacity: 1;
    }
    Shape {
        opacity: (model.shape === sHAPE_IRREGULAR) ? 1 : (model.volume + 2) / 17.0;
        ShapePath {
            strokeWidth: 0
            strokeColor: "transparent"
            fillColor: shape_color
            startX: 0
            startY: 0

            PathLine { x: model.length; y: y2 }
            PathLine { x: model.length; y: y2 + note_item.height }
            PathLine { x: 0; y: note_item.height }
            PathLine { x: 0; y: 0 }
        }
    }
}
