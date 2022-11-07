﻿/**************************************************************************
 * This file is part of the Nunchuk software (https://nunchuk.io/)        *
 * Copyright (C) 2020-2022 Enigmo								          *
 * Copyright (C) 2022 Nunchuk								              *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 3         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                        *
 **************************************************************************/
import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.12
import HMIEVENTS 1.0
import EWARNING 1.0
import QRCodeItem 1.0
import DataPool 1.0
import "../../../Components/customizes"
import "../../../Components/origins"
import "../../../../localization/STR_QML.js" as STR

Row {
    id: conversationPageRoot
    spacing: 0
    property bool  createRoom: false
    property alias modelCoversation: listView.model
    signal createRoomDone()
    signal triggerEditGroupName()
    signal triggerAddMembers()
    signal triggerLeaveGroup()
    signal requestCancelWallet()
    Column {
        id: panelColumn
        width: parent.width - conversationInfo.width
        height: parent.height
        Item {
            id: conversationheader
            width: parent.width
            height: 80
            Rectangle {
                id: bgheader
                width: parent.width - 4
                height: parent.height - 4
                anchors.centerIn: parent
                color: "#FFFFFF"
                visible: false
            }
            DropShadow {
                anchors.fill: bgheader
                verticalOffset: 2
                cached: true
                radius: 8
                samples: 16
                color: Qt.rgba(0, 0, 0, 0.15)
                source: bgheader
                Loader {
                    id: headerLoader
                    anchors.fill: parent
                    sourceComponent: createRoom ? userlist : roomtitle
                }
                Component {
                    id: userlist
                    Item {
                        id: userlistItem
                        property var requestlist: []
                        property var requestlistChatId: []
                        anchors.fill: parent
                        QText {
                            id: totext
                            color: "#000000"
                            font.family: "Lato"
                            font.weight: Font.Bold
                            font.pixelSize: 20
                            text: "To:"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                        }
                        Flickable {
                            anchors {
                                fill: parent
                                rightMargin: 6
                                topMargin: 6 + ((flowuser.implicitHeight > 36) ? 0 : 14)
                                leftMargin: 50
                                bottomMargin: 6
                            }
                            clip: true
                            contentHeight: flowuser.implicitHeight
                            interactive: contentHeight > height
                            flickableDirection: Flickable.VerticalFlick
                            contentY : contentHeight > height ? contentHeight - height : 0
                            ScrollBar.vertical: ScrollBar { active: true }
                            Flow {
                                id: flowuser
                                spacing: 6
                                width: parent.width
                                anchors.verticalCenter: parent.verticalCenter
                                Repeater {
                                    id: userrepeat
                                    model: 0
                                    Rectangle {
                                        id: background
                                        width: emailelement.width + 16
                                        height: 36
                                        radius: 36
                                        color: "#F5F5F5"
                                        Row {
                                            id: emailelement
                                            width: emailtext.width + 60
                                            height: 24
                                            spacing: 6
                                            anchors.verticalCenter: parent.verticalCenter
                                            QAvatar {
                                                width: 36
                                                height: 36
                                                activeStatus: false
                                                avatarUrl: ""
                                                username: userlistItem.requestlist[index]
                                                anchors.verticalCenter: parent.verticalCenter
                                                displayStatus: false
                                            }
                                            QText {
                                                id: emailtext
                                                text: userlistItem.requestlist[index]
                                                height: 24
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                            QImage {
                                                width: 24
                                                height: 24
                                                source: "qrc:/Images/Images/OnlineMode/close_24px.png"
                                                anchors.verticalCenter: parent.verticalCenter
                                                MouseArea {
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                    cursorShape: Qt.PointingHandCursor
                                                    onClicked: {
                                                        suggestItems.visible = false
                                                        userlistItem.requestlist.splice(index, 1)
                                                        userlistItem.requestlistChatId.splice(index, 1)
                                                        userrepeat.model = userlistItem.requestlist.length
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                QTextField {
                                    id: input
                                    height: 36
                                    verticalAlignment: Text.AlignVCenter
                                    font.family: "Lato"
                                    color: "#031F2B"
                                    font.pixelSize: 20
                                    placeholderText: "Name or email"
                                    background: Rectangle {color: "transparent"}
                                    visible: focus ? true : userlistItem.requestlist.length > 0 ? false : true
                                    onTextChanged: {
                                        suggestItems.friends = ClientController.contactsByStringList()
                                        suggestItems.visible = (text !== "")
                                        suggestItems.searchUser(text)
                                    }
                                    Keys.onReturnPressed: {userlistItem.processCreateRoom()}
                                    Keys.onEnterPressed:  {userlistItem.processCreateRoom("")}
                                    function finishInputUser(index){
                                        if(index >= 0) {
                                            userlistItem.requestlist[userlistItem.requestlist.length] = suggestModel.get(index).name
                                            userlistItem.requestlistChatId[userlistItem.requestlistChatId.length] = suggestModel.get(index).chat_id
                                            suggestModel.setProperty(index, "selected", true)
                                            userrepeat.model = userlistItem.requestlist.length
                                        }
                                        input.text = ""
                                    }
                                }
                            }
                        }
                        Rectangle {
                            id: suggestItems
                            property var friends
                            width: parent.width - 50
                            height: suggestItemsBg.height
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.bottom
                            anchors.topMargin: 12
                            visible: false
                            MouseArea {anchors.fill: parent; onClicked: {}}
                            color: Qt.rgba(255, 255, 255, 0.7)
                            radius: 8
                            Rectangle {
                                id: suggestItemsBg
                                width: parent.width
                                height: suggestlist.height
                                color: "#FFFFFF"
                                anchors.centerIn: parent
                                radius: 8
                                QListView {
                                    id: suggestlist
                                    width: parent.width
                                    spacing: 0
                                    height: Math.min(count*48, 10*48)
                                    model: suggestModel
                                    visible: count > 0
                                    clip: true
                                    ScrollBar.horizontal: ScrollBar { active: true }
                                    delegate: Rectangle {
                                        width: suggestlist.width
                                        height: 48
                                        radius: 8
                                        color: suggestMouse.containsMouse ? "#EFEFEF" : "transparent"
                                        Row {
                                            width: parent.width - 32
                                            height: 48
                                            anchors.centerIn: parent
                                            spacing: 8
                                            QAvatar {
                                                width: 36
                                                height: 36
                                                activeStatus: false
                                                avatarUrl: ""
                                                username: model.name
                                                displayStatus: false
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                            QText {
                                                color: "#000000"
                                                font.family: "Lato"
                                                font.weight: Font.DemiBold
                                                font.pixelSize: 16
                                                text: model.name
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                            QText {
                                                color: "#000000"
                                                font.family: "Lato"
                                                font.pixelSize: 12
                                                text: "(" + model.email + ")"
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                            QImage {
                                                width: 24
                                                height: 24
                                                anchors.verticalCenter: parent.verticalCenter
                                                visible: model.selected
                                                source: "qrc:/Images/Images/OnlineMode/check_circle_24px copy.png"
                                            }
                                        }
                                        MouseArea {
                                            id: suggestMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {input.finishInputUser(index)}
                                        }
                                    }
                                }
                                ListModel {
                                    id: suggestModel
                                }
                            }
                            DropShadow {
                                anchors.fill: suggestItemsBg
                                horizontalOffset: 3
                                verticalOffset: 3
                                radius: 8.0
                                samples: 17
                                color: "#80000000"
                                source: suggestItemsBg
                            }
                            function searchUser( word ){
                                suggestModel.clear()
                                if(word === "") return;
                                for(var i = 0; i < suggestItems.friends.length; i++){
                                    var id = suggestItems.friends[i].split('|')[0]
                                    var name = suggestItems.friends[i].split('|')[1]
                                    var mail = suggestItems.friends[i].split('|')[2]
                                    var chat_id = suggestItems.friends[i].split('|')[3]
                                    if(name.toLowerCase().includes(word.toLowerCase()) || mail.toLowerCase().includes(word.toLowerCase())){
                                        var data = {'id': id, 'name': name, 'email': mail, 'chat_id': chat_id, 'selected': userlistItem.requestlist.includes(name)};
                                        suggestModel.append(data)
                                    }
                                }
                            }
                        }
                        Component.onCompleted: { suggestItems.friends = ClientController.contactsByStringList() }
                        function processCreateRoom(firstMessage){
                            ClientController.createRoomChat(userlistItem.requestlistChatId, userlistItem.requestlist, firstMessage)
                            createRoomDone()
                        }
                    }
                }
                Component {
                    id: roomtitle
                    Row {
                        spacing: 12
                        anchors {
                            fill: parent
                            leftMargin: 22
                            rightMargin: 36
                            topMargin: 22
                            bottomMargin: 22
                            verticalCenter: parent.verticalCenter
                        }
                        Row {
                            id: avartarArray
                            spacing: -8
                            anchors.verticalCenter: parent.verticalCenter
                            Repeater {
                                model: (RoomWalletData.currentRoom !== null) ? RoomWalletData.currentRoom.userCount : 0
                                QAvatar {
                                    id: avatarheader
                                    width: 36
                                    height: 36
                                    avatarUrl: RoomWalletData.currentRoom.users.get(index).avatar + RoomWalletData.currentRoom.users.change
                                    username: RoomWalletData.currentRoom.users.get(index).name + RoomWalletData.currentRoom.users.change
                                    displayStatus: false
                                }
                            }
                        }
                        QText {
                            width: parent.width - 48 - avartarArray.width
                            elide: Text.ElideRight
                            color: "#000000"
                            font.family: "Lato"
                            font.weight: Font.Bold
                            font.pixelSize: 20
                            text: RoomWalletData.currentRoom ? RoomWalletData.currentRoom.roomName : STR.STR_QML_502
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        MouseArea {
                            id: chatinfoMouse
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            enabled: ClientController.rooms ? ClientController.rooms.count > 0 : false
                            onEnabledChanged: {
                                if(chatinfoMouse.enabled === false){
                                    conversationInfo.width = 0
                                }
                            }

                            width: ico.width
                            height: ico.height
                            anchors.verticalCenter: parent.verticalCenter
                            QImage {
                                id: ico
                                width: 24
                                height: 24
                                scale: chatinfoMouse.containsMouse ? 1.1 : 1
                                transformOrigin: Item.Center
                                source: "qrc:/Images/Images/tooltip.png"
                                layer.enabled: ClientController.rooms ? ClientController.rooms.count === 0 : false
                                layer.effect: Desaturate {
                                    anchors.fill: ico
                                    source: ico
                                    desaturation: 1.0
                                }
                            }
                            onClicked:  { conversationInfo.width = 266 }
                        }
                    }
                }
            }
        }
        spacing: 0
        QListView {
            id: listView
            readonly property bool noNeedMoreContent: RoomWalletData.currentRoom === null ? true : RoomWalletData.currentRoom.allHisLoaded
            readonly property int indicatorY: (contentY-originY) * (height/contentHeight)
            readonly property int indicatorMaxY: height-scrollContact.size*height
            readonly property int minDelegateHeight: 20
            readonly property bool showSrollBottom: (listView.indicatorY < (listView.indicatorMaxY - minDelegateHeight)) && listView.count > 0
            width: parent.width
            height: parent.height - conversationheader.height - conversationfooter.height
            clip: true
            focus: true
            enabled: RoomWalletData.currentRoom !== null && !createRoom
            currentIndex: (RoomWalletData.currentRoom !== null && RoomWalletData.currentRoom.conversation !== null) ? RoomWalletData.currentRoom.conversation.currentIndex : 0
            delegate: MouseArea{
                width: listView.width
                height: cons.height == 0 ? 0 : (cons.height + 8)
                QConversationDelegate {
                    id:cons
                    anchors.verticalCenter: parent.verticalCenter
                    width: listView.width
                    isSentByMe: model.sendByMe
                    avatarLink: model.avatar
                    sendername: model.sender
                    messageContent: model.message
                    messageTime: model.timestamp
                    isStateEvent: model.isStateEvent
                    timeSection: model.timesection
                    messageType: model.messageType
                    init_event_id: model.init_event_id
                    txnId: model.txnId
                    evtId: model.evtId
                    onRequestCancelWallet: conversationPageRoot.requestCancelWallet()
                }
                onWheel: {
                    wheel.accepted = false
                    if(wheel.angleDelta.y !== 0){
                        if(scrollContact)
                        scrollContact.wheel(wheel.angleDelta.y > 0)
                    }
                }
            }
            add: Transition {
                NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 300 }
                NumberAnimation { property: "scale"  ; from: 1.5; to: 1; duration: 300 }
            }
            highlightFollowsCurrentItem: true
            onModelChanged: {
                var currentIndexFocus = 0;
                if(RoomWalletData.currentRoom && RoomWalletData.currentRoom.conversation){
                    currentIndexFocus = RoomWalletData.currentRoom.conversation.currentIndex
                }
                listView.positionViewAtIndex(currentIndexFocus, ListView.End)
            }
            onCurrentIndexChanged: { listView.positionViewAtIndex(currentIndex, ListView.End) }
            onContentYChanged: ensurePreviousContent()
            Component.onCompleted: {
                var currentIndexFocus = 0;
                if(RoomWalletData.currentRoom && RoomWalletData.currentRoom.conversation){
                    currentIndexFocus = RoomWalletData.currentRoom.conversation.currentIndex
                }
                listView.positionViewAtIndex(currentIndexFocus, ListView.End)
            }
            function ensurePreviousContent() {
                if (noNeedMoreContent) return
                if(contentY < originY){ if(RoomWalletData.currentRoom) { RoomWalletData.currentRoom.getMoreContents() } }
            }
            section
            {
                property: "timesection"
                criteria: ViewSection.FullString
                delegate: Item {
                    width: listView.width
                    height: 36
                    QText {
                        text: section
                        font.pixelSize: 12
                        font.family: "Lato"
                        anchors.centerIn: parent
                        color: "#595959"
                    }
                }
            }
            ScrollBar.vertical: ScrollBar { id: scrollContact;
                active: true;
                function wheel(up)
                {
                    if(up){
                        //                        decrease();
                    }else{
                        //                        increase();
                        RoomWalletData.currentRoom.markFiveMessagesAsRead()
                    }
                }
            }

            onFlickEnded: {
                if (listView.verticalVelocity > 0) {
                    RoomWalletData.currentRoom.markFiveMessagesAsRead()
                }
            }

            Item {
                id: typingItem
                width: parent.width
                height: 20
                anchors.bottom: parent.bottom
                visible: RoomWalletData.currentRoom !== null && RoomWalletData.currentRoom.typingNames !== ""
                QText {
                    text: RoomWalletData.currentRoom !== null ? RoomWalletData.currentRoom.typingNames : ""
                    font.pixelSize: 12
                    font.family: "Lato"
                    anchors.centerIn: parent
                    color: "#595959"
                }
            }
            Connections{
                target: RoomWalletData.currentRoom
                onUnreadCountChanged:{
                    RoomWalletData.currentRoom.markAllMessagesAsRead()
                }
            }

            MouseArea {
                id: btnScrollTopMouse
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                width: btnScrollTop.width
                height: btnScrollTop.height
                visible: RoomWalletData.currentRoom !== null && RoomWalletData.currentRoom.unreadCount > 0
                enabled: visible
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: 16
                }

                Rectangle{
                    id: btnScrollTop
                    width: 112
                    height: 36
                    color: "#031F2B"
                    radius: 24
                    QText {
                        id: _unread
                        color: "#FFFFFF"
                        font.family: "Lato"
                        font.pixelSize: 16
                        text: (RoomWalletData.currentRoom !== null ? RoomWalletData.currentRoom.unreadCount : 0)  + " Unread"
                        anchors{
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: 12
                        }
                    }
                    QImage {
                        width: 24
                        height: 24
                        anchors{
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                            rightMargin: 2
                        }
                        scale: btnScrollTopMouse.pressed ? 1 : btnScrollTopMouse.containsMouse ? 1.1 : 1
                        transformOrigin: Item.Center
                        source: "qrc:/Images/Images/close_24px.png"
                    }
                }

                onClicked:  {
                    RoomWalletData.currentRoom.markAllMessagesAsRead()
                    listView.positionViewAtIndex(listView.count - 1, ListView.End)
                }
            }

            MouseArea {
                id: btnScrollBottomMouse
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                width: btnScrollBottom.width
                height: btnScrollBottom.height
                visible: RoomWalletData.currentRoom !== null && listView.showSrollBottom
                enabled: visible
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16
                QImage {
                    id: btnScrollBottom
                    width: 36
                    height: 36
                    scale: btnScrollBottomMouse.pressed ? 1 : btnScrollBottomMouse.containsMouse ? 1.1 : 1
                    transformOrigin: Item.Center
                    source: "qrc:/Images/Images/OnlineMode/downend.png"
                }
                onClicked:  { listView.positionViewAtIndex((listView.count-1), ListView.End) }
            }
        }
        Item {
            id: conversationfooter
            width: parent.width
            height: 80
            Rectangle {
                id: bgfooter
                width: parent.width - 4
                height: parent.height - 4
                anchors.centerIn: parent
                color: "#FFFFFF"
                visible: false
            }
            DropShadow {
                anchors.fill: bgfooter
                verticalOffset: 2
                cached: true
                radius: 8
                samples: 16
                color: Qt.rgba(0, 0, 0, 0.15)
                source: bgfooter
                QTextField {
                    id: messageField
                    width: parent.width - 108
                    height: 48
                    anchors.left: parent.left
                    anchors.leftMargin: 24
                    anchors.verticalCenter: parent.verticalCenter
                    placeholderText: "Type your message..."
                    Keys.onReturnPressed: { conversationfooter.sendMessage() }
                    Keys.onEnterPressed:  { conversationfooter.sendMessage() }
                    clip: true
                    color: "#031F2B"
                    font.pixelSize: 16
                    selectByMouse: true
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        border.color: "#DEDEDE"
                        color: "#FFFFFF"
                    }
                    onActiveFocusChanged:{ if(activeFocus){RoomWalletData.currentRoom.markFiveMessagesAsRead()}}
                }
                Loader {
                    id: commandButton
                    sourceComponent: RoomWalletData.roomWalletCreated ? btnCreateTransaction : btnCreateSharedWallet
                    anchors {
                        left: messageField.right
                        leftMargin: 12
                        verticalCenter: messageField.verticalCenter
                    }
                }
            }
            function sendMessage(){
                if(createRoom){
                    headerLoader.item.processCreateRoom(messageField.text)
                }
                else{
                    if(RoomWalletData.currentRoom !== null && messageField.text !== "") {RoomWalletData.currentRoom.sendMessage(messageField.text)}
                    messageField.text = "";
                }
            }
        }
    }
    QConversationInfo{
        id: conversationInfo
        width: 0
        height: parent.height
    }
    Component {
        id: btnCreateSharedWallet
        MouseArea {
            id: createSharedBottom
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            width: icoSwl.width
            height: icoSwl.height
            QImage {
                id: icoSwl
                width: 24
                height: 24
                scale: createSharedBottom.pressed ? 1: createSharedBottom.containsMouse ? 1.1 : 1
                transformOrigin: Item.Center
                source: "qrc:/Images/Images/OnlineMode/Joint wallet_031F2B.png"
            }
            enabled: RoomWalletData.currentRoom
            onClicked:{
                if(RoomWalletData.roomWalletInitialized){
                    QMLHandle.sendEvent(EVT.EVT_HOME_SHARED_WALLET_CONFIGURE, RoomWalletData.currentRoom.roomid)
                }
                else{
                    QMLHandle.sendEvent(EVT.EVT_HOME_ONLINE_CREATE_SHARED_WALLET)
                }
            }
        }
    }
    Component {
        id: btnCreateTransaction
        MouseArea {
            id: createTXBottom
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            width: icoSwl.width
            height: icoSwl.height
            QImage {
                id: icoSwl
                width: 24
                height: 24
                scale: createTXBottom.pressed ? 1: createTXBottom.containsMouse ? 1.1 : 1
                transformOrigin: Item.Center
                source: "qrc:/Images/Images/OnlineMode/monetization_on-24px.png"
            }
            enabled: RoomWalletData.currentRoom
            onClicked:{
                QMLHandle.sendEvent(EVT.EVT_HOME_SHARED_WL_SEND_REQUEST)
            }
        }
    }
}
