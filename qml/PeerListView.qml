import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: peerListView
    radius: 16
    color: "#1a1a2e"
    clip: true

    signal peerSelected(string peerId, string configPath)

    property string selectedPeerId: ""
    property string pendingPrivateKey: ""

    ListModel {
        id: peersModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 12

        // Header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            spacing: 10

            Label {
                text: qsTr("Devices")
                font.pixelSize: 16
                font.weight: Font.DemiBold
                color: "#ffffff"
            }

            Rectangle {
                visible: peersModel.count > 0
                width: 24
                height: 20
                radius: 10
                color: "#2a2a4a"

                Label {
                    anchors.centerIn: parent
                    text: peersModel.count
                    font.pixelSize: 11
                    color: "#888899"
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                visible: !hasCurrentDevice()
                implicitWidth: 36
                implicitHeight: 36
                text: "+"
                font.pixelSize: 20

                background: Rectangle {
                    color: parent.pressed ? "#3ca85a" : "#4ade80"
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: createPeerDialog.open()
            }
        }

        // List
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: peersModel

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                id: delegateRoot
                width: listView.width
                height: 68
                radius: 12

                readonly property bool isCurrentDevice: model.peerId === configManager.currentPeerId

                color: model.peerId === selectedPeerId ? "#253050" :
                       (isCurrentDevice && delegateMouseArea.containsMouse ? "#1f1f3a" : "#151528")
                border.color: model.peerId === selectedPeerId ? "#4ade80" : "transparent"
                border.width: model.peerId === selectedPeerId ? 2 : 0
                opacity: isCurrentDevice ? 1.0 : 0.6

                MouseArea {
                    id: delegateMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    z: 0
                    cursorShape: delegateRoot.isCurrentDevice ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (delegateRoot.isCurrentDevice)
                            selectPeer(model.peerId, model.deviceName)
                    }
                }

                RowLayout {
                    z: 1
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 10
                    spacing: 12

                    // Icon
                    Rectangle {
                        width: 42
                        height: 42
                        radius: 10
                        color: delegateRoot.isCurrentDevice
                               ? (model.peerId === selectedPeerId ? "#4ade80" : "#e94560")
                               : "#3a3a5a"

                        Label {
                            anchors.centerIn: parent
                            text: delegateRoot.isCurrentDevice ? "\uD83D\uDCBB" : "\uD83D\uDD12"
                            font.pixelSize: 18
                        }
                    }

                    // Info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        Label {
                            text: model.deviceName
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: delegateRoot.isCurrentDevice ? "#ffffff" : "#999999"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 6

                            Label {
                                text: delegateRoot.isCurrentDevice
                                      ? (model.ipAddress || "\u2014")
                                      : qsTr("Other device")
                                font.pixelSize: 12
                                color: "#666680"
                            }

                            Rectangle {
                                visible: model.isActive
                                width: 6
                                height: 6
                                radius: 3
                                color: "#4ade80"
                            }
                        }
                    }

                    // Delete — only for current device
                    ToolButton {
                        visible: delegateRoot.isCurrentDevice
                        implicitWidth: 32
                        implicitHeight: 32
                        text: "\u2715"
                        font.pixelSize: 12
                        opacity: delegateMouseArea.containsMouse || model.peerId === selectedPeerId ? 1 : 0

                        background: Rectangle {
                            color: parent.pressed ? "#ef4444" : "transparent"
                            radius: 6
                        }

                        contentItem: Text {
                            text: parent.text
                            color: parent.hovered ? "#ef4444" : "#666680"
                            font: parent.font
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            deleteConfirmDialog.peerId = model.peerId
                            deleteConfirmDialog.open()
                        }
                    }
                }
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                visible: peersModel.count === 0
                text: qsTr("No devices\nTap + to add")
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 14
                color: "#555566"
                lineHeight: 1.4
            }
        }
    }

    // Delete dialog
    Dialog {
        id: deleteConfirmDialog
        property string peerId: ""

        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 280)
        title: qsTr("Delete Device")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No

        background: Rectangle {
            color: "#1a1a2e"
            radius: 12
            border.color: "#2a2a4a"
        }

        Label {
            text: qsTr("Delete this device?")
            color: "#cccccc"
            font.pixelSize: 14
        }

        onAccepted: apiClient.deletePeer(peerId)
    }

    // Create dialog
    Dialog {
        id: createPeerDialog
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 300)
        title: qsTr("Add Device")
        modal: true
        standardButtons: Dialog.Cancel

        background: Rectangle {
            color: "#1a1a2e"
            radius: 12
            border.color: "#2a2a4a"
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            TextField {
                id: deviceNameField
                Layout.fillWidth: true
                placeholderText: qsTr("Device name")

                background: Rectangle {
                    color: "#0f0f1a"
                    radius: 8
                    border.color: deviceNameField.activeFocus ? "#e94560" : "#2a2a4a"
                }

                color: "#ffffff"
                placeholderTextColor: "#555566"
                font.pixelSize: 14
                padding: 12
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                text: apiClient.loading ? qsTr("Creating...") : qsTr("Add")
                enabled: deviceNameField.text.length > 0 && !apiClient.loading

                background: Rectangle {
                    color: parent.enabled ? "#4ade80" : "#2a2a4a"
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }

                onClicked: createPeer(deviceNameField.text)
            }
        }

        onOpened: {
            deviceNameField.text = ""
            deviceNameField.forceActiveFocus()
        }
    }

    Component.onCompleted: loadPeers()

    Connections {
        target: apiClient

        function onPeersLoaded(peerList) {
            peersModel.clear()
            for (var i = 0; i < peerList.length; i++) {
                var p = peerList[i]
                peersModel.append({
                    peerId: p.id,
                    deviceName: p.deviceName,
                    ipAddress: p.ipAddress,
                    isActive: p.isActive
                })
            }
            // Auto-select current device if it exists in the list
            var curId = configManager.currentPeerId
            if (curId.length > 0 && selectedPeerId === "") {
                for (var j = 0; j < peersModel.count; j++) {
                    if (peersModel.get(j).peerId === curId) {
                        selectPeer(curId, peersModel.get(j).deviceName)
                        break
                    }
                }
            }
        }

        function onPeerCreated(peer, config) {
            createPeerDialog.close()
            configManager.currentPeerId = peer.id
            if (pendingPrivateKey.length > 0) {
                apiClient.getPeerConfig(peer.id)
            }
            loadPeers()
        }

        function onPeerConfigLoaded(peerId, config) {
            if (pendingPrivateKey.length > 0) {
                configManager.saveWireGuardConfig(peerId, config, pendingPrivateKey)
                pendingPrivateKey = ""
            }
        }

        function onPeerDeleted(peerId) {
            configManager.deleteWireGuardConfig(peerId)
            if (configManager.currentPeerId === peerId) {
                configManager.currentPeerId = ""
            }
            if (selectedPeerId === peerId) {
                selectedPeerId = ""
                peerListView.peerSelected("", "")
            }
            loadPeers()
        }
    }

    function loadPeers() {
        apiClient.getPeers()
    }

    function createPeer(deviceName) {
        if (!keyGenerator.generateKeyPair()) {
            console.error("Failed to generate key pair")
            return
        }
        pendingPrivateKey = keyGenerator.privateKey()
        apiClient.createPeer(deviceName, keyGenerator.publicKey())
    }

    function selectPeer(peerId, deviceName) {
        selectedPeerId = peerId
        var configPath = configManager.configFilePath(peerId)
        peerListView.peerSelected(peerId, configPath)
    }

    function hasCurrentDevice() {
        var curId = configManager.currentPeerId
        if (curId.length === 0)
            return false
        for (var i = 0; i < peersModel.count; i++) {
            if (peersModel.get(i).peerId === curId)
                return true
        }
        return false
    }
}
