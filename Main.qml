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
            text: dialogueManager.currentText
            font.pixelSize: 24
            color: "white"
            wrapMode: Text.WordWrap
        }

        // =========================
        //  EVENT TEXT (NEW)
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

            visible: dialogueManager.choicesModel.rowCount() === 0

            onClicked: {
                if (dialogueManager.choicesModel.rowCount() === 0) {
                    console.log("Next pressed")
                    dialogueManager.next()
                }
            }
        }

        // =========================
        // CHOICES
        // =========================
        Column {
            spacing: 10

            Repeater {
                model: dialogueManager.choicesModel

                delegate: Button {
                    width: 400

                    text: model.text +
                          (model.enabled ? "" : " (" + model.requirement + ")")

                    enabled: model.enabled

                    onClicked: {
                        console.log("Choice clicked:", model.text, "Index:", index)

                        if (model.enabled)
                            dialogueManager.selectChoice(index)
                        else
                            console.log("Blocked choice")
                    }
                }
            }
        }
    }

    // =========================
    //  EVENT CONNECTIONS (CORE STEP 19.2)
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
            // sound system later
        }
    }
}