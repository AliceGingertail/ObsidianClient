import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Obsidian 1.0

Page {
    id: mainPage

    signal logout()
    signal openSettings()

    background: Rectangle {
        color: "#0f0f1a"
    }

    header: ToolBar {
        height: 64
        background: Rectangle {
            color: "#0f0f1a"
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#1a1a2e"
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            Label {
                text: qsTr("Obsidian VPN")
                font.pixelSize: 20
                font.weight: Font.Bold
                color: "#ffffff"
            }

            Item { Layout.fillWidth: true }

            ToolButton {
                implicitWidth: 44
                implicitHeight: 44
                text: "⚙"
                font.pixelSize: 20

                onClicked: mainPage.openSettings()

                background: Rectangle {
                    color: parent.pressed ? "#2a2a4a" : (parent.hovered ? "#1a1a2e" : "transparent")
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    color: "#888899"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            ToolButton {
                implicitWidth: 44
                implicitHeight: 44
                text: "⏻"
                font.pixelSize: 18

                onClicked: logoutConfirmDialog.open()

                background: Rectangle {
                    color: parent.pressed ? "#3a1a2a" : (parent.hovered ? "#2a1a2a" : "transparent")
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    color: "#e94560"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Connection status card
        ConnectionView {
            id: connectionView
            Layout.fillWidth: true
            Layout.preferredHeight: 220
        }

        // Peers list
        PeerListView {
            id: peerListView
            Layout.fillWidth: true
            Layout.fillHeight: true

            onPeerSelected: function(peerId, configPath) {
                connectionView.selectedPeerId = peerId
                connectionView.selectedConfigPath = configPath
            }
        }
    }

    Dialog {
        id: logoutConfirmDialog
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 300)
        title: qsTr("Sign Out")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No

        background: Rectangle {
            color: "#1a1a2e"
            radius: 12
            border.color: "#2a2a4a"
        }

        header: Label {
            text: logoutConfirmDialog.title
            font.pixelSize: 18
            font.weight: Font.Bold
            color: "#ffffff"
            padding: 20
            bottomPadding: 0
        }

        Label {
            text: qsTr("Are you sure you want to sign out?")
            color: "#cccccc"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: parent.width
        }

        onAccepted: {
            vpnConnection.disconnectVpn()
            mainPage.logout()
        }
    }
}
