import QtQuick 2.4
import QtQuick.Controls 1.3

Item {
  width: 783
  height: 500

  Rectangle {
    x: 5
    y: 5
    width: 773
    height: 245
    color: "#000000"
  }

  TabView {
    x: 5
    y: 250
    width: 775
    height: 245

    tabPosition: Qt.BottomEdge

    Tab {
      title: "Main"
      TabMain {
      }
    }
    Tab {
      title: "RX settings"
      TabRX {
      }
    }
    Tab {
      title: "TX settings"
      TabTX {
      }
    }
  }
}
