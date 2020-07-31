import QtQuick 2.0

Rectangle {
    y: if (audiofile.lowestTone <= model.semitone_id && model.semitone_id <= audiofile.highestTone && model.shape !== sHAPE_NONE) {
           (paddedHighestTone - model.semitone_id) * noteHeight
       } else {
           0
       }
    width: model.length
    height: if (model.semitone_id > -999 && model.shape !== sHAPE_NONE) {
               noteHeight - noteSpacing;
           } else {
               mainrow.height;
           }
    color: if (model.shape === sHAPE_NONE) {
               "#22000000"
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
    z: if (model.shape === sHAPE_IRREGULAR || model.shape === sHAPE_FIXED) { 1 } else { 0 }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            input_nes_timer.text = model.nes_timer
            input_length.text = model.length
            input_name.text = model.name
            input_semitone_id.text = Math.round(model.semitone_id * 1000) / 1000
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
        color: parent.color
        visible: if (model.shape === sHAPE_NONE) { false } else { true }
    }
}
