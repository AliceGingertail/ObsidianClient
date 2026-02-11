import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Obsidian 1.0

ApplicationWindow {
    id: window
    visible: true
    width: Math.max(420, Screen.width * 0.25)
    height: Math.max(680, Screen.height * 0.7)
    minimumWidth: 380
    minimumHeight: 550
    title: qsTr("Obsidian VPN")

    color: "#0f0f1a"

    font.family: "Segoe UI, Roboto, sans-serif"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: loginPage//apiClient.isAuthenticated ? mainPage : serverPage

        pushEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
            PropertyAnimation { property: "x"; from: 50; to: 0; duration: 200; easing.type: Easing.OutCubic }
        }
        pushExit: Transition {
            PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
        }
        popEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
        }
        popExit: Transition {
            PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
            PropertyAnimation { property: "x"; from: 0; to: 50; duration: 200; easing.type: Easing.InCubic }
        }
    }

    Component {
        id: serverPage
        ServerPage {
            onContinueToLogin: {
                stackView.push(loginPage)
            }
        }
    }

    Component {
        id: loginPage
        LoginPage {
            onLoginSuccess: {
                stackView.replace(null, mainPage)
            }
            onOpenServerSettings: {
                stackView.pop()
            }
        }
    }

    Component {
        id: mainPage
        MainPage {
            onLogout: {
                apiClient.logout()
                stackView.replace(null, serverPage)
            }
            onOpenSettings: {
                stackView.push(settingsPage)
            }
        }
    }

    Component {
        id: settingsPage
        SettingsPage {
            onBack: {
                stackView.pop()
            }
        }
    }

    Connections {
        target: apiClient
        function onLoginError(error) {
            errorDialog.text = error
            errorDialog.open()
        }
        function onApiError(error) {
            errorDialog.text = error
            errorDialog.open()
        }
    }

    Dialog {
        id: errorDialog
        property alias text: errorLabel.text

        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 320)
        title: qsTr("Error")
        modal: true
        standardButtons: Dialog.Ok

        background: Rectangle {
            color: "#1a1a2e"
            radius: 12
            border.color: "#2a2a4a"
            border.width: 1
        }

        header: Label {
            text: errorDialog.title
            font.pixelSize: 18
            font.weight: Font.Bold
            color: "#e94560"
            padding: 20
            bottomPadding: 0
        }

        Label {
            id: errorLabel
            wrapMode: Text.WordWrap
            width: parent.width
            color: "#cccccc"
            font.pixelSize: 14
        }
    }
}
