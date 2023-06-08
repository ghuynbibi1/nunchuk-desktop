/************************************************************************* 
* This file is part of the Nunchuk software (https://nunchuk.io/)        * 
* Copyright (C) 2020-2022 Enigmo                                         * 
* Copyright (C) 2022 Nunchuk                                             * 
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


#include "STATE_ID_SCR_SELECT_WALLET_CO_SIGN_POLICE.h"
#include "Chats/QUserWallets.h"

void SCR_SELECT_WALLET_CO_SIGN_POLICE_Entry(QVariant msg) {

}

void SCR_SELECT_WALLET_CO_SIGN_POLICE_Exit(QVariant msg) {

}

void EVT_SELECT_WALLET_REQUEST_HANDLER(QVariant msg) {
    QString wallet_id = msg.toString();
    QUserWallets::instance()->serverKeyGetCurrentPolicies(wallet_id);
    QQuickViewer::instance()->sendEvent(E::EVT_CLOSE_TO_SERVICE_SETTINGS_REQUEST, E::STATE_ID_SCR_SELECT_WALLET_CO_SIGN_POLICE);
}

