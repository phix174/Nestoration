import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Shapes 1.12

Column {
    Shape {
        id: adsr
        width: 400
        height: 100
        property real max_volume: height * (1 - env_volume.value)
        property real sustain_volume: height * (1 - env_volume.value * env_sustain.value)

        ShapePath {
            strokeColor: "#ffffff"
            strokeWidth: 2
            fillColor: "transparent"
            startX: 0; startY: adsr.height
            PathQuad { controlX: 0; controlY: adsr.max_volume;
                x: 100 * env_attack.value; y: adsr.max_volume }
            PathLine { x: 100 * env_attack.value + 100 * env_decay.value; y: adsr.sustain_volume }
            PathLine { x: 400 - 100 * env_release.value; y: adsr.sustain_volume; }
            PathQuad { controlX: 400; controlY: adsr.sustain_volume; x: 400; y: 100 }
        }
    }
    Slider { id: env_volume; from: 0; to: 1; value: 1 }
    Slider { id: env_attack; from: 0; to: 1 }
    Slider { id: env_decay; from: 0; to: 1 }
    Slider { id: env_sustain; from: 0; to: 1; value: 1 }
    Slider { id: env_release; from: 0; to: 1 }
}
