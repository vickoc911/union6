import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.qmlmodels

Kirigami.Page {
    title: "Tables QtQuick.Controls"
    
    Kirigami.ColumnView.interactiveResizeEnabled: true
    Kirigami.ColumnView.minimumWidth: Kirigami.Units.gridUnit * 10
    Kirigami.ColumnView.preferredWidth: Kirigami.Units.gridUnit * 25
    Kirigami.ColumnView.maximumWidth: Kirigami.Units.gridUnit * 50
    
    Rectangle {
        anchors.fill: parent

        Controls.HorizontalHeaderView {
            id: horizontalHeader
            anchors.left: tableView.left
            anchors.top: parent.top
            syncView: tableView
            clip: true
        }
        
        Controls.VerticalHeaderView {
            id: verticalHeader
            anchors.top: tableView.top
            anchors.left: parent.left
            syncView: tableView
            clip: true
        }
        
        TableView {
            id: tableView
            anchors.left: verticalHeader.right
            anchors.top: horizontalHeader.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            
            columnSpacing: 1
            rowSpacing: 1
            
            model: TableModel {
                TableModelColumn { display: "name" }
                TableModelColumn { display: "color" }
                
                rows: [
                    {
                        "name": "cat",
                        "color": "black"
                    },
                    {
                        "name": "dog",
                        "color": "brown"
                    },
                    {
                        "name": "bird",
                        "color": "white"
                    }
                ]
            }
            
            delegate: Rectangle {
                implicitWidth: 100
                implicitHeight: 20
                color: palette.base
                Controls.Label {
                    text: display
                }
            }
        }
    }
}