import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    // DANGER LEVEL (CORE DRIVER)
    property int dangerLevel: Math.abs(dialogueManager.state["sanity"]) +
                              Math.abs(dialogueManager.state["time"])

    // BACKGROUND REACTS TO DANGER
    color: dangerLevel > 6 ? "#220000" : "#202020"

    Column {
        anchors.fill: parent
        spacing: 20
        padding: 20

        // ================= CONTROLS =================

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

        // ================= STATS =================

        Text {
            id: statsText

            text: dialogueManager
                  ? "Karma: " + dialogueManager.state["karma"]
                    + " | Sanity: " + dialogueManager.state["sanity"]
                    + " | Knowledge: " + dialogueManager.state["knowledge"]
                    + " | Violence: " + dialogueManager.state["violence"]
                    + " | Time: " + dialogueManager.state["time"]
                  : ""

            color: "#bbbbbb"
            font.pixelSize: 14
        }

        // ================= TIMER =================

        Text {
            text: "Time Left: " + dialogueManager.timer

            color: dialogueManager.timer <= 2 ? "red" : "white"

            font.pixelSize: dialogueManager.timer <= 2 ? 26 : 22

            scale: dialogueManager.timer <= 2 ? 1.1 : 1.0

            Behavior on scale {
                NumberAnimation { duration: 150 }
            }
        }

        // ================= DIALOGUE =================

        Text {
            text: dialogueManager ? dialogueManager.currentText : ""

            font.pixelSize: 24
            color: "white"
            wrapMode: Text.WordWrap

            // PULSE UNDER PRESSURE
            scale: dialogueManager.timer <= 2 ? 1.05 : 1.0

            Behavior on scale {
                NumberAnimation { duration: 200 }
            }

            // SANITY DISTORTION (SUBTLE)
            rotation: dialogueManager.state["sanity"] <= -6
                      ? (Math.random() * 2 - 1) * 2
                      : 0
        }

        // ================= EVENT BOX =================

        Rectangle {
            visible: eventText.visible
            width: parent.width
            height: 60
            color: "#111111"
            radius: 6

            opacity: visible ? 1 : 0

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            Text {
                id: eventText
                anchors.centerIn: parent
                text: ""
                color: "#ffd166"
                font.pixelSize: 18
            }
        }

        // ================= NEXT BUTTON =================

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

        // ================= CHOICES =================

        Column {
            spacing: 10

            Repeater {
                model: dialogueManager ? dialogueManager.choicesModel : null

                delegate: Item {
                    width: 400
                    height: 70

                    property bool hovered: false
                    property bool pressed: false

                    Rectangle {
                        anchors.fill: parent
                        radius: 8

                        color: pressed
                               ? "#1f3d2b"
                               : hovered
                                 ? "#2a2a2a"
                                 : (model.enabled ? "#2e2e2e" : "#1a1a1a")

                        border.color: model.enabled ? "#5cff8d" : "#555555"
                        border.width: 1
                    }

                    Column {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 4

                        Text {
                            text: model.text
                            color: model.enabled ? "white" : "#888888"
                            font.pixelSize: 16
                            wrapMode: Text.WordWrap

                            // DISTORT WHEN INSANE
                            rotation: dialogueManager.state["sanity"] <= -6
                                      ? (Math.random() * 2 - 1) * 2
                                      : 0
                        }

                        Text {
                            text: model.enabled ? "" : model.requirement
                            color: "#ff6b6b"
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            visible: !model.enabled
                        }

                        Text {
                            text: {
                                if (!model.setFlags)
                                    return ""

                                let parts = []

                                for (let key in model.setFlags) {
                                    let val = model.setFlags[key]

                                    if (val === true)
                                        parts.push("+ " + key)
                                    else if (val > 0)
                                        parts.push("+" + val + " " + key)
                                    else if (val < 0)
                                        parts.push(val + " " + key)
                                }

                                return parts.join(", ")
                            }

                            color: "#6be3ff"
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            visible: model.enabled
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: dialogueManager
                                 ? (model.enabled && !dialogueManager.inputLocked)
                                 : false

                        hoverEnabled: true

                        onEntered: parent.hovered = true
                        onExited: parent.hovered = false

                        onPressed: parent.pressed = true
                        onReleased: parent.pressed = false

                        onClicked: {
                            dialogueManager.selectChoice(index)
                        }
                    }
                }
            }
        }
    }

    // ================= HEARTBEAT OVERLAY =================

    Rectangle {
        anchors.fill: parent
        color: "red"

        opacity: dialogueManager.timer <= 2 ? 0.15 : 0

        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }
    }

    // ================= SIGNAL CONNECTIONS =================

    Connections {
        target: dialogueManager

        function onChoicesChanged() {
            console.log("Choices updated:",
                        dialogueManager.choicesModel.count)
        }

        function onEventPrint(message) {
            eventText.text = message
            eventText.visible = true
        }

        function onEventLog(message) {
            console.log("LOG:", message)
        }

        function onEventSound(file) {
            console.log("SOUND:", file)
        }
    }
}