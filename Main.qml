import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    color: "#202020"

    Column {
        anchors.fill: parent
        spacing: 20
        padding: 20

        // =========================
        // TOP BAR (SAVE / LOAD / RESTART)
        // =========================
        Row {
            spacing: 10

            Button {
                text: "Save"
                onClicked: {
                    console.log("Saving game...")
                    dialogueManager.saveGame()
                    statusText.text = "Game Saved"
                }
            }

            Button {
                text: "Load"
                onClicked: {
                    console.log("Loading game...")
                    dialogueManager.loadGame()
                    statusText.text = "Game Loaded"
                }
            }

            Button {
                text: "Restart"
                onClicked: {
                    console.log("Restarting...")
                    dialogueManager.restartGame()
                    statusText.text = "Game Restarted"
                }
            }
        }

        // =========================
        // STATUS TEXT
        // =========================
        Text {
            id: statusText
            text: ""
            color: "lightgreen"
            font.pixelSize: 14
        }

        // =========================
        // DIALOGUE TEXT
        // =========================
        Text {
            text: dialogueManager ? dialogueManager.currentText : ""
            font.pixelSize: 24
            color: "white"
            wrapMode: Text.WordWrap
        }

        // =========================
        // EVENT TEXT
        // =========================
        Text {
            id: eventText
            text: ""
            color: "yellow"
            font.pixelSize: 20
            visible: false
        }

        // =========================
        // NEXT BUTTON
        // =========================
        Button {
            text: "Next"

            enabled: dialogueManager ? !dialogueManager.inputLocked : false
            opacity: enabled ? 1.0 : 0.4

            onClicked: {
                if (!enabled)
                    return;

                dialogueManager.next()
            }
        }

        // =========================
        // CHOICES
        // =========================
        Column {
            spacing: 10

            Repeater {
                model: dialogueManager ? dialogueManager.choicesModel : null

                delegate: Button {
                    width: 400

                    text: model.text +
                          (model.enabled ? "" : " (" + model.requirement + ")")

                    enabled: dialogueManager
                             ? (model.enabled && !dialogueManager.inputLocked)
                             : false

                    background: Rectangle {
                        color: enabled ? "#e0e0e0" : "#555555"
                        radius: 4
                    }

                    contentItem: Text {
                        text: model.text +
                              (model.enabled ? "" : " (" + model.requirement + ")")
                        color: enabled ? "black" : "#bbbbbb"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (!enabled)
                            return;

                        dialogueManager.selectChoice(index)
                    }
                }
            }
        }
    }

    // =========================
    // EVENT CONNECTIONS
    // =========================
    Connections {
        target: dialogueManager

        function onEventPrint(message) {
            console.log("QML PRINT:", message)
            eventText.text = message
            eventText.visible = true
        }

        function onEventLog(message) {
            console.log("QML LOG:", message)
        }

        function onEventSound(file) {
            console.log("QML SOUND:", file)
        }
    }
}