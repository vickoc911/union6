/*
 *    SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
 *    SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
 *    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQml.Models
import QtQuick
import QtQuick.Templates as T
import org.kde.union.impl as Union
import org.kde.sonnet as Sonnet

Menu {
    id: root

    property Item target

    // assuming that Instantiator::active is bound to target.Kirigami.SpellCheck.enabled
    property Instantiator/*<Sonnet.SpellcheckHighlighter>*/ spellcheckHighlighterInstantiator

    // assuming that spellchecker's active state is not writable, use target.Kirigami.SpellCheck.enabled instead.
    readonly property Sonnet.SpellcheckHighlighter spellcheckHighlighter:
    spellcheckHighlighterInstantiator?.object as Sonnet.SpellcheckHighlighter

    property /*list<string>*/var spellcheckSuggestions: []

    // context menu keyboard key
    function targetKeyPressed(event, target) {
        if (event.modifiers === Qt.NoModifier && event.key === Qt.Key_Menu) {
            this.target = target;
            target.persistentSelection = true; // persist selection when menu is opened
            const targetCursorRectangle = target.cursorRectangle;
            popup(target, targetCursorRectangle.right, targetCursorRectangle.bottom);
        }
    }

    function __hasSelectedText(): bool {
        return target !== null
        && target.selectedText !== "";
    }

    function __editable(): bool {
        return target !== null
        && !target.readOnly;
    }

    function __hasSpellcheckCapability(): bool {
        return __editable()
        && spellcheckHighlighterInstantiator !== null;
    }

    function __showSpellcheckActions(): bool {
        return __editable()
        && spellcheckHighlighter !== null
        && spellcheckHighlighter.active
        && spellcheckHighlighter.wordIsMisspelled;
    }

    // Show actions which should normally be hidden for password field
    function __showPasswordRestrictedActions(): bool {
        return target !== null
        && target.echoMode !== TextInput.PasswordEchoOnEdit
        && target.echoMode !== TextInput.Password;
    }

    // Show text editing actions which should normally be hidden for password field
    function __showPasswordRestrictedEditingActions(): bool {
        return __showPasswordRestrictedActions() && !target.readOnly;
    }

    modal: true

    Instantiator {
        active: root.__showSpellcheckActions()

        model: root.spellcheckSuggestions
        delegate: MenuItem {
            required property string modelData

            text: modelData

            onClicked: {
                root.target.persistentSelection = true;
                root.spellcheckHighlighter.replaceWord(modelData);
            }
        }
        onObjectAdded: (index, object) => {
            root.insertItem(0, object);
        }
        onObjectRemoved: (index, object) => {
            root.removeItem(object);
        }
    }

    MenuItem {
        visible: root.__showSpellcheckActions() && root.spellcheckSuggestions.length === 0
        action: T.Action {
            enabled: false
            text: root.spellcheckHighlighter
            ? qsTr('No Suggestions for "%1"')
            .arg(root.spellcheckHighlighter.wordUnderMouse)
            : ""
        }
    }

    MenuSeparator {
        visible: root.__showSpellcheckActions()
    }

    MenuItem {
        visible: root.__showSpellcheckActions()
        action: T.Action {
            text: root.spellcheckHighlighter
            ? qsTr('Add "%1" to Dictionary')
            .arg(root.spellcheckHighlighter.wordUnderMouse)
            : ""

            onTriggered: {

                root.spellcheckHighlighter.addWordToDictionary(root.spellcheckHighlighter.wordUnderMouse);
            }
        }
    }

    MenuItem {
        visible: root.__showSpellcheckActions()
        action: T.Action {
            text: qsTr("Ignore")
            onTriggered: {

                root.spellcheckHighlighter.ignoreWord(root.spellcheckHighlighter.wordUnderMouse);
            }
        }
    }

    MenuItem {
        visible: root.__hasSpellcheckCapability()

        checkable: true
        checked: root.target?.Union.OutputProperties.spellCheckEnabled ?? false
        text: qsTr("Spell Check")

        onToggled: {
            if (root.target) {
                root.target.Union.OutputProperties.spellCheckEnabled = checked;
            }
        }
    }

    MenuSeparator {
        visible: root.__hasSpellcheckCapability()
        && (root.__editable() || root.__showPasswordRestrictedActions())
    }

    MenuItem {
        action: T.Action {
            icon.name: "edit-undo-symbolic"
            text: qsTr("Undo")
            shortcut: StandardKey.Undo
        }
        visible: root.__showPasswordRestrictedEditingActions()
        enabled: root.target?.canUndo ?? false
        onTriggered: {

            root.target.undo();
        }
    }
    MenuItem {
        action: T.Action {
            icon.name: "edit-redo-symbolic"
            text: qsTr("Redo")
            shortcut: StandardKey.Redo
        }
        visible: root.__showPasswordRestrictedEditingActions()
        enabled: root.target?.canRedo ?? false
        onTriggered: {

            root.target.redo();
        }
    }
    MenuSeparator {
        visible: root.__showPasswordRestrictedEditingActions()
    }
    MenuItem {
        action: T.Action {
            icon.name: "edit-cut-symbolic"
            text: qsTr("Cut")
            shortcut: StandardKey.Cut
        }
        visible: root.__showPasswordRestrictedEditingActions()
        enabled: root.__hasSelectedText()
        onTriggered: {

            root.target.cut();
        }
    }
    MenuItem {
        action: T.Action {
            icon.name: "edit-copy-symbolic"
            text: qsTr("Copy")
            shortcut: StandardKey.Copy
        }
        visible: root.__showPasswordRestrictedActions()
        enabled: root.__hasSelectedText()
        onTriggered: {
            console.warn(root.target.selectedText);

            root.target.copy();
        }
    }
    MenuItem {
        action: T.Action {
            icon.name: "edit-paste-symbolic"
            text: qsTr("Paste")
            shortcut: StandardKey.Paste
        }
        visible: root.__editable()
        enabled: target?.canPaste ?? false
        onTriggered: {
            root.target.paste();
        }
    }
    MenuItem {
        action: T.Action {
            icon.name: "edit-delete-symbolic"
            text: qsTr("Delete")
            shortcut: StandardKey.Delete
        }
        visible: root.__editable()
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.target.remove(root.target.selectionStart, root.target.selectionEnd);
        }
    }
    MenuSeparator {
        visible: root.target !== null
        && (root.__editable() || root.__showPasswordRestrictedActions())
    }
    MenuItem {
        action: T.Action {
            icon.name: "edit-select-all-symbolic"
            text: qsTr("Select All")
            shortcut: StandardKey.SelectAll
        }
        visible: root.target !== null
        onTriggered: {

            root.target.selectAll();
        }
    }
}
