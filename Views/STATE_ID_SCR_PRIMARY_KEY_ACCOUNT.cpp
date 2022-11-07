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
#include "STATE_ID_SCR_PRIMARY_KEY_ACCOUNT.h"
#include "Models/AppModel.h"
#include "QQuickViewer.h"
#include "Models/SingleSignerModel.h"
#include "Models/WalletModel.h"
#include "bridgeifaces.h"
#include "Draco.h"
#include "Chats/ClientController.h"
#include "Chats/matrixbrigde.h"

QVariant getPrimaryKey(const nunchuk::PrimaryKey &pkey){
    QVariantMap maps;
    maps["name"] = QString::fromStdString(pkey.get_name());
    maps["master_fingerprint"] = QString::fromStdString(pkey.get_master_fingerprint());
    maps["account"] = QString::fromStdString(pkey.get_account());
    maps["address"] = QString::fromStdString(pkey.get_address());
    return QVariant::fromValue(maps);
}

void SCR_PRIMARY_KEY_ACCOUNT_Entry(QVariant msg) {
    std::vector<nunchuk::PrimaryKey> primaryKeys = qUtils::GetPrimaryKeys(AppSetting::instance()->storagePath(),
                                                                          (nunchuk::Chain)AppSetting::instance()->primaryServer());
    QVariantList primaryKeyList;
    for(nunchuk::PrimaryKey key: primaryKeys){
        primaryKeyList.append(getPrimaryKey(key));
    }
    DBG_INFO << msg << primaryKeyList.size() << primaryKeys.size();
    QObject *obj = QQuickViewer::instance()->getQmlObj().first();
    if(obj){
        obj->setProperty("isSignIn",msg);
        obj->setProperty("primaryKeyList",primaryKeyList);
        obj->setProperty("primaryKeySize",(int)primaryKeys.size());
    }
}

void SCR_PRIMARY_KEY_ACCOUNT_Exit(QVariant msg) {

}


void EVT_PRIMARY_KEY_ACCOUNT_BACK_HANDLER(QVariant msg) {

}

void EVT_ADD_PRIMARY_KEY_REQUEST_HANDLER(QVariant msg) {
    QQuickViewer::instance()->setCurrentFlow((int)ENUNCHUCK::IN_FLOW::FLOW_PRIMARY_KEY);
}

void EVT_SHOW_PRIMARY_KEY_ENTER_PASSPHRASE_REQUEST_HANDLER(QVariant msg) {

}

void EVT_SHOW_SIGN_IN_BY_IMPORTING_THE_PRIMARY_KEY_REQUEST_HANDLER(QVariant msg) {
    QQuickViewer::instance()->setCurrentFlow((int)ENUNCHUCK::IN_FLOW::FLOW_PRIMARY_KEY);
}

void EVT_SCR_SIGN_IN_MANUALLY_REQUEST_HANDLER(QVariant msg) {
    QQuickViewer::instance()->setCurrentFlow((int)ENUNCHUCK::IN_FLOW::FLOW_PRIMARY_KEY);
}

void LOGIN_WITH_SOFTWARE_KEY_BACK_HANDLER(QVariant msg) {

}

void EVT_LOGIN_WITH_SOFTWARE_KEY_REQUEST_HANDLER(QVariant msg) {

}

void EVT_PRIMARY_KEY_BACK_TO_CREATE_ACCOUNT_HANDLER(QVariant msg) {

}

void EVT_PRIMARY_KEY_BACK_TO_SIGN_IN_HANDLER(QVariant msg) {

}


