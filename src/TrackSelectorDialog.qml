import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12

Dialog {
    anchors.centerIn: parent
    title: qsTr("Track Number and Length")
    modal: true
    standardButtons: Dialog.Open | Dialog.Cancel
    onAccepted: audiofile.select_track(track_combobox.currentIndex, Number(open_length.text));
    onRejected: console.log("Cancel")

    Column {
        width: parent.width
        spacing: 32

        Column {
            width: parent.width

            Text {
                text: qsTr("File:")
                font.bold: true
            }

            Text {
                text: open_file_name || qsTr("(no file is currently open)")
            }
        }

        Column {
            width: parent.width

            Text {
                text: qsTr("Track:")
                font.bold: true
            }

            ComboBox {
                id: track_combobox
                width: parent.width
                model: open_tracks

                onCurrentIndexChanged: {
                    var length = open_track_lengths[currentIndex] / 1000.0;
                    open_length.text = length;
                    default_length.text = length + qsTr(" sec");
                }
            }
        }

        GridLayout {
            columns: 2
            columnSpacing: 32

            Text {
                Layout.preferredWidth: 160
                Layout.fillWidth: true
                text: qsTr("Length (sec):")
                font.bold: true
            }

            Text {
                Layout.preferredWidth: 160
                Layout.fillWidth: true
                text: qsTr("Default Length:")
                font.bold: true
            }

            TextField {
                id: open_length
            }

            Text {
                id: default_length
                text: "150"
            }
        }
    }
}
