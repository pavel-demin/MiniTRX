
import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

Item {

  GroupBox {
    x: 5
    y: 5
    width: 230
    height: 210
    title: "RX"

    Button {
      x: 2
      y: 5
      width: 30
      height: 30
      checkable: true
      text: "ON"
      onClicked: {
        if(checked) {
          text = "OFF"
          client.on_StartRX_clicked()
        } else {
          text = "ON"
          client.on_StopRX_clicked()
        }
      }
    }

    Indicator {
      x: 47
      y: 5
    }

    ComboBox {
      x: 2
      y: 65
      width: 100
    }

    ComboBox {
      x: 112
      y: 65
      width: 100
    }

    ComboBox {
      x: 2
      y: 115
      width: 210
      height: 20
    }

    ProgressBar {
      x: 2
      y: 145
      width: 210
      height: 10
    }

    Slider {
      x: 2
      y: 165
      width: 210
      height: 20
    }

    Label {
      x: 2
      y: 45
      width: 100
      height: 15
      text: "Mode"
    }

    Label {
      x: 112
      y: 45
      width: 100
      height: 15
      text: "Filter"
    }

    Label {
      x: 2
      y: 95
      width: 100
      height: 15
      text: "Device"
    }
  }

  GroupBox {
    x: 285
    y: 5
    width: 230
    height: 210
    title: "TX"

    Button {
      x: 2
      y: 5
      width: 30
      height: 30
      checkable: true
      text: "ON"
      onClicked: {
        if(checked) {
          text = "OFF"
          client.on_StartTX_clicked()
        } else {
          text = "ON"
          client.on_StopTX_clicked()
        }
      }
    }

    Indicator {
      x: 47
      y: 5
    }

    ComboBox {
      x: 112
      y: 65
      width: 100
    }

    ComboBox {
      x: 2
      y: 65
      width: 100
    }

    ComboBox {
      x: 2
      y: 115
      width: 210
      height: 20
    }

    ProgressBar {
      x: 2
      y: 145
      width: 210
      height: 10
    }

    Slider {
      x: 2
      y: 165
      width: 210
      height: 20
    }

    Label {
      x: 2
      y: 45
      width: 100
      height: 15
      text: "Mode"
    }

    Label {
      x: 112
      y: 45
      width: 100
      height: 15
      text: "Filter"
    }

    Label {
      x: 2
      y: 95
      width: 100
      height: 15
      text: "Device"
    }
  }

  GroupBox {
    x: 565
    y: 5
    width: 230
    height: 160
    title: "Spectrum"

    Button {
      x: 2
      y: 5
      width: 30
      height: 30
      checkable: true
      text: "ON"
      onClicked: {
        if(checked) {
          text = "OFF"
          client.on_StartFFT_clicked()
        } else {
          text = "ON"
          client.on_StopFFT_clicked()
        }
      }
    }

    Indicator {
      x: 47
      y: 5
    }

    ComboBox {
      x: 2
      y: 65
      width: 100
    }

    Slider {
      x: 115
      y: 65
      width: 100
      height: 20
    }

    Slider {
      x: 115
      y: 115
      width: 100
      height: 20
    }

    Label {
      x: 2
      y: 45
      width: 100
      height: 15
      text: "Bandwidth"
    }

    Label {
      x: 112
      y: 45
      width: 100
      height: 15
      text: "Range"
    }

    Label {
      x: 2
      y: 95
      width: 100
      height: 15
      text: "Freq. corr. (ppm)"
    }

    Label {
      x: 115
      y: 95
      width: 100
      height: 15
      text: "Offset"
    }
  }

  Button {
    x: 245
    y: 5
    width: 30
    height: 30
    text: ">"
  }

  Button {
    x: 245
    y: 45
    width: 30
    height: 30
    text: "<"
  }

  Button {
    x: 525
    y: 5
    width: 30
    height: 30
    text: "RX>"
  }

  Button {
    x: 525
    y: 45
    width: 30
    height: 30
    text: "TX>"
  }

  Label {
    x: 565
    y: 180
    width: 100
    height: 15
    text: "IP address"
  }

  TextField {
    x: 565
    y: 195
    width: 130
    height: 20
    text: "192.168.1.5"
  }

  Button {
    x: 715
    y: 190
    width: 80
    height: 30
    checkable: true
    text: "Connect"
    onClicked: {
      if(checked) {
        text = "Disconnect"
        client.on_Connect_clicked(address.text)
      } else {
        text = "Connect"
        client.on_Disconnect_clicked()
      }
    }
  }
}