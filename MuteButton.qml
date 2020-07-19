import QtQuick 2.0
import QtQuick.Controls 2.5

Button {
    property bool muted: false
    onClicked: {
        player.toggle_mute(channel_i)
        muted = !muted
    }
    background: Rectangle {
        color: parent.muted ? "#ee3333" : "#886666"
    }
}
