/**************************************************************************
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
import QtQuick.Controls 2.1
import QtGraphicalEffects 1.0
import HMIEVENTS 1.0
import EWARNING 1.0
import QRCodeItem 1.0
import DataPool 1.0
import NUNCHUCKTYPE 1.0
import "../../../Components/customizes"
import "../../../Components/origins"
import "../../../../localization/STR_QML.js" as STR

Rectangle {
    id: roomRoot
    width: 326
    height: columnItem.height
    color: "transparent"
    property string serverkeyMessage: ""
    property string devicetype: ""
    property string signername: "A B"
    property string signerxfp : "A B"
    property bool   isLocaluser: true
    property string sectionname: ""
    property bool   alreadySigned: false
    property bool   signerReadyToSign: true
    property int    signerType: -1
    property int    tx_status: -1
    property string card_id: ""
    signal signRequest()
    signal scanRequest()

    Column {
        id: columnItem
        spacing: 4
        width: parent.width
        Row {
            id: roomDelegateContent
            spacing: 8
            Rectangle {
                id: _keyIcon
                width: 48
                height: 48
                radius: width
                color: "#F5F5F5"
                anchors.verticalCenter: parent.verticalCenter
                QImage {
                    width: 30
                    height: 30
                    anchors.centerIn: parent
                    source: GlobalData.iconTypes(devicetype,signerType)
                }
            }

            Column {
                width: 170
                spacing: 4
                anchors.verticalCenter: parent.verticalCenter
                QText{
                    width: parent.width
                    height: 28
                    font.family: "Lato"
                    text: signername
                    font.pixelSize: 16
                    color: "#031F2B"
                    elide: Text.ElideRight
                }
                Rectangle {
                    width: typesigner.width + 10
                    height: 16
                    color: "#EAEAEA"
                    visible: isLocaluser && (signerType === NUNCHUCKTYPE.AIRGAP ||
                                             signerType === NUNCHUCKTYPE.SOFTWARE ||
                                             signerType === NUNCHUCKTYPE.COLDCARD_NFC ||
                                             signerType === NUNCHUCKTYPE.NFC)
                    radius: 8
                    QText {
                        id: typesigner
                        font.family: "Lato"
                        color: "#031F2B"
                        font.pixelSize: 10
                        anchors.centerIn: parent
                        font.weight: Font.Bold
                        text: GlobalData.signers(signerType)
                    }
                }
                QText {
                    height: 16
                    visible: (signerxfp !== "" || card_id !== "") && signerType !== NUNCHUCKTYPE.SERVER
                    font.family: "Lato"
                    font.pixelSize: 12
                    color: "#031F2B"
                    text: {
                        if (signerType === NUNCHUCKTYPE.NFC) {
                            var textR = card_id.substring(card_id.length - 5,card_id.length).toUpperCase()
                            return "Card ID: .." + textR
                        } else {
                            return "XFP: " + signerxfp.toUpperCase()
                        }
                    }
                }
                QText {
                    height: 16
                    visible: serverkeyMessage !== "" && signerType === NUNCHUCKTYPE.SERVER
                    font.family: "Lato"
                    font.pixelSize: 12
                    color: "#A66800"
                    text:  serverkeyMessage
                }
            }
            Loader {
                width: 80
                height: parent.height
                sourceComponent: {
                    if(alreadySigned ) return signedComp
                    else{
                        if(AppModel.transactionInfo.status !== NUNCHUCKTYPE.PENDING_SIGNATURES || !isLocaluser) return null;
                        else{
                            if(signerType === NUNCHUCKTYPE.AIRGAP
                              || signerType === NUNCHUCKTYPE.FOREIGN_SOFTWARE
                              || signerType === NUNCHUCKTYPE.NFC) return helpComp;
                            else if(signerType === NUNCHUCKTYPE.SERVER) return null;
                            else{
                                if(signerReadyToSign) return requiredSignature
                                else requiredScan
                            }
                        }
                    }
                }
            }
        }
        Row {
            visible: (AppModel.transactionInfo.status !== NUNCHUCKTYPE.PENDING_SIGNATURES) ?
                         false : !alreadySigned && !signerReadyToSign && (signerType === NUNCHUCKTYPE.HARDWARE)
            height: 16
            spacing: 4
            QImage {
                width: 16
                height: 16
                source: "qrc:/Images/Images/OnlineMode/warning_amber-24px 2.png"
                anchors.verticalCenter: parent.verticalCenter
            }
            QText {
                font.family: "Lato"
                color: "#A66800"
                font.pixelSize: 16
                text: STR.STR_QML_506
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
    Component {
        id: helpComp
        Item {
            QTooltip {
                width: 24
                height: 24
                tipWidth: 270
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                source: "qrc:/Images/Images/OnlineMode/help_outline-24px.png"
                toolTip: {
                    switch(signerType){
                    case NUNCHUCKTYPE.AIRGAP: return STR.STR_QML_507
                    case NUNCHUCKTYPE.NFC: return STR.STR_QML_688
                    default: return STR.STR_QML_508
                    }
                }
                rightOfParent: true
            }
        }
    }
    Component {
        id: signedComp
        Item {
            QImage {
                width: 24
                height: 24
                source: "qrc:/Images/Images/OnlineMode/check_circle_24px_n.png"
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
            }
        }
    }
    Component {
        id: requiredSignature
        Item {
            QTextButton {
                width: 57
                height: 36
                label.text: STR.STR_QML_509
                label.font.pixelSize: 12
                label.font.family: "Lato"
                type: eTypeE
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                onButtonClicked: { signRequest() }
            }
        }
    }
    Component {
        id: requiredScan
        Item {
            QRefreshButton {
                width: 70
                height: 36
                iconSize: 18
                iconSpacing: 4
                label: STR.STR_QML_510
                fontPixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                onButtonClicked: { scanRequest() }
            }
        }
    }
}