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
import QtQuick 2.4
import HMIEVENTS 1.0
import "../origins"
import "../../../localization/STR_QML.js" as STR

Rectangle {
    id: homestep1
    width: 976
    height: 910
    color: "#F1FAFE"
    QAddWelcome{
        anchors.fill: parent
        btnTextLink: STR.STR_QML_021
        titleSuggest: STR.STR_QML_616
        titleWelcome: STR.STR_QML_463
        content: STR.STR_QML_617
        contentHeight: 152
        icon:"qrc:/Images/Images/wallet.svg"
        onAddButtonClicked: {
            QMLHandle.sendEvent(EVT.EVT_HOME_ADD_WALLET_REQUEST)
        }
    }
}
