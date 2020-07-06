import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("Scroll")
    property int noteHeight: 17

    Rectangle {
        id: tools
        width: parent.width
        height: 50
        color: "#444444"

        Row {
            anchors.fill: parent
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Open..."
                onClicked: {
                    openClicked()
                    toneviewer.mainrepeater_model = channel0
                    toneviewer.reset()
                }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Seek 0"
                onClicked: {
                    seekZero()
                }
            }
        }
    }
    ToneViewer {
        id: toneviewer
        y: tools.height
        property variant mainrepeater_model
    }
}
