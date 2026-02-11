import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: settingsPage

    signal back()

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
            anchors.leftMargin: 8
            anchors.rightMargin: 20

            ToolButton {
                implicitWidth: 44
                implicitHeight: 44
                text: "←"
                font.pixelSize: 22

                onClicked: settingsPage.back()

                background: Rectangle {
                    color: parent.pressed ? "#2a2a4a" : (parent.hovered ? "#1a1a2e" : "transparent")
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Label {
                text: qsTr("Settings")
                font.pixelSize: 20
                font.weight: Font.Bold
                color: "#ffffff"
            }

            Item { Layout.fillWidth: true }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 4 }

            // Server settings
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                height: serverColumn.height + 48
                radius: 16
                color: "#1a1a2e"

                ColumnLayout {
                    id: serverColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 24
                    spacing: 16

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 10
                            color: "#2a2a4a"

                            Label {
                                anchors.centerIn: parent
                                text: "🌐"
                                font.pixelSize: 18
                            }
                        }

                        Label {
                            text: qsTr("Server")
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Server URL")
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            color: "#888899"
                        }

                        TextField {
                            id: serverUrlField
                            Layout.fillWidth: true
                            text: configManager.serverUrl
                            placeholderText: qsTr("https://vpn.example.com")

                            background: Rectangle {
                                color: "#0f0f1a"
                                radius: 10
                                border.color: serverUrlField.activeFocus ? "#e94560" : "#2a2a4a"
                            }

                            color: "#ffffff"
                            placeholderTextColor: "#555566"
                            font.pixelSize: 14
                            font.family: "monospace"
                            padding: 12

                            onEditingFinished: {
                                configManager.serverUrl = text
                            }
                        }
                    }
                }
            }

            // About section
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                height: aboutColumn.height + 48
                radius: 16
                color: "#1a1a2e"

                ColumnLayout {
                    id: aboutColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 24
                    spacing: 16

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 10
                            color: "#2a2a4a"

                            Label {
                                anchors.centerIn: parent
                                text: "ℹ"
                                font.pixelSize: 18
                            }
                        }

                        Label {
                            text: qsTr("About")
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: qsTr("Version")
                                font.pixelSize: 14
                                color: "#888899"
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: Qt.application.version || "1.0.0"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#2a2a4a"
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: qsTr("Protocol")
                                font.pixelSize: 14
                                color: "#888899"
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: "WireGuard"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                        }
                    }
                }
            }

            // Security info
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                height: securityColumn.height + 48
                radius: 16
                color: "#1a1a2e"

                ColumnLayout {
                    id: securityColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 24
                    spacing: 16

                    RowLayout {
                        spacing: 12

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 10
                            color: "#1a3a2a"

                            Label {
                                anchors.centerIn: parent
                                text: "🔐"
                                font.pixelSize: 18
                            }
                        }

                        Label {
                            text: qsTr("Security")
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            color: "#ffffff"
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Your private keys are generated locally on this device and never transmitted to the server. Only public keys are shared for establishing secure connections.")
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                        lineHeight: 1.4
                        color: "#888899"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#2a2a4a"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: "#4ade80"
                        }

                        Label {
                            text: qsTr("End-to-end encrypted")
                            color: "#4ade80"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 20 }
        }
    }
}
