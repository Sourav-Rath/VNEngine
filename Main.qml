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

        Row {
            spacing: 10

            Button {
                text: "Save"
                onClicked: {
                    dialogueManager.saveGame()
                    statusText.text = "Game Saved"
                }
            }

            Button {
                text: "Load"
                onClicked: {
                    dialogueManager.loadGame()
                    statusText.text = "Game Loaded"
                }
            }

            Button {
                text: "Restart"
                onClicked: {
                    dialogueManager.restartGame()
                    statusText.text = "Game Restarted"
                }
            }
        }

        Text {
            id: statusText
            text: ""
            color: "lightgreen"
            font.pixelSize: 14
        }

        Text {
            text: dialogueManager ? dialogueManager.currentText : ""
            font.pixelSize: 24
            color: "white"
            wrapMode: Text.WordWrap
        }

        Rectangle {
            visible: eventText.visible   // THIS IS THE FIX

            width: parent.width
            height: 60
            color: "#111111"
            radius: 6

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            opacity: visible ? 1 : 0

            Text {
                id: eventText
                anchors.centerIn: parent
                text: ""
                color: "#ffd166"
                font.pixelSize: 18
            }
        }

        // FIXED NEXT BUTTON (NO BROKEN BINDING)
        Button {
            text: "Next"

            visible: dialogueManager
                     ? (dialogueManager.choicesModel
                        && dialogueManager.choicesModel.count <= 0)
                     : false

            enabled: dialogueManager
                     ? !dialogueManager.inputLocked
                     : false

            opacity: enabled ? 1.0 : 0.4

            onClicked: {
                if (!enabled) return;
                dialogueManager.next()
            }
        }

        Column {
            spacing: 10

            Repeater {
                id: choiceRepeater
                model: dialogueManager ? dialogueManager.choicesModel : null

                delegate: Rectangle {
                    width: 400
                    height: 60
                    radius: 8

                    color: model.enabled ? "#2e2e2e" : "#1a1a1a"
                    border.color: model.enabled ? "#5cff8d" : "#555555"
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 10

                        Text {
                            text: model.text
                            color: model.enabled ? "white" : "#888888"
                            font.pixelSize: 16
                        }

                        Text {
                            text: model.enabled ? "" : "Requires: " + model.requirement
                            color: "#ff6b6b"
                            font.pixelSize: 12
                            visible: !model.enabled
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: dialogueManager
                                 ? (model.enabled && !dialogueManager.inputLocked)
                                 : false

                        onClicked: {
                            dialogueManager.selectChoice(index)
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: dialogueManager

        function onEventPrint(message) {
            eventText.text = message
            eventText.visible = true
        }

        function onChoicesChanged() {
            // FORCE UI REFRESH
            // This is the missing piece
            console.log("Choices updated, count:",
                        dialogueManager.choicesModel.count)
        }
    }
}