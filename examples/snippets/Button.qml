import QtQuick
import QtQuick.Layouts

import QtTest

import org.kde.union

import org.kde.plasma.components as PC

ApplicationWindow {
    width: 800
    height: 600
    visible: true

    header: ToolBar {
        Element.elementId: "application-header"

        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: toolButton1
                text: "Header ToolButton 1"
                icon.name: "document-save"
                checkable: true
            }
            ToolButton {
                text: "Header ToolButton 2"
                icon.name: "document-save"
                enabled: !toolButton1.checked
            }

            ToolSeparator { }

            PC.ToolButton {
                text: "PC3 ToolButton"
                icon.name: "document-save"

                onClicked: pc3menu.popup(this, 0, height)

                PC.Menu {
                    id: pc3menu

                    PC.MenuItem { text: "Example item" }
                    PC.Menu {
                        title: "Example Submenu"

                        PC.MenuItem { text: "Example Submenu Item 1" }
                        PC.MenuItem { text: "Example Submenu Item 2" }
                    }
                    PC.MenuItem {
                        text: "Checkable Item";
                        checkable: true;
                    }
                    PC.MenuSeparator { }
                    PC.MenuItem {
                        text: "Quit";
                        icon.name: "application-exit-symbolic"
                        onClicked: Qt.quit()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            ToolButton {
                display: ToolButton.IconOnly
                icon.name: "application-menu-symbolic"

                checkable: true
                checked: menu.opened

                onClicked: menu.popup(this, 0, height)

                Menu {
                    id: menu

                    MenuItem { text: "Example item" }
                    Menu {
                        title: "Example Submenu"

                        MenuItem { text: "Example Submenu Item 1" }
                        MenuItem { text: "Example Submenu Item 2" }
                    }
                    MenuItem {
                        text: "Checkable Item";
                        checkable: true
                    }
                    MenuSeparator { }
                    MenuItem {
                        text: "Quit";
                        icon.name: "application-exit-symbolic"
                        onClicked: Qt.quit()
                    }
                }
            }
        }
    }

    GridLayout {
        Element.elementId: "window-contents"

        anchors.centerIn: parent

        columns: 2

        Button {
            id: button1
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            icon.name: "document-save"
            text: "Test"
            checkable: true
        }

        PC.Button {
            id: pcButton1
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            icon.name: "document-save"
            text: "Test"
            checkable: true
        }

        Button {
            text: "Test 2"
            icon.name: "document-save"
            enabled: !button1.checked

            display: Button.TextUnderIcon

            onClicked: {
                switch (display) {
                    case Button.TextBesideIcon:
                        display = Button.IconOnly
                        break
                    case Button.IconOnly:
                        display = Button.TextOnly
                        break;
                    case Button.TextOnly:
                        display = Button.TextUnderIcon
                        break
                    case Button.TextUnderIcon:
                        display = Button.TextBesideIcon
                        break
                }
            }
        }

        PC.Button {
            text: "Test 2"
            icon.name: "document-save"
            enabled: !pcButton1.checked

            onClicked: {
                switch (display) {
                    case Button.TextBesideIcon:
                        display = Button.IconOnly
                        break
                    case Button.IconOnly:
                        display = Button.TextOnly
                        break;
                    case Button.TextOnly:
                        display = Button.TextUnderIcon
                        break
                    case Button.TextUnderIcon:
                        display = Button.TextBesideIcon
                        break
                }
            }
        }

        SpinBox {
        }

        PC.SpinBox {
        }

        ItemDelegate {
            icon.name: "document-save"
            text: "Test 4"
            onClicked: {
                switch (display) {
                    case Button.TextBesideIcon:
                        display = Button.IconOnly
                        break
                    case Button.IconOnly:
                        display = Button.TextOnly
                        break;
                    case Button.TextOnly:
                        display = Button.TextUnderIcon
                        break
                    case Button.TextUnderIcon:
                        display = Button.TextBesideIcon
                        break
                }
            }
        }

        PC.ItemDelegate {
            icon.name: "document-save"
            text: "Test 4"
        }

        MenuItem {
            text: "Menu Item"
            icon.name: "application-exit-symbolic"
        }
    }

    footer: ToolBar {
        Element.elementId: "window-footer"

        enabled: !toolButton1.checked

        TextField {
            anchors.fill: parent

            placeholderText: "Placeholder Text"
        }
    }
}

