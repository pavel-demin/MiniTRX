import QtQuick 2.0

Item {
  width: 160
  height: width / 5.2

  ListView {
    id: indicator

    property int value: 600000

    anchors.fill: parent

    orientation: ListView.Horizontal

    model: 8

    delegate: Item {
      id: indicatorItem

      property string colorTop: "#00000000"
      property string colorBottom: "#00000000"

      property int delta: Math.pow(10, indicator.count - model.index - 1)
      property int tmp: 0

      width: (indicator.count - model.index - 1) % 3 == 2 ? parent.height * 0.8 : parent.height * 0.6
      height: parent.height

      Rectangle {
        anchors.fill: parent
        anchors.leftMargin: (indicator.count - model.index - 1) % 3 == 2 ? parent.width / 4 : 0

        Text {
          id: label
          anchors.centerIn: parent
          font.pixelSize: parent.height
          color: Math.floor(indicator.value / indicatorItem.delta) > 0 ? "black" : "lightgrey"
          text: Math.floor(indicator.value / indicatorItem.delta) % 10
        }

        gradient: Gradient {
          GradientStop { id: stop1; position: 0.00; color: indicatorItem.colorTop; }
          GradientStop { id: stop2; position: 0.49; color: indicatorItem.colorTop; }
          GradientStop { id: stop5; position: 0.51; color: indicatorItem.colorBottom; }
          GradientStop { id: stop6; position: 1.00; color: indicatorItem.colorBottom; }
        }

        MouseArea {
          anchors.fill: parent
          hoverEnabled: true
          acceptedButtons: Qt.LeftButton

          onClicked: {
            if(mouseY < 0.5 * parent.height) {
              indicatorItem.tmp = indicator.value + indicatorItem.delta
              indicator.value = indicatorItem.tmp > 50000000 ? 50000000 : indicatorItem.tmp
            } else if(Math.floor(indicator.value / indicatorItem.delta) > 0) {
              indicatorItem.tmp = indicator.value - indicatorItem.delta
              indicator.value = indicatorItem.tmp < 0 ? 0 : indicatorItem.tmp
            }
          }

          onWheel: {
            if(wheel.angleDelta.y > 0) {
              indicatorItem.tmp = indicator.value + Math.floor(wheel.angleDelta.y / 90) * indicatorItem.delta
              indicator.value = indicatorItem.tmp > 50000000 ? 50000000 : indicatorItem.tmp
            } else if(Math.floor(indicator.value / indicatorItem.delta) > 0) {
              indicatorItem.tmp = indicator.value + Math.ceil(wheel.angleDelta.y / 90) * indicatorItem.delta
              indicator.value = indicatorItem.tmp < 0 ? 0 : indicatorItem.tmp
            }
          }

          onPositionChanged: {
            if(mouseY < 0.5 * parent.height) {
              indicatorItem.colorTop = "#FFFF99FF"
              indicatorItem.colorBottom = "#00000000"
            } else {
              indicatorItem.colorTop = "#00000000"
              indicatorItem.colorBottom = "#FF99CCFF"
            }
          }

          onExited: {
            indicatorItem.colorTop = "#00000000"
            indicatorItem.colorBottom = "#00000000"
          }
        }
      }
    }
  }
}