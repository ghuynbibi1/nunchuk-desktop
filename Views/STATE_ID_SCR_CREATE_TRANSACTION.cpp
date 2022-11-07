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
#include "STATE_ID_SCR_CREATE_TRANSACTION.h"
#include "QQuickViewer.h"
#include "Models/AppModel.h"
#include "bridgeifaces.h"
#include "Chats/matrixbrigde.h"
#include "Chats/ClientController.h"
#include "localization/STR_CPP.h"

void SCR_CREATE_TRANSACTION_Entry(QVariant msg) {
    AppModel::instance()->startGetEstimatedFee();
    Q_UNUSED(msg);
}

void SCR_CREATE_TRANSACTION_Exit(QVariant msg) {
    AppModel::instance()->setTxidReplacing("");
}

void EVT_CREATE_TRANSACTION_SIGN_REQUEST_HANDLER(QVariant msg) {
    bool subtractFromFeeAmout = msg.toMap().value("subtractFromFeeAmout").toBool();
    qint64 feeRate = msg.toMap().value("feeRate").toDouble()*1000; // Convert sats/Byte to sats/kB
    bool manualFee = msg.toMap().value("manualFee").toBool();
    bool manualOutput = msg.toMap().value("manualOutput").toBool();
    if(!manualFee) feeRate = -1;

    if(AppModel::instance()->transactionInfo()){
        QString wallet_id = AppModel::instance()->transactionInfo()->walletId();
        if(QQuickViewer::instance()->onsRequester() == E::STATE_ID_SCR_TRANSACTION_INFO){
            DBG_INFO << "REPLACE BY FEE REQUEST";
            if(AppModel::instance()->getTxidReplacing() != ""){
                QWarningMessage msgwarning;
                QTransactionPtr trans = bridge::nunchukReplaceTransaction(wallet_id,
                                                                          AppModel::instance()->getTxidReplacing(),
                                                                          feeRate,
                                                                          msgwarning);
                if((int)EWARNING::WarningType::NONE_MSG == msgwarning.type()){
                    if(trans){
                        trans.data()->setIsReceiveTx(false);
                        AppModel::instance()->setTransactionInfo(trans);
                        wallet_id = AppModel::instance()->transactionInfo()->walletId();
                        AppModel::instance()->startGetTransactionHistory(wallet_id);
                        AppModel::instance()->startGetUsedAddresses(wallet_id);
                        AppModel::instance()->startGetUnusedAddresses(wallet_id);
                        QQuickViewer::instance()->sendEvent(E::EVT_CREATE_TRANSACTION_SIGN_SUCCEED);
                        AppModel::instance()->setTxidReplacing("");
                    }
                }
                else{
                    AppModel::instance()->showToast(msgwarning.code(),
                                                    msgwarning.what(),
                                                    (EWARNING::WarningType)msgwarning.type(),
                                                    STR_CPP_075);
                }
            }
        }
        else{
            DBG_INFO << "CREATE NEW TRANSACTION" << "subtract:" << subtractFromFeeAmout << "| manual Output:"
                     << manualOutput << "| manual Fee:" << manualFee << "| free rate:" << feeRate;
            QUTXOListModelPtr inputs = NULL;
            if(true == manualOutput){
                inputs = QUTXOListModelPtr(new UTXOListModel());
                if(AppModel::instance()->utxoList()){
                    for (int i = 0; i < AppModel::instance()->utxoList()->rowCount(); i++) {
                        QUTXOPtr it = AppModel::instance()->utxoList()->getUTXOByIndex(i);
                        if(it.data() && it.data()->selected()){
                            DBG_INFO << "UTXO Selected:" << it.data()->txid() << it.data()->amountSats();
                            inputs->addUTXO(it.data()->txid(),
                                            it.data()->vout(),
                                            it.data()->address(),
                                            it.data()->amountSats(),
                                            it.data()->height(),
                                            it.data()->memo());
                        }
                    }
                }
            }
            QMap<QString, qint64> outputs;
            if(AppModel::instance()->destinationList()){
                outputs = AppModel::instance()->destinationList()->getOutputs();
            }
            QString memo = "";
            if(AppModel::instance()->transactionInfo()){
                memo = AppModel::instance()->transactionInfo()->memo();
            }
            if(AppModel::instance()->walletInfo()->isSharedWallet() || (int)ENUNCHUCK::TabSelection::CHAT_TAB == AppModel::instance()->tabIndex() ){
                QString room_id = CLIENT_INSTANCE->getRoomIdByWalletId(wallet_id);
                if(room_id != ""){
                    QWarningMessage msginit;
                    QNunchukMatrixEvent evtmatrix = matrixbrigde::InitTransaction(room_id,
                                                                                  outputs,
                                                                                  memo,
                                                                                  inputs,
                                                                                  feeRate,
                                                                                  subtractFromFeeAmout,
                                                                                  msginit);
                    if((int)ENUNCHUCK::TabSelection::CHAT_TAB == AppModel::instance()->tabIndex()){
                        QQuickViewer::instance()->sendEvent(E::EVT_ONLINE_ONS_CLOSE_REQUEST, E::STATE_ID_SCR_TRANSACTION_INFO);
                    }
                    else{
                        QQuickViewer::instance()->sendEvent(E::EVT_ONS_CLOSE_REQUEST, E::STATE_ID_SCR_TRANSACTION_INFO);
                    }
                    if((int)EWARNING::WarningType::NONE_MSG == msginit.type()){
                        AppModel::instance()->startGetTransactionHistory(wallet_id);
                        AppModel::instance()->startGetUsedAddresses(wallet_id);
                        AppModel::instance()->startGetUnusedAddresses(wallet_id);
                    }
                    else{
                        AppModel::instance()->showToast(msginit.code(),
                                                        msginit.what(),
                                                        (EWARNING::WarningType)msginit.type(),
                                                        STR_CPP_074);
                    }
                }
            }
            else{
                QWarningMessage msgwarning;
                QTransactionPtr trans = bridge::nunchukCreateTransaction(wallet_id,
                                                                         outputs,
                                                                         memo,
                                                                         inputs,
                                                                         feeRate,
                                                                         subtractFromFeeAmout,
                                                                         msgwarning);
                if((int)EWARNING::WarningType::NONE_MSG == msgwarning.type()){
                    if(trans){
                        trans.data()->setIsReceiveTx(false);
                        AppModel::instance()->setTransactionInfo(trans);
                        wallet_id = AppModel::instance()->transactionInfo()->walletId();
                        AppModel::instance()->startGetTransactionHistory(wallet_id);
                        AppModel::instance()->startGetUsedAddresses(wallet_id);
                        AppModel::instance()->startGetUnusedAddresses(wallet_id);
                        QQuickViewer::instance()->sendEvent(E::EVT_CREATE_TRANSACTION_SIGN_SUCCEED);
                    }
                }
                else{
                    AppModel::instance()->showToast(msgwarning.code(),
                                                    msgwarning.what(),
                                                    (EWARNING::WarningType)msgwarning.type(),
                                                    STR_CPP_073);
                }
            }
        }
    }
}

void EVT_CREATE_TRANSACTION_BACK_REQUEST_HANDLER(QVariant msg) {

}

void EVT_CREATE_TRANSACTION_SAVE_REQUEST_HANDLER(QVariant msg) {
    QString file_path = qUtils::QGetFilePath(msg.toString());
    if (file_path != ""){
        QWarningMessage msgwarning;
        bool ret = bridge::nunchukExportTransaction(AppModel::instance()->walletInfo()->id(),
                                                    AppModel::instance()->transactionInfo()->txid(),
                                                    file_path,
                                                    msgwarning);
        if((int)EWARNING::WarningType::NONE_MSG != msgwarning.type()){
            AppModel::instance()->showToast(msgwarning.code(),
                                            msgwarning.what(),
                                            (EWARNING::WarningType)msgwarning.type(),
                                            STR_CPP_076);
        }
        DBG_INFO << file_path << ret;
    }
}

void EVT_CREATE_TRANSACTION_UTXO_SORT_REQUEST_HANDLER(QVariant msg) {
    int sortRole = msg.toMap().value("sortRole").toInt();
    int sortOrder = msg.toMap().value("sortOrder").toInt();

    if(AppModel::instance()->utxoList()){
        AppModel::instance()->utxoList()->requestSort(sortRole, sortOrder);
    }
}

void EVT_CREATE_TRANSACTION_MAKE_DRAFT_TX_HANDLER(QVariant msg) {
    bool subtractFromFeeAmout = msg.toMap().value("subtractFromFeeAmout").toBool();
    int feeRate = msg.toMap().value("feeRate").toDouble()*1000; // Convert sats/Byte to sats/kB
    bool manualFee = msg.toMap().value("manualFee").toBool();
    bool manualOutput = msg.toMap().value("manualOutput").toBool();
    if(!manualFee) feeRate = -1;
    DBG_INFO << "subtract:" << subtractFromFeeAmout << "| manual Output:" << manualOutput << "| manual Fee:" << manualFee << "| free rate:" << feeRate;
    QUTXOListModelPtr inputs = NULL;
    if(true == manualOutput){
        inputs = QUTXOListModelPtr(new UTXOListModel());
        if(AppModel::instance()->utxoList()){
            for (int i = 0; i < AppModel::instance()->utxoList()->rowCount(); i++) {
                QUTXOPtr it = AppModel::instance()->utxoList()->getUTXOByIndex(i);
                if(it.data() && it.data()->selected()){
                    DBG_INFO << "UTXO Selected:" << it.data()->txid() << it.data()->amountSats();
                    inputs->addUTXO(it.data()->txid(),
                                    it.data()->vout(),
                                    it.data()->address(),
                                    it.data()->amountSats(),
                                    it.data()->height(),
                                    it.data()->memo());
                }
            }
        }
    }

    QMap<QString, qint64> outputs;
    if(AppModel::instance()->destinationList()){
        outputs = AppModel::instance()->destinationList()->getOutputs();
    }

    QString wallet_id = "";
    if(AppModel::instance()->walletInfo()){
        wallet_id = AppModel::instance()->walletInfo()->id();
    }

    QWarningMessage msgwarning;
    QTransactionPtr trans = bridge::nunchukDraftTransaction(wallet_id, outputs, inputs, feeRate, subtractFromFeeAmout, msgwarning);
    if((int)EWARNING::WarningType::NONE_MSG == msgwarning.type()){
        if(trans){
            if(AppModel::instance()->transactionInfo()){
                trans.data()->setMemo(AppModel::instance()->transactionInfo()->memo());
                if(QQuickViewer::instance()->onsRequester() == E::STATE_ID_SCR_TRANSACTION_INFO){
                    DBG_INFO << "REPLACE BY FEE, KEEP ORIGIN FEE";
                    trans.data()->setFee(AppModel::instance()->transactionInfo()->feeSats());
                }
            }
            trans.data()->setIsReceiveTx(false);
            AppModel::instance()->setTransactionInfo(trans);
        }
    }
    else{
        AppModel::instance()->showToast(msgwarning.code(),
                                        msgwarning.what(),
                                        (EWARNING::WarningType)msgwarning.type(),
                                        STR_CPP_073);
    }
}

void EVR_CREATE_TRANSACTION_BACK_UTXO_CONSILIDATE_HANDLER(QVariant msg) {

}

void EVT_CREATE_TRANSACTION_SIGN_SUCCEED_HANDLER(QVariant msg) {

}


