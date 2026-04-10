import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    color: "#202020"

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
        // DEBUG
        // =========================
        Text {
            text: "Model size: " + dialogueManager.choicesModel.rowCount()
            color: "red"
        }

        // =========================
        // NEXT BUTTON (only when no choices)
        // =========================
        Button {
            text: "Next"

            visible: dialogueManager.choicesModel.rowCount() === 0

            onClicked: {
                if (dialogueManager.choicesModel.rowCount() === 0)
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
                    //  show requirement text
                    text: model.text + (model.enabled ? "" : " (" + model.requirement + ")")

                    //  disable button
                    enabled: model.enabled

                    onClicked: {
                        if (model.enabled)
                            dialogueManager.selectChoice(index)
                    }
                }
            }
        }
    }
}