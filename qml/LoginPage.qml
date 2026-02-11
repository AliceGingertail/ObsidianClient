import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: loginPage

    signal loginSuccess()

    background: Rectangle {
        color: "#0f0f1a"
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(80, parent.height * 0.15)
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 32
                Layout.rightMargin: 32
                Layout.maximumWidth: 400
                Layout.alignment: Qt.AlignHCenter
                spacing: 24

                // Logo
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 88
                    height: 88
                    radius: 44
                    color: "#1a1a2e"
                    border.color: "#e94560"
                    border.width: 2

                    Label {
                        anchors.centerIn: parent
                        text: "◈"
                        font.pixelSize: 42
                        color: "#e94560"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Obsidian VPN")
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        color: "#ffffff"
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Secure & Private Connection")
                        font.pixelSize: 14
                        color: "#666680"
                    }
                }

                Item { Layout.preferredHeight: 24 }

                // Username field
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Username")
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: "#888899"
                    }

                    TextField {
                        id: usernameField
                        Layout.fillWidth: true
                        text: configManager.lastUsername
                        placeholderText: qsTr("Enter your username")

                        background: Rectangle {
                            color: "#1a1a2e"
                            radius: 10
                            border.color: usernameField.activeFocus ? "#e94560" : "#2a2a4a"
                            border.width: usernameField.activeFocus ? 2 : 1
                        }

                        color: "#ffffff"
                        placeholderTextColor: "#555566"
                        font.pixelSize: 15
                        leftPadding: 16
                        rightPadding: 16
                        topPadding: 14
                        bottomPadding: 14
                    }
                }

                // Password field
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Password")
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        color: "#888899"
                    }

                    TextField {
                        id: passwordField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Enter your password")
                        echoMode: TextInput.Password

                        background: Rectangle {
                            color: "#1a1a2e"
                            radius: 10
                            border.color: passwordField.activeFocus ? "#e94560" : "#2a2a4a"
                            border.width: passwordField.activeFocus ? 2 : 1
                        }

                        color: "#ffffff"
                        placeholderTextColor: "#555566"
                        font.pixelSize: 15
                        leftPadding: 16
                        rightPadding: 16
                        topPadding: 14
                        bottomPadding: 14

                        onAccepted: if (loginButton.enabled) loginButton.clicked()
                    }
                }

                Item { Layout.preferredHeight: 8 }

                // Login button
                Button {
                    id: loginButton
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    text: apiClient.loading ? qsTr("Signing in...") : qsTr("Sign In")
                    enabled: usernameField.text.length > 0 && passwordField.text.length > 0 && !apiClient.loading

                    background: Rectangle {
                        color: loginButton.enabled ? (loginButton.pressed ? "#c73e54" : "#e94560") : "#333344"
                        radius: 10

                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    contentItem: Text {
                        text: loginButton.text
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                        color: loginButton.enabled ? "#ffffff" : "#666677"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        configManager.lastUsername = usernameField.text
                        apiClient.login(usernameField.text, passwordField.text)
                    }
                }

                // Register link
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    Layout.topMargin: 8

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 4

                        Label {
                            text: qsTr("Don't have an account?")
                            color: "#666680"
                            font.pixelSize: 14
                        }

                        Label {
                            text: qsTr("Register")
                            color: "#e94560"
                            font.pixelSize: 14
                            font.weight: Font.Medium

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: registerDialog.open()
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Loading overlay
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: apiClient.loading

        BusyIndicator {
            anchors.centerIn: parent
            running: parent.visible
        }
    }

    Connections {
        target: apiClient
        function onLoginSuccess(tokens) {
            loginPage.loginSuccess()
        }
        function onRegisterSuccess() {
            successDialog.text = qsTr("Registration successful! You can now sign in.")
            successDialog.open()
        }
    }

    Dialog {
        id: registerDialog
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 340)
        title: qsTr("Create Account")
        modal: true
        standardButtons: Dialog.Cancel

        background: Rectangle {
            color: "#1a1a2e"
            radius: 12
            border.color: "#2a2a4a"
            border.width: 1
        }

        header: Label {
            text: registerDialog.title
            font.pixelSize: 20
            font.weight: Font.Bold
            color: "#ffffff"
            padding: 24
            bottomPadding: 8
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            TextField {
                id: regUsernameField
                Layout.fillWidth: true
                placeholderText: qsTr("Username")

                background: Rectangle {
                    color: "#0f0f1a"
                    radius: 8
                    border.color: regUsernameField.activeFocus ? "#e94560" : "#2a2a4a"
                }

                color: "#ffffff"
                placeholderTextColor: "#555566"
                font.pixelSize: 14
                padding: 12
            }

            TextField {
                id: regPasswordField
                Layout.fillWidth: true
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password

                background: Rectangle {
                    color: "#0f0f1a"
                    radius: 8
                    border.color: regPasswordField.activeFocus ? "#e94560" : "#2a2a4a"
                }

                color: "#ffffff"
                placeholderTextColor: "#555566"
                font.pixelSize: 14
                padding: 12
            }

            TextField {
                id: regPasswordConfirmField
                Layout.fillWidth: true
                placeholderText: qsTr("Confirm Password")
                echoMode: TextInput.Password

                background: Rectangle {
                    color: "#0f0f1a"
                    radius: 8
                    border.color: regPasswordConfirmField.activeFocus ? "#e94560" : "#2a2a4a"
                }

                color: "#ffffff"
                placeholderTextColor: "#555566"
                font.pixelSize: 14
                padding: 12
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: qsTr("Create Account")
                enabled: regUsernameField.text.length >= 3 &&
                         regPasswordField.text.length >= 6 &&
                         regPasswordField.text === regPasswordConfirmField.text

                background: Rectangle {
                    color: parent.enabled ? "#e94560" : "#333344"
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font.pixelSize: 15
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    apiClient.registerUser(regUsernameField.text, regPasswordField.text)
                    registerDialog.close()
                }
            }
        }

        onOpened: {
            regUsernameField.text = ""
            regPasswordField.text = ""
            regPasswordConfirmField.text = ""
            regUsernameField.forceActiveFocus()
        }
    }

    Dialog {
        id: successDialog
        property alias text: successLabel.text

        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 300)
        title: qsTr("Success")
        modal: true
        standardButtons: Dialog.Ok

        background: Rectangle {
            color: "#1a1a2e"
            radius: 12
            border.color: "#2a2a4a"
        }

        header: Label {
            text: successDialog.title
            font.pixelSize: 18
            font.weight: Font.Bold
            color: "#4ade80"
            padding: 20
            bottomPadding: 0
        }

        Label {
            id: successLabel
            wrapMode: Text.WordWrap
            width: parent.width
            color: "#cccccc"
            font.pixelSize: 14
        }
    }
}
