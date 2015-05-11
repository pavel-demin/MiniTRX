import QtQuick 2.4
import QtQuick.Controls 1.3
import MiniTRX 1.0

Item {
  width: 782
  height: 500

  Rectangle {
    x: 5
    y: 5
    width: 772
    height: 160
    color: "black"
    Spectrum {
      objectName: "spectrum"
      anchors.fill: parent
    }
  }

  Waterfall {
    objectName: "waterfall"
    x: 5
    y: 165
    width: 772
    height: 80
  }

  TabView {
    x: 5
    y: 250
    width: 774
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
