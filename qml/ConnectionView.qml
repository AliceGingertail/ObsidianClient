import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Obsidian 1.0

Rectangle {
    id: connectionView
    height: 220
    radius: 16
    color: "#1a1a2e"
    clip: true

    property string selectedPeerId: ""
    property string selectedConfigPath: ""

    // Gradient overlay for connected state
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        visible: vpnConnection.state === VpnConnection.Connected
        opacity: 0.1

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#4ade80" }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        // Status indicator
        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 72
            Layout.preferredHeight: 72

            Rectangle {
                id: statusCircle
                anchors.centerIn: parent
                width: 64
                height: 64
                radius: 32
                color: "transparent"
                border.color: getStatusColor()
                border.width: 3

                Behavior on border.color { ColorAnimation { duration: 300 } }

                Label {
                    anchors.centerIn: parent
                    text: getStatusIcon()
                    font.pixelSize: 28
                    color: getStatusColor()
                }

                // Pulse animation for connecting
                SequentialAnimation on scale {
                    running: vpnConnection.state === VpnConnection.Connecting ||
                             vpnConnection.state === VpnConnection.Disconnecting
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.05; duration: 600; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                }
            }

            // Rotating ring for loading states
            Rectangle {
                anchors.centerIn: parent
                width: 72
                height: 72
                radius: 36
                color: "transparent"
                border.color: getStatusColor()
                border.width: 2
                opacity: 0.3
                visible: vpnConnection.state === VpnConnection.Connecting ||
                         vpnConnection.state === VpnConnection.Disconnecting

                RotationAnimation on rotation {
                    running: parent.visible
                    from: 0
                    to: 360
                    duration: 2000
                    loops: Animation.Infinite
                }
            }
        }

        // Status text
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 2

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: getStatusText()
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: "#ffffff"
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                visible: vpnConnection.state === VpnConnection.Connected
                text: qsTr("Your connection is protected")
                font.pixelSize: 12
                color: "#4ade80"
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                visible: vpnConnection.state === VpnConnection.Disconnected && selectedConfigPath.length === 0
                text: qsTr("Select a device to connect")
                font.pixelSize: 12
                color: "#666680"
            }
        }

        Item { Layout.fillHeight: true }

        // Connect/Disconnect button
        Button {
            id: connectButton
            Layout.fillWidth: true
            Layout.preferredHeight: 48

            text: getButtonText()
            enabled: !isTransitioning() && (vpnConnection.state === VpnConnection.Connected ||
                                            selectedConfigPath.length > 0)

            background: Rectangle {
                color: {
                    if (!connectButton.enabled) return "#2a2a4a"
                    if (vpnConnection.state === VpnConnection.Connected) {
                        return connectButton.pressed ? "#c73e54" : "#e94560"
                    }
                    return connectButton.pressed ? "#3ca85a" : "#4ade80"
                }
                radius: 12

                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: Text {
                text: connectButton.text
                font.pixelSize: 16
                font.weight: Font.DemiBold
                color: connectButton.enabled ? "#ffffff" : "#555566"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                if (vpnConnection.state === VpnConnection.Connected) {
                    vpnConnection.disconnectVpn()
                } else if (selectedConfigPath.length > 0) {
                    vpnConnection.connectVpn(selectedConfigPath)
                }
            }
        }
    }

    function getStatusColor() {
        switch (vpnConnection.state) {
            case VpnConnection.Connected:
                return "#4ade80"
            case VpnConnection.Connecting:
            case VpnConnection.Disconnecting:
                return "#f59e0b"
            case VpnConnection.Error:
                return "#ef4444"
            default:
                return "#555566"
        }
    }

    function getStatusIcon() {
        switch (vpnConnection.state) {
            case VpnConnection.Connected:
                return "🔒"
            case VpnConnection.Connecting:
            case VpnConnection.Disconnecting:
                return "⏳"
            case VpnConnection.Error:
                return "⚠"
            default:
                return "🔓"
        }
    }

    function getStatusText() {
        switch (vpnConnection.state) {
            case VpnConnection.Connected:
                return qsTr("Connected")
            case VpnConnection.Connecting:
                return qsTr("Connecting...")
            case VpnConnection.Disconnecting:
                return qsTr("Disconnecting...")
            case VpnConnection.Error:
                return vpnConnection.errorMessage || qsTr("Connection Error")
            default:
                return qsTr("Not Connected")
        }
    }

    function getButtonText() {
        switch (vpnConnection.state) {
            case VpnConnection.Connected:
                return qsTr("Disconnect")
            case VpnConnection.Connecting:
                return qsTr("Connecting...")
            case VpnConnection.Disconnecting:
                return qsTr("Disconnecting...")
            default:
                return qsTr("Connect")
        }
    }

    function isTransitioning() {
        return vpnConnection.state === VpnConnection.Connecting ||
               vpnConnection.state === VpnConnection.Disconnecting
    }
}
