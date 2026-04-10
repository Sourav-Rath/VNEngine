import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    color: "#202020"   // background fix

    Column {
        anchors.centerIn: parent
        spacing: 20

        // =========================
        // Dialogue Text
        // =========================
        Text {
            text: dialogueManager.currentText
            font.pixelSize: 24
            color: "white"
        }

        // =========================
        // DEBUG (Model size)
        // =========================
        Text {
            text: "Model size: " + dialogueManager.choicesModel.rowCount()
            color: "red"
        }

        // =========================
        // NEXT BUTTON (no choices)
        // =========================
        Button {
            text: "Next"

            visible: dialogueManager.choicesModel.rowCount() === 0

            onClicked: {
                dialogueManager.next()
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
                    text: model.text

                    onClicked: {
                        dialogueManager.selectChoice(model.nextNodeId)
                    }
                }
            }
        }
    }
}