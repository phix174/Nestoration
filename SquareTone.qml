import QtQuick 2.0

Rectangle {
    y: if (lowestTone <= model.semitone_id && model.semitone_id <= highestTone) {
           (paddedHighestTone - model.semitone_id) * noteHeight
       } else {
           0
       }
    width: model.length
    height: if (model.semitone_id > -999) {
               noteHeight - noteSpacing;
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
               "#22000000"
           } else if (model.duty === 5) {
               "#cc0000"
           } else if (model.duty === 6) {
               "#ff9900"
           }

    z: if (model.duty === 5 || model.duty === 6) { 1 } else { 0 }
    Rectangle {
        width: 1 / itemsScale.xScale
        height: parent.height
        color: parent.color
        visible: if (model.duty === 4) { false } else { true }
    }
}
