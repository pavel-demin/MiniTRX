import QtQuick 2.0

Item {
  id: root

  property int value: 600000

  width: 160
  height: width / 5.2

  ListView {
    id: view

    anchors.fill: parent

    orientation: ListView.Horizontal

    model: 8

    delegate: Item {
      id: viewItem

      property string colorTop: "#00000000"
      property string colorBottom: "#00000000"

      property int delta: Math.pow(10, view.count - model.index - 1)
      property int tmp: 0

      width: (view.count - model.index - 1) % 3 == 2 ? parent.height * 0.8 : parent.height * 0.6
      height: parent.height

      Rectangle {
        anchors.fill: parent
        anchors.leftMargin: (view.count - model.index - 1) % 3 == 2 ? parent.width / 4 : 0

        Text {
          id: label
          anchors.centerIn: parent
          font.pixelSize: parent.height
          color: Math.floor(root.value / viewItem.delta) > 0 ? "black" : "lightgrey"
          text: Math.floor(root.value / viewItem.delta) % 10
        }

        gradient: Gradient {
          GradientStop { id: stop1; position: 0.00; color: viewItem.colorTop; }
          GradientStop { id: stop2; position: 0.49; color: viewItem.colorTop; }
          GradientStop { id: stop5; position: 0.51; color: viewItem.colorBottom; }
          GradientStop { id: stop6; position: 1.00; color: viewItem.colorBottom; }
        }

        MouseArea {
          anchors.fill: parent
          hoverEnabled: true
          acceptedButtons: Qt.LeftButton

          onClicked: {
            if(mouseY < 0.5 * parent.height) {
              viewItem.tmp = root.value + viewItem.delta
              root.value = viewItem.tmp > 50000000 ? 50000000 : viewItem.tmp
            } else if(Math.floor(root.value / viewItem.delta) > 0) {
              viewItem.tmp = root.value - viewItem.delta
              root.value = viewItem.tmp < 0 ? 0 : viewItem.tmp
            }
          }

          onWheel: {
            if(wheel.angleDelta.y > 0) {
              viewItem.tmp = root.value + Math.floor(wheel.angleDelta.y / 90) * viewItem.delta
              root.value = viewItem.tmp > 50000000 ? 50000000 : viewItem.tmp
            } else if(Math.floor(root.value / viewItem.delta) > 0) {
              viewItem.tmp = root.value + Math.ceil(wheel.angleDelta.y / 90) * viewItem.delta
              root.value = viewItem.tmp < 0 ? 0 : viewItem.tmp
            }
          }

          onPositionChanged: {
            if(mouseY < 0.5 * parent.height) {
              viewItem.colorTop = "#FFFF99FF"
              viewItem.colorBottom = "#00000000"
            } else {
              viewItem.colorTop = "#00000000"
              viewItem.colorBottom = "#FF99CCFF"
            }
          }

          onExited: {
            viewItem.colorTop = "#00000000"
            viewItem.colorBottom = "#00000000"
          }
        }
      }
    }
  }
}