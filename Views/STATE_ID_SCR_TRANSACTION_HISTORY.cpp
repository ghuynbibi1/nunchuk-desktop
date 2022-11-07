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
#include "STATE_ID_SCR_TRANSACTION_HISTORY.h"
#include "QQuickViewer.h"
#include "Models/AppModel.h"
#include "Models/SingleSignerModel.h"
#include "Models/WalletModel.h"
#include "bridgeifaces.h"

void SCR_TRANSACTION_HISTORY_Entry(QVariant msg) {

}

void SCR_TRANSACTION_HISTORY_Exit(QVariant msg) {

}

void EVT_TRANSACTION_HISTORY_BACK_REQUEST_HANDLER(QVariant msg) {

}

void EVT_TRANSACTION_INFO_ITEM_SELECTED_HANDLER(QVariant msg) {
    if(msg.toString() != "" && AppModel::instance()->walletInfo() && AppModel::instance()->walletInfo()->transactionHistory()){
        QTransactionPtr it = AppModel::instance()->walletInfo()->transactionHistory()->getTransactionByTxid(msg.toString());
        AppModel::instance()->setTransactionInfo(it);
    }
}

void EVT_TRANSACTION_HISTORY_SORT_REQUEST_HANDLER(QVariant msg) {
    int sortRole = msg.toMap().value("sortRole").toInt();
    int sortOrder = msg.toMap().value("sortOrder").toInt();
    if(AppModel::instance()->walletInfo() && AppModel::instance()->walletInfo()->transactionHistory()){
        AppModel::instance()->walletInfo()->transactionHistory()->requestSort(sortRole, sortOrder);
    }
}


