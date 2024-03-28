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
#include "TransactionModel.h"
#include "AppSetting.h"
#include "QtGui/qclipboard.h"
#include "qUtils.h"
#include "QOutlog.h"
#include "bridgeifaces.h"
#include "Chats/matrixbrigde.h"
#include "AppModel.h"
#include <QQmlEngine>
#include <nunchukmatrix.h>
#include "Servers/Byzantine.h"
#include "Premiums/QGroupDashboard.h"

Destination::Destination(): address_(""), amount_(0){
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

Destination::Destination(const QString &address, const qint64 amount) : address_(address), amount_(amount){
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

Destination::~Destination() {}

QString Destination::amountDisplay() const
{
    if((int)AppSetting::Unit::SATOSHI == AppSetting::instance()->unit()){
        QLocale locale(QLocale::English);
        return locale.toString(amountSats());
    }
    else{
        return amountBTC();
    }
}

QString Destination::amountBTC() const
{
    return qUtils::QValueFromAmount(amount_);
}

QString Destination::amountCurrency() const
{
    return qUtils::currencyLocale(amountSats());
}

qint64 Destination::amountSats() const
{
    return amount_;
}

QString Destination::address() const {
    return address_;
}

void Destination::setAddress(const QString &value){
    if(value != address_){
        address_ = value;
        emit addressChanged();
    }
}

void Destination::setAmount(const qint64 value){
    if(value != amount_){
        amount_ = value;
        emit amountChanged();
    }
}

DestinationListModel::DestinationListModel(){
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

DestinationListModel::~DestinationListModel(){m_data.clear();}

int DestinationListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_data.count();
}

QVariant DestinationListModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case destination_address_role:
        return m_data[index.row()]->address();
    case destination_amount_role:
        return m_data[index.row()]->amountDisplay();
    case destination_amount_btc_role:
        return m_data[index.row()]->amountBTC();
    case destination_amount_currency_role:
        return m_data[index.row()]->amountCurrency();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DestinationListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[destination_address_role] = "destination_address";
    roles[destination_amount_role] = "destination_amount";
    roles[destination_amount_btc_role] = "destination_amount_btc";
    roles[destination_amount_currency_role] = "destination_amount_currency";
    return roles;
}

void DestinationListModel::addDestination(const QString &address, const qint64 amount) {
    qint64 am = 0;
    if(!contains(address, am)){
        beginResetModel();
        m_data.append(QDestinationPtr(new Destination(address, amount)));
        endResetModel();
    }
}

QMap<QString, qint64> DestinationListModel::getOutputs() const
{
    QMap<QString, qint64> outputs;
    foreach (QDestinationPtr i , m_data ){
        outputs[i->address()] = i->amountSats();
    }
    return outputs;
}

bool DestinationListModel::contains(const QString &address, qint64 &amount)
{
    foreach (QDestinationPtr i , m_data ){
        if(address == i.data()->address()){
            amount = i.data()->amountSats();
            return true;
        }
    }
    return false;
}

qint64 DestinationListModel::getAmountByAddress(const QString &address)
{
    foreach (QDestinationPtr i , m_data ){
        if(address == i.data()->address()){
            return i.data()->amountSats();
        }
    }
    return 0;
}

void DestinationListModel::notifyUnitChanged()
{
    beginResetModel();
    foreach (QDestinationPtr it, m_data) {
        if(it.data()){
            it.data()->amountChanged();
        }
    }
    endResetModel();
}

QString DestinationListModel::reciever()
{
    if(m_data.count() > 2){
        return "Multiple addresses";
    }
    else{
        return (m_data.count() == 0 ? "unknown" : m_data.at(0).data()->address());
    }
}

void DestinationListModel::clearAll()
{
    m_data.clear();
}

Transaction::Transaction() :
    m_destinations(QDestinationListModelPtr(new DestinationListModel())),
    m_signers(QSingleSignerListModelPtr(new (SingleSignerListModel))),
    m_change(QDestinationPtr(new Destination())),
    m_walletId(""),
    m_roomId(""),
    m_initEventId(""),
    m_createByMe(true),
    m_serverKeyMessage("")
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

Transaction::~Transaction()
{
    m_destinations.clear();
    m_signers.clear();
    m_change.clear();
}

bool Transaction::isReceiveTx() const
{
    return m_transaction.is_receive();
}

void Transaction::setIsReceiveTx(bool receive)
{
    m_transaction.set_receive(receive);
    emit isReceiveTxChanged();
}

QString Transaction::txid() const {
    return QString::fromStdString(m_transaction.get_txid());
}

QString Transaction::memo() const {
    return QString::fromStdString(m_transaction.get_memo());;
}

void Transaction::setMemo(const QString &memo)
{
    m_transaction.set_memo(memo.toStdString());
    emit memoChanged();
}

int Transaction::status() const {
    return (int)m_transaction.get_status();
}

void Transaction::setStatus(int status)
{
    m_transaction.set_status((nunchuk::TransactionStatus)status);
    emit statusChanged();
}

int Transaction::m() const {
    return m_transaction.get_m();
}

int Transaction::height() const
{
    return m_transaction.get_height();
}

void Transaction::setHeight(const int value){
    m_transaction.set_height(value);
    emit heightChanged();
}

QString Transaction::feeDisplay() const
{
    if((int)AppSetting::Unit::SATOSHI == AppSetting::instance()->unit()){
        QLocale locale(QLocale::English);
        return locale.toString(feeSats());
    }
    else{
        return feeBTC();
    }
}

QString Transaction::feeBTC() const
{
    return qUtils::QValueFromAmount(feeSats());
}

QString Transaction::feeCurrency() const
{
    return qUtils::currencyLocale(feeSats());
}

qint64 Transaction::feeSats() const
{
    return m_transaction.get_fee();
}

void Transaction::setFee(const qint64 fee)
{
    m_transaction.set_fee(fee);
    emit feeChanged();
}

bool Transaction::subtractFromFeeAmount() const
{
    return m_transaction.subtract_fee_from_amount();
}

QString Transaction::feeRate() const
{
    quint32 rate = m_transaction.get_fee_rate();
    return QString::number((double)rate/1000, 'f', 2);
}

QString Transaction::psbt() const
{
    return QString::fromStdString(m_transaction.get_psbt());
}

time_t Transaction::scheduleTime(){
    return m_transaction.get_schedule_time();
}

bool Transaction::isCosigning() const
{
    return m_is_cosigning;
}

void Transaction::setIsCosigning(bool is_cosigning)
{
    m_is_cosigning = is_cosigning;
}

int Transaction::scan_percent() const
{
    return m_scan_percent;
}

void Transaction::setScan_percent(int scan_percent)
{
    if (m_scan_percent == scan_percent) return;
    m_scan_percent = scan_percent;
    emit scan_percentChanged();
}

bool Transaction::parseQRTransaction(const QStringList &qrtags)
{
    QStringList in = qrtags;
    in.removeDuplicates();
    if(in.isEmpty()){
        return false;
    }
    QString wallet_id = walletId();
    if(ImportQRTransaction(in)){
        AppModel::instance()->requestSyncWalletDb(wallet_id);
        AppModel::instance()->transactionInfoChanged();
        return true;
    }
    return false;
}

bool Transaction::parseQRSuccess(const QStringList &qrtags)
{
    nunchuk::AnalyzeQRResult progress = qUtils::AnalyzeQR(qrtags);
    setScan_percent(progress.estimated_percent_complete * 100);
    return progress.is_complete;
}

void Transaction::copyTransactionID()
{
    qApp->clipboard()->setText(txid());
}

void Transaction::requestSignatures(const QString &membership_id)
{
    QJsonObject data;
    QString errormsg = "";
    QString groupid = AppModel::instance()->walletInfo() ? AppModel::instance()->walletInfo()->groupId() : "";
    DBG_INFO << groupid << walletId() << txid() << membership_id;
    bool ret = Byzantine::instance()->requestSignature(groupid, walletId(), txid(), membership_id, data, errormsg);
    if(ret){
        AppModel::instance()->showToast(0, "Signature request sent", EWARNING::WarningType::SUCCESS_MSG);
    }
}

void Transaction::scheduleBroadcast()
{

}

void Transaction::cancelTransaction()
{

}

bool Transaction::ImportQRTransaction(const QStringList& qrtags)
{
    QStringList in = qrtags;
    in.removeDuplicates();
    QString wallet_id = walletId();
    QWarningMessage msg;
    nunchuk::Transaction trans_result = bridge::nunchukImportQRTransaction(wallet_id, in, msg);
    if((int)EWARNING::WarningType::NONE_MSG == msg.type()){
        setNunchukTransaction(trans_result);
        if (roomId() != "") {
            QWarningMessage room_msg;
            nunchuk::RoomTransaction room_tx = matrixbrigde::GetOriginPendingTransaction(roomId(), txid(), room_msg);
            if((int)EWARNING::WarningType::NONE_MSG == room_msg.type()){
                setInitEventId(QString::fromStdString(room_tx.get_init_event_id()));
            }

            QWarningMessage sign_msg;
            auto signers = singleSignersAssigned();
            for(QSingleSignerPtr signer : signers->fullList()){
                if(signer->signerType() == (int)ENUNCHUCK::SignerType::AIRGAP
                        && signer->signerSigned()
                        && signers->containsFingerPrint(signer->masterFingerPrint())
                        && initEventId() != ""){
                    matrixbrigde::SignAirgapTransaction(initEventId(),
                                                        signer->masterFingerPrint(),
                                                        sign_msg);
                }
            }
        }
        return true;
    }
    else{
        return false;
    }
}

void Transaction::setTxJson(const QJsonObject &txJs)
{
    m_txJson = txJs;
}

void Transaction::setSignatures(const QMap<QString, QString> &signatures)
{
    m_signatures = signatures;
}

QMap<QString, QString> Transaction::signatures() const
{
    return m_signatures;
}

bool Transaction::hasMoreBtn() const
{
    return m_hasMoreBtn;
}

void Transaction::setHasMoreBtn(bool has)
{
    if (m_hasMoreBtn == has)
        return;

    m_hasMoreBtn = has;
    emit hasMoreBtnChanged();
}

QString Transaction::dummyXfp() const
{
    return {};
}

bool Transaction::isDummyTx() const
{
    return false;
}

bool Transaction::enableRequestSignature()
{
    if(AppModel::instance()->walletInfo()){
        QString groupid = AppModel::instance()->walletInfo()->groupId();
        if(groupid != "" && AppModel::instance()->walletInfo()->dashboard()){
            QString role = AppModel::instance()->walletInfo()->dashboard()->myRole();
            DBG_INFO << !qUtils::strCompare(role, "OBSERVER");
            return (!qUtils::strCompare(role, "OBSERVER"));
        }
    }
    return false;
}

bool Transaction::enableScheduleBroadcast()
{
    return false;
}

bool Transaction::enableCancelTransaction()
{
    if(AppModel::instance()->walletInfo() && AppModel::instance()->walletInfo()->isAssistedWallet()){
        QString role = AppModel::instance()->walletInfo()->dashboard() ? AppModel::instance()->walletInfo()->dashboard()->myRole() : "";
        if(role == ""){
            return false;
        }
        else{
            return !(qUtils::strCompare(role, "OBSERVER"));
        }
    }
    return true;
}

bool Transaction::hasChange() const {
    int index_change = m_transaction.get_change_index();
    if(index_change >= 0 && index_change < (int)m_transaction.get_outputs().size()) {
        return true;
    }
    else{
        return false;
    }
}

Destination *Transaction::change() {
    if(!m_change){
        m_change = QDestinationPtr(new Destination());
    }
    int index_change = m_transaction.get_change_index();
    if(index_change >= 0 && index_change < (int)m_transaction.get_outputs().size()) {
        std::pair<std::string, int> change_ret = m_transaction.get_outputs().at(index_change);
        m_change.data()->setAddress(QString::fromStdString(change_ret.first));
        m_change.data()->setAmount(change_ret.second);
    }
    return m_change.data();
}

QString Transaction::blocktimeDisplay() const
{
    if(0 >= m_transaction.get_blocktime()){
        return "--/--/----"; // There is no time
    }
    else{
        return QDateTime::fromTime_t(m_transaction.get_blocktime()).toString( "MM/dd/yyyy hh:mm AP");
    }
}

time_t Transaction::blocktime() const
{
    return m_transaction.get_blocktime();
}

QString Transaction::walletId() const
{
    return m_walletId;
}

void Transaction::setWalletId(const QString &walletId)
{
    if(m_walletId != walletId){
        m_walletId = walletId;
        emit walletIdChanged();
    }
}

SingleSignerListModel *Transaction::singleSignersAssigned() {
    if(m_signers){
        QWalletPtr wallet = AppModel::instance()->walletList()->getWalletById(walletId());
        if(wallet && wallet->singleSignersAssigned()){
            setRoomId(wallet->roomId());
            if(wallet->isSharedWallet() && wallet->singleSignersAssigned()->needSyncNunchukEmail()){
                wallet->syncCollabKeyname();
            }
            if(m_signers.data()->rowCount() == 0){
                m_signers = wallet->singleSignersAssigned()->clone();
            }
            else{
                if(m_signers->needSyncNunchukEmail()){
                    m_signers = wallet->singleSignersAssigned()->clone();
                }
            }
        }
        m_signers.data()->initSignatures();
        std::map<std::string, bool> signers = m_transaction.get_signers();
        for ( std::map<std::string, bool>::iterator it = signers.begin(); it != signers.end(); it++ ){
            m_signers.data()->updateSignatures(QString::fromStdString(it->first), it->second, "");
        }
        DBG_INFO << isDummyTx() << m_txJson;
        if (!m_txJson.isEmpty() || isDummyTx()) {// Use for dummy transaction
            if (!m_txJson.isEmpty()) {
                QJsonArray signatures = m_txJson["signatures"].toArray();
                for (auto js : signatures) {
                    QString xfp = js.toObject()["xfp"].toString();
                    QString signature = js.toObject()["signature"].toString().split(".").at(1);
                    m_signers.data()->updateSignatures(xfp, true, signature);
                }
            } else if (m_signatures.size() > 0) {
                for (auto xfp : m_signatures.uniqueKeys()) {
                    QString signature = m_signatures.value(xfp);
                    m_signers.data()->updateSignatures(xfp, true, signature);
                }
            }
            for (int i = 0; i < m_signers->rowCount(); i++) {
                auto signer = m_signers->getSingleSignerByIndex(i);
                signer->setHasSignBtn(true);
                if (signer->signerType() == (int)ENUNCHUCK::SignerType::SERVER) {
                    m_signers->removeSingleSignerByIndex(i);
                    --i;
                }
                if (!dummyXfp().isEmpty()) {
                    if (dummyXfp() != signer->masterFingerPrint() ) {
                        signer->setHasSignBtn(false);
                    }
                }
            }
        }
        m_signers.data()->requestSort();
    }
    return m_signers.data();
}

int Transaction::numberSigned()
{
    int number_signed = 0;
    std::map<std::string, bool> signers = m_transaction.get_signers();
    for ( std::map<std::string, bool>::iterator it = signers.begin(); it != signers.end(); it++ ){
        if(it->second) { number_signed++; }
    }

    for (std::pair<std::string, nunchuk::Amount> item : m_transaction.get_inputs()) {
        if(AppModel::instance()->utxoList()){
            AppModel::instance()->utxoList()->updateSelected(QString::fromStdString(item.first), item.second);
        }
    }
    return number_signed;
}

QString Transaction::subtotalCurrency() const
{
    return qUtils::currencyLocale(subtotalSats());
}

QString Transaction::subtotalDisplay() const
{
    if((int)AppSetting::Unit::SATOSHI == AppSetting::instance()->unit()){
        QLocale locale(QLocale::English);
        return locale.toString(subtotalSats());
    }
    else{
        return subtotalBTC();
    }
}

qint64 Transaction::subtotalSats() const
{
    return m_transaction.get_sub_amount();
}

QString Transaction::subtotalBTC() const
{
    return qUtils::QValueFromAmount(subtotalSats());
}

QString Transaction::totalCurrency() const
{
    return qUtils::currencyLocale(totalSats());
}

QString Transaction::totalDisplay() const
{
    if((int)AppSetting::Unit::SATOSHI == AppSetting::instance()->unit()){
        QLocale locale(QLocale::English);
        return locale.toString(totalSats());
    }
    else{
        return totalBTC();
    }
}

qint64 Transaction::totalSats() const
{
    return (subtotalSats() + feeSats());
}

QString Transaction::totalBTC() const
{
    return qUtils::QValueFromAmount(totalSats());
}

DestinationListModel *Transaction::destinationList() {
    if(!m_destinations){
        m_destinations = QDestinationListModelPtr(new DestinationListModel());
    }
    m_destinations.data()->clearAll();

    std::vector<nunchuk::TxOutput> addrs;
    if(isReceiveTx()){
        addrs = m_transaction.get_receive_outputs();
        for (std::pair<std::string, nunchuk::Amount> item : addrs) {
            m_destinations.data()->addDestination(QString::fromStdString(item.first), item.second);
        }
    }
    else{
        addrs = m_transaction.get_outputs();
        int index_change = m_transaction.get_change_index();
        for (int i = 0; i < (int)addrs.size(); i++){
            if(index_change != i){
                std::pair<std::string, nunchuk::Amount> item = addrs.at(i);
                m_destinations.data()->addDestination(QString::fromStdString(item.first), item.second);
            }
        }
    }
    return m_destinations.data();
}

QString Transaction::get_replaced_by_txid() const
{
    return QString::fromStdString(m_transaction.get_replaced_by_txid());
}

QString Transaction::get_replace_txid()
{
    return QString::fromStdString(m_transaction.get_replace_txid());
}

nunchuk::Transaction Transaction::nunchukTransaction() const
{
    return m_transaction;
}

void Transaction::setNunchukTransaction(const nunchuk::Transaction &tx)
{
    m_transaction = tx;
}

QString Transaction::roomId()
{
    if(AppModel::instance()->walletList()){
        QWalletPtr wallet = AppModel::instance()->walletList()->getWalletById(walletId());
        if(wallet){
            m_roomId = wallet.data()->roomId();
        }
    }
    return m_roomId;
}

void Transaction::setRoomId(const QString &roomId)
{
    if(m_roomId != roomId){
        m_roomId = roomId;
        emit roomIdChanged();
    }
}

QString Transaction::initEventId() const
{
    return m_initEventId;
}

void Transaction::setInitEventId(const QString &initEventId)
{
    if(m_initEventId != initEventId){
        m_initEventId = initEventId;
        emit initEventIdChanged();
    }
}

bool Transaction::createByMe() const
{
    return m_createByMe;
}

void Transaction::setCreateByMe(bool createByMe)
{
    if(m_createByMe != createByMe){
        m_createByMe = createByMe;
        emit createByMeChanged();
    }
}

QString Transaction::serverKeyMessage() const
{
    return m_serverKeyMessage;
}

void Transaction::setServerKeyMessage(const QJsonObject &data)
{
    if (status() == (int)ENUNCHUCK::TransactionStatus::PENDING_SIGNATURES) {
        QJsonObject transaction = data.value("transaction").toObject();
        QJsonObject spending_limit_reached  = transaction.value("spending_limit_reached").toObject();
        double time = transaction.value("sign_time_milis").toDouble();
        if (!spending_limit_reached.isEmpty()) {
            m_serverKeyMessage = spending_limit_reached.value("message").toString();
        }
        else if(time != 0) {
            m_serverKeyMessage = QString("Co-sign at %1").arg(QDateTime::fromMSecsSinceEpoch(time).toString( "hh:mm AP MMM d"));
        }
        else{}
        bool is_cosigning  = transaction.value("is_cosigning").toBool();
        DBG_INFO << "FIXME is_cosigning" << is_cosigning << m_serverKeyMessage;
        setIsCosigning(is_cosigning);
        emit serverKeyMessageChanged();
    }
}

QString Transaction::packageFeeRate()
{
    return QString::number((double)m_packageFeeRate/1000, 'f', 2);
}

void Transaction::setPackageFeeRate(int satvKB)
{
    if (m_packageFeeRate == satvKB)
        return;

    m_packageFeeRate = satvKB;
    emit packageFeeRateChanged();
}

QString Transaction::destination()
{
    QString ret = "";
    std::vector<nunchuk::TxOutput> addrs;
    if(isReceiveTx()){
        addrs = m_transaction.get_receive_outputs();
        ret = addrs.size() > 0 ? QString::fromStdString(addrs.at(0).first) : "";
    }
    else{
        addrs = m_transaction.get_outputs();
        int index_change = m_transaction.get_change_index();
        if(index_change >= 0 && index_change < (int)m_transaction.get_outputs().size()) {
            addrs.erase(addrs.begin() + index_change);
        }
        ret = addrs.size() > 1 ? "Multiple addresses" : addrs.size() > 0 ? QString::fromStdString(addrs.at(0).first) : "";
    }
    return ret;
}

bool Transaction::isCpfp()
{
    bool ret = false;
    QWarningMessage msg;
    nunchuk::Amount packageFeeRate{0};
    if (bridge::IsCPFP(walletId(), nunchukTransaction(), packageFeeRate, msg)) {
        ret = true;
    }
    setPackageFeeRate(packageFeeRate);
    DBG_INFO << ret << packageFeeRate;
    return ret;
}

TransactionListModel::TransactionListModel() {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

TransactionListModel::~TransactionListModel() {m_data.clear();}

int TransactionListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_data.count();
}

QVariant TransactionListModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case transaction_txid_role:
        return m_data[index.row()]->txid();
    case transaction_memo_role:
        return m_data[index.row()]->memo();
    case transaction_status_role:
        return m_data[index.row()]->status();
    case transaction_fee_role:
        return m_data[index.row()]->feeDisplay();
    case transaction_m_role:
        return m_data[index.row()]->m();
    case transaction_hasChange_role:
        return m_data[index.row()]->hasChange();
    case transaction_destinationList_role:
        return qVariantFromValue(m_data[index.row()]->destinationList());
    case transaction_destinationDisp_role:
        return m_data[index.row()]->destination();
    case transaction_change_role:
        return qVariantFromValue(m_data[index.row()]->change());
    case transaction_singleSignersAssigned_role:
        return qVariantFromValue(m_data[index.row()]->singleSignersAssigned());
    case transaction_subtotal_role:
        return m_data[index.row()]->subtotalDisplay();
    case transaction_total_role:
        return m_data[index.row()]->totalDisplay();
    case transaction_numberSigned_role:
        return m_data[index.row()]->numberSigned();
    case transaction_blocktime_role:
        return m_data[index.row()]->blocktimeDisplay();
    case transaction_height_role:
        return m_data[index.row()]->height();
    case transaction_isReceiveTx_role:
        return m_data[index.row()]->isReceiveTx();
    case transaction_replacedTx_role:
        return m_data[index.row()]->get_replaced_by_txid();
    case transaction_subtotalCurrency_role:
        return m_data[index.row()]->subtotalCurrency();
    case transaction_totalCurrency_role:
        return m_data[index.row()]->totalCurrency();
    case transaction_isRbf_role:
        return m_data[index.row()]->get_replace_txid() != "";
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TransactionListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[transaction_txid_role]            = "transaction_txid";
    roles[transaction_memo_role]            = "transaction_memo";
    roles[transaction_status_role]          = "transaction_status";
    roles[transaction_fee_role]             = "transaction_fee";
    roles[transaction_m_role]               = "transaction_m";
    roles[transaction_hasChange_role]       = "transaction_hasChange";
    roles[transaction_destinationList_role] = "transaction_destinationList";
    roles[transaction_destinationDisp_role] = "transaction_destinationDisp_role";
    roles[transaction_change_role]          = "transaction_change_role";
    roles[transaction_singleSignersAssigned_role]  = "transaction_singleSignersAssigned";
    roles[transaction_subtotal_role]        = "transaction_subtotal";
    roles[transaction_total_role]           = "transaction_total";
    roles[transaction_numberSigned_role]    = "transaction_numberSigned";
    roles[transaction_blocktime_role]       = "transaction_blocktime";
    roles[transaction_height_role]          = "transaction_height";
    roles[transaction_isReceiveTx_role]     = "transaction_isReceiveTx";
    roles[transaction_replacedTx_role]      = "transaction_replacedTx";
    roles[transaction_subtotalCurrency_role]= "transaction_subtotalCurrency";
    roles[transaction_totalCurrency_role]   = "transaction_totalCurrency";
    roles[transaction_isRbf_role]           = "transaction_isRbf";
    return roles;
}

QTransactionPtr TransactionListModel::getTransactionByIndex(const int index)
{
    if(index < 0 || index >= m_data.count()){
        DBG_INFO << "Index out of range";
        return NULL;
    }
    else {
        return m_data.at(index);
    }
}

QTransactionPtr TransactionListModel::getTransactionByTxid(const QString &txid)
{
    foreach (QTransactionPtr i , m_data ){
        if(txid == i.data()->txid()){
            return i;
        }
    }
    return NULL;
}

void TransactionListModel::addTransaction(const QTransactionPtr &d){
    if(d){
        if(!contains(d.data()->txid())){
            m_data.append(d);
        }
    }
}

void TransactionListModel::updateTransactionMemo(const QString &tx_id, const QString &memo)
{
    for (int i = 0; i < m_data.count(); i++) {
        if(m_data.at(i) && (0 == QString::compare(tx_id, m_data.at(i).data()->txid(), Qt::CaseInsensitive))){
            m_data.at(i)->setMemo(memo);
            emit dataChanged(index(i),index(i));
        }
    }
}

void TransactionListModel::updateTransaction(const QString &tx_id, const QTransactionPtr &tx)
{
    if(tx){
        bool existed = false;
        for (int i = 0; i < m_data.count(); i++) {
            if(m_data.at(i) && (0 == QString::compare(tx_id, m_data.at(i).data()->txid(), Qt::CaseInsensitive))){
                m_data.at(i)->setNunchukTransaction(tx.data()->nunchukTransaction());
                emit dataChanged(index(i),index(i));
                existed = true;
                break;
            }
        }
        if(!existed){
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            m_data.append(tx);
            endInsertRows();
            emit countChanged();
        }
    }
}

void TransactionListModel::updateTransaction(const QString &wallet_id, std::vector<nunchuk::Transaction> txs)
{
    beginResetModel();
    m_data.clear();
    for (auto it = txs.begin(); it != txs.end(); ++it) {
        const nunchuk::Transaction &element = *it;
        QTransactionPtr tx = bridge::convertTransaction(element, wallet_id);
        m_data.append(tx);
    }
    endResetModel();
    emit countChanged();
}

void TransactionListModel::removeTransaction(const QString &tx_id)
{
    foreach (QTransactionPtr it, m_data) {
        if(it.data() && it.data()->txid() == tx_id){
            beginResetModel();
            m_data.removeAll(it);
            endResetModel();
            emit countChanged();
            break;
        }
    }
}

bool TransactionListModel::contains(const QString &tx_id)
{
    foreach (QTransactionPtr i , m_data ){
        if(0 == QString::compare(tx_id, i.data()->txid(), Qt::CaseInsensitive)){
            return true;
        }
    }
    return false;
}

void TransactionListModel::requestSort(int role, int order)
{
    beginResetModel();
    if(m_data.count() > 1){
        switch (role) {
        case transaction_txid_role:
            break;
        case transaction_memo_role:
        {
            if(Qt::DescendingOrder == order){
                qSort(m_data.begin(), m_data.end(), sortTXsByMemoDescending);
            }
            else{
                qSort(m_data.begin(), m_data.end(), sortTXsByMemoAscending);
            }
        }
            break;
        case transaction_status_role:
        {
            if(Qt::DescendingOrder == order){
                qSort(m_data.begin(), m_data.end(), sortTXsByStatusDescending);
            }
            else{
                qSort(m_data.begin(), m_data.end(), sortTXsByStatusAscending);
            }
        }
            break;
        case transaction_subtotal_role:
        case transaction_total_role:
        {
            if(Qt::DescendingOrder == order){
                qSort(m_data.begin(), m_data.end(), sortTXsByAmountDescending);
            }
            else{
                qSort(m_data.begin(), m_data.end(), sortTXsByAmountAscending);
            }
        }
            break;
        case transaction_blocktime_role:
        {
            if(Qt::DescendingOrder == order){
                qSort(m_data.begin(), m_data.end(), sortTXsByBlocktimeDescending);
            }
            else{
                qSort(m_data.begin(), m_data.end(), sortTXsByBlocktimeAscending);
            }
            linkingReplacedTransactions();
        }
            break;
        default:
            break;
        }
    }
    endResetModel();
}

void TransactionListModel::notifyUnitChanged()
{
    const QVector<int>& roles = {};
    for (int i = 0; i < m_data.count(); i++) {
        if(m_data.at(i).data()){
            const auto idx = index(i);
            emit dataChanged(idx, idx, roles);
        }
    }
}

void TransactionListModel::linkingReplacedTransactions()
{
    QMap<QString, QString> replaces;
    for (int i = 0; i < m_data.count(); i++) {
        if(m_data.at(i) && (int)nunchuk::TransactionStatus::REPLACED == m_data.at(i).data()->status()){
            replaces[m_data.at(i).data()->txid()] = m_data.at(i).data()->get_replaced_by_txid();
        }
    }

    int from_index = -1;
    int to_index = -1;
    for (int j = 0; j < replaces.keys().count(); j++) {
        from_index = -1;
        to_index = -1;
        for (int k = 0; k < m_data.count(); k++) {
            if(m_data.at(k)){
                if(m_data.at(k).data()->txid() == replaces.keys()[j]){ // old tx replaced
                    from_index = k;
                }
                if(m_data.at(k).data()->txid() == replaces[replaces.keys()[j]]){ // Find new tx
                    to_index = k;
                }
            }
        }

        if((-1 != from_index) && (-1 != to_index) && (to_index != from_index)){
            m_data.move(from_index, to_index);
        }
    }
}

void TransactionListModel::cleardata()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
    emit countChanged();
}

int TransactionListModel::count() const
{
    return m_data.size();
}

bool sortTXsByBlocktimeAscending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    if(v1.data()->blocktime() <= 0 && v2.data()->blocktime() <= 0){
        if(v1.data()->status() == v2.data()->status()){
            return (v1.data()->totalSats() > v2.data()->totalSats());
        }
        else if(v1.data()->status() < v2.data()->status()){
            return (true);
        }
        else{
            return v1.data()->blocktime() < v2.data()->blocktime();
        }
    }
    else if(v1.data()->blocktime() <= 0 && v2.data()->blocktime() > 0){ return true;}
    else if(v1.data()->blocktime() > 0 && v2.data()->blocktime() <= 0){ return false;}
    else {return v1.data()->blocktime() < v2.data()->blocktime();}
}

bool sortTXsByBlocktimeDescending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    if(v1.data()->blocktime() <= 0 && v2.data()->blocktime() <= 0){
        if(v1.data()->status() == v2.data()->status()){
            return (v1.data()->totalSats() > v2.data()->totalSats());
        }
        else if(v1.data()->status() < v2.data()->status()){
            return (true);
        }
        else{
            return v1.data()->blocktime() < v2.data()->blocktime();
        }
    }
    else if(v1.data()->blocktime() <= 0 && v2.data()->blocktime() > 0){ return true;}
    else if(v1.data()->blocktime() > 0 && v2.data()->blocktime() <= 0){ return false;}
    else {return v1.data()->blocktime() > v2.data()->blocktime();}
}

bool sortTXsByAmountAscending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    return ((v1.data()->subtotalBTC() + v1.data()->totalBTC()) < (v2.data()->subtotalBTC() + v2.data()->totalBTC()));
}

bool sortTXsByAmountDescending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    return ((v1.data()->subtotalBTC() + v1.data()->totalBTC()) > (v2.data()->subtotalBTC() + v2.data()->totalBTC()));
}

bool sortTXsByMemoAscending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    return (QString::compare((v1.data()->memo()), (v2.data()->memo())) < 0);
}

bool sortTXsByMemoDescending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    return (QString::compare((v1.data()->memo()), (v2.data()->memo())) > 0);
}

bool sortTXsByStatusAscending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    return ((v1.data()->status()) < (v2.data()->status()));
}

bool sortTXsByStatusDescending(const QTransactionPtr &v1, const QTransactionPtr &v2)
{
    return ((v1.data()->status()) > (v2.data()->status()));
}
