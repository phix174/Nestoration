import QtQuick 2.0

Column {
    height: noteHeight * tone_count
    spacing: noteSpacing
    default property bool hasLabels: false

    Repeater {
        model: tone_count
        delegate: Rectangle {
            property int pitch_class: (paddedHighestTone - index) % 12
            property int octave: Math.floor((paddedHighestTone - index) / 12)
            height: noteHeight - noteSpacing
            width: parent.width
            color: { if ([1, 3, 6, 8, 10].includes(pitch_class)) { return "#555555" } else { return "#666666" } }
            Text {
                anchors.centerIn: parent
                text: ["C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"][pitch_class] + octave
                font.family: "monospace"
                font.pointSize: 6
                color: "#aaaaaa"
                visible: hasLabels
            }
        }
    }
}
