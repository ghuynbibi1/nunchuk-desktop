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
#include "MasterSignerModel.h"
#include <QQmlEngine>
#include "bridgeifaces.h"

MasterSigner::MasterSigner():
    id_(""),
    name_("UNKNOW"),
    device_(new QDevice()),
    health_(-1),
    messageToSign_(""),
    signature_(""),
    path_(""),
    checked_(false),
    isNeedXpub_(false),
    signer_type_(0),
    passphrase_(""),
    isPrimaryKey_(false)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

MasterSigner::MasterSigner(const QString &id,
                           const QString& name,
                           const QDevicePtr device,
                           const int health,
                           const int signertype,
                           const nunchuk::PrimaryKey &key) :
    id_(id),
    name_(name),
    device_(device),
    health_(health),
    messageToSign_(""),
    signature_(""),
    path_(""),
    checked_(false),
    isNeedXpub_(false),
    signer_type_(signertype),
    isPrimaryKey_(false),
    primaryKey_(key)
{
    if(key.get_master_fingerprint() != ""){
        isPrimaryKey_ = true;
    }
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

MasterSigner::~MasterSigner(){
    device_.clear();
}

QString MasterSigner::id() const{
    return id_;
}

QString MasterSigner::name() const { return name_;}

QDevice *MasterSigner::device() const {
    return device_.data();
}

QDevicePtr MasterSigner::devicePtr() const
{
    return device_;
}

int MasterSigner::health() const {return health_;}

bool MasterSigner::connected() const
{
    if(device_.data()){
        return device_.data()->connected();
    }
    return false;
}

void MasterSigner::setId(const QString &d){
    if(d != id_){
        id_ = d;
        emit idChanged();
    }
}

void MasterSigner::setName(const QString &d) {
    if(d != name_){
        name_ = d;
        emit nameChanged();
    }
}

void MasterSigner::setDevice(const QDevicePtr &d) {
    device_ = d;
    emit deviceChanged();
    emit fingerPrintChanged();
}

void MasterSigner::setHealth(const int d)
{
    if(d != health_){
        health_ = d;
        emit healthChanged();
    }
}

bool MasterSigner::checked() const
{
    return checked_;
}

void MasterSigner::setChecked(const bool checked)
{
    if(checked != checked_){
        checked_ = checked;
        emit checkedChanged();
    }
}

bool MasterSigner::needPinSent() const
{
    return device_ ? device_.data()->needsPinSent() : false;
}

void MasterSigner::setNeedPinSent(bool value)
{
    if(device_){
        device_.data()->setNeedsPinSent(value);
        emit needPinSentChanged();
    }
}

bool MasterSigner::needPassphraseSent() const
{
    return device_ ? device_.data()->needsPassPhraseSent() : false;
}

void MasterSigner::setNeedPassphraseSent(bool value)
{
    if(device_){
        device_.data()->setNeedsPassPhraseSent(value);
        emit needPassphraseSentChanged();
    }
}

QString MasterSigner::deviceType() const
{
    return device_ ? device_.data()->type() : "";
}

bool MasterSigner::needXpub() const
{
    return isNeedXpub_;
}

void MasterSigner::setNeedXpub(bool isNeedXpub)
{
    isNeedXpub_ = isNeedXpub;
}

QString MasterSigner::fingerPrint() const
{
    if(device_){
        return device_.data()->masterFingerPrint();
    }
    else{
        return "";
    }
}

QString MasterSigner::message() const
{
    if(messageToSign_.isEmpty() || messageToSign_.isNull() || messageToSign_ == ""){
        return qUtils::QGenerateRandomMessage();
    }
    else{
        return messageToSign_;
    }
}

void MasterSigner::setMessage(const QString &messageToSign)
{
    if(messageToSign_ != messageToSign){
        messageToSign_ = messageToSign;
        emit messageToSignChanged();
    }
}

QString MasterSigner::signature() const
{
    return signature_;
}

void MasterSigner::setSignature(const QString &signature)
{
    if(signature_ != signature){
        signature_ = signature;
        emit signatureChanged();
    }
}

QString MasterSigner::path() const
{
    return path_;
}

void MasterSigner::setPath(const QString &path)
{
    if(path_ != path){
        path_ = path;
        emit pathChanged();
    }
}


int MasterSigner::signerType() const
{
    return signer_type_;
}

void MasterSigner::setSignerType(int signer_type)
{
    if(signer_type_ != signer_type){
        signer_type_ = signer_type;
        emit signerTypeChanged();
    }
}

QString MasterSigner::passphrase() const
{
    return passphrase_;
}

void MasterSigner::setPassphrase(const QString &passphrase){
    if(passphrase_ != passphrase){
        passphrase_ = passphrase;
        emit passphraseChanged();
    }
}

bool MasterSigner::isPrimaryKey() const
{
    return isPrimaryKey_;
}

void MasterSigner::setIsPrimaryKey(bool isPrimaryKey)
{
    if (isPrimaryKey_ == isPrimaryKey)
        return;

    isPrimaryKey_ = isPrimaryKey;
    emit isPrimaryKeyChanged();
}

nunchuk::PrimaryKey MasterSigner::primaryKey() const
{
    return primaryKey_;
}

MasterSignerListModel::MasterSignerListModel() {
    primaryKeys = qUtils::GetPrimaryKeys(AppSetting::instance()->storagePath(),(nunchuk::Chain)AppSetting::instance()->primaryServer());
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

MasterSignerListModel::~MasterSignerListModel() {d_.clear();}

int MasterSignerListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return d_.count();
}

QVariant MasterSignerListModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case master_signer_id_Role:
        return d_[index.row()]->id();
    case master_signer_name_Role:
        return d_[index.row()]->name();
    case master_signer_device_Role:
        return qVariantFromValue((QDevice*) d_[index.row()]->device());
    case master_signer_checked_Role:
        return d_[index.row()]->checked();
    case master_signer_fingerPrint_Role:
        return d_[index.row()]->fingerPrint();
    case master_signer_msg2Sign_Role:
        return d_[index.row()]->message();
    case master_signer_deviceType_Role:
        return d_[index.row()]->deviceType();
    case master_signer_signerType_Role:
        return d_[index.row()]->signerType();
    case master_signer_need_passphrase_Role:
        return d_[index.row()]->needPassphraseSent();
    case master_signer_need_pin_Role:
        return d_[index.row()]->needPinSent();
    case master_signer_need_xpub_Role:
        return d_[index.row()]->needXpub();
    case master_signer_primary_key_Role:
        return d_[index.row()]->isPrimaryKey();
    default:
        return QVariant();
    }
}

bool MasterSignerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == master_signer_checked_Role) {
        d_[index.row()]->setChecked(value.toBool());
    }
    else{ }
    emit dataChanged(index, index, { role } );
    return true;
}

QHash<int, QByteArray> MasterSignerListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[master_signer_id_Role]        = "master_signer_id";
    roles[master_signer_name_Role]      = "master_signer_name";
    roles[master_signer_device_Role]    = "master_signer_device";
    roles[master_signer_checked_Role]   = "master_signer_checked";
    roles[master_signer_fingerPrint_Role]   = "master_signer_fingerPrint";
    roles[master_signer_msg2Sign_Role]      = "master_signer_messageToSign";
    roles[master_signer_deviceType_Role]    = "master_signer_deviceType";
    roles[master_signer_signerType_Role]    = "master_signer_type";
    roles[master_signer_need_passphrase_Role]    = "master_signer_need_passphrase";
    roles[master_signer_need_pin_Role]      = "master_signer_need_pin";
    roles[master_signer_need_xpub_Role]     = "master_signer_need_xpub";
    roles[master_signer_primary_key_Role]   = "master_signer_primary_key";
    return roles;
}

void MasterSignerListModel::addMasterSigner(const QString &id,
                                            const QString& name,
                                            const QDevicePtr device,
                                            const int health ,
                                            const int signertype){
    beginResetModel();
    if(!contains(id)){
        d_.append(QMasterSignerPtr(new MasterSigner(id, name, device, health, signertype,containPrimaryKey(device->masterFingerPrint()))));
    }
    endResetModel();
}

void MasterSignerListModel::addMasterSigner(const QMasterSignerPtr &d)
{
    beginResetModel();
    if(!contains(d.data()->id())){
        d_.append(d);
    }
    endResetModel();
}

void MasterSignerListModel::updateMasterSignerName(const QString &signerId, const QString &name){
    foreach (QMasterSignerPtr it, d_) {
        if(it.data()->id() == signerId){
            beginResetModel();
            it.data()->setName(name);
            endResetModel();
        }
    }
}

void MasterSignerListModel::updateDeviceStatus(const QString &fingerprint, const bool connected)
{
    foreach (QMasterSignerPtr it, d_) {
        if(it.data()->device()){
            if(it.data()->device()->masterFingerPrint() == fingerprint){
                it.data()->device()->setConnected(connected);
                emit dataChanged(this->index(d_.indexOf(it)),this->index(d_.indexOf(it)));
            }
        }
    }
}

QMasterSignerPtr MasterSignerListModel::getMasterSignerByIndex(const int index) {
    if(index < 0 || index >= d_.count()){
        DBG_INFO << "Index out of range";
        return QMasterSignerPtr(NULL);
    }
    else {
        return d_.at(index);
    }
}

QMasterSignerPtr MasterSignerListModel::getMasterSignerById(const QString &id)
{
    foreach (QMasterSignerPtr it, d_) {
        if(it.data()->id() == id){
            return it;
        }
    }
    return QMasterSignerPtr(NULL);
}

QMasterSignerPtr MasterSignerListModel::getMasterSignerByXfp(const QString &xfp)
{
    foreach (QMasterSignerPtr it, d_) {
        if(0 == QString::compare(xfp, it.data()->fingerPrint(), Qt::CaseInsensitive)){
            return it;
        }
    }
    return NULL;
}

QString MasterSignerListModel::getMasterSignerNameByFingerPrint(const QString &fingerprint)
{
    foreach (QMasterSignerPtr it, d_) {
        if(0 == QString::compare(fingerprint, it.data()->fingerPrint(), Qt::CaseInsensitive)){
            return it.data()->name();
        }
    }
    return "";
}

bool MasterSignerListModel::removeMasterSigner(const QString &masterSignerId)
{
    beginResetModel();
    foreach (QMasterSignerPtr it, d_) {
        if(it.data() && it.data()->id() == masterSignerId){
            d_.removeAll(it);
            return true;
        }
    }
    endResetModel();
    return false;
}

void MasterSignerListModel::resetUserChecked()
{
    beginResetModel();
    foreach (QMasterSignerPtr it, d_) {
        it.data()->setChecked(false);
    }
    endResetModel();
}

void MasterSignerListModel::setUserChecked(const bool state, const int index)
{
    if(index < 0 || index >= d_.count()){
        DBG_INFO << "Index out of range";
    }
    else {
        beginResetModel();
        d_.at(index)->setChecked(state);
        endResetModel();
    }
}

void MasterSignerListModel::setUserCheckedById(const bool state, const QString &id)
{
    beginResetModel();
    foreach (QMasterSignerPtr it, d_) {
        if(0 == QString::compare(id, it.data()->id(), Qt::CaseInsensitive)){
            it.data()->setChecked(state);
        }
    }
    endResetModel();
}

void MasterSignerListModel::setUserCheckedByFingerprint(const bool state, const QString &xfp)
{
    beginResetModel();
    foreach (QMasterSignerPtr it, d_) {
        if(0 == QString::compare(xfp, it.data()->fingerPrint(), Qt::CaseInsensitive)){
            it.data()->setChecked(state);
        }
    }
    endResetModel();
}

bool MasterSignerListModel::removeMasterSigner(const QMasterSignerPtr it)
{
    beginResetModel();
    d_.removeAll(it);
    endResetModel();
    return true;
}

bool MasterSignerListModel::contains(const QString &masterSignerId)
{
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(masterSignerId, i.data()->id(), Qt::CaseInsensitive)){
            return true;
        }
    }
    return false;
}

bool MasterSignerListModel::containsFingerPrint(const QString &fingerprint)
{
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(fingerprint, i.data()->fingerPrint(), Qt::CaseInsensitive)){
            return true;
        }
    }
    return false;
}

bool MasterSignerListModel::hardwareContainsFingerPrint(const QString &fingerprint)
{
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(fingerprint, i.data()->fingerPrint(), Qt::CaseInsensitive)
                && i.data()->signerType() == 0){
            return true;
        }
    }
    return false;
}

void MasterSignerListModel::updateDeviceNeedPinSent(const QString &fingerprint, const bool needpin)
{
    beginResetModel();
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(fingerprint, i.data()->fingerPrint(), Qt::CaseInsensitive)){
            i.data()->device()->setNeedsPinSent(needpin);
        }
    }
}

void MasterSignerListModel::updateDeviceNeedPassphraseSent(const QString &fingerprint, const bool needpassphrase)
{
    beginResetModel();
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(fingerprint, i.data()->fingerPrint(), Qt::CaseInsensitive)){
            i.data()->setNeedPassphraseSent(needpassphrase);
        }
    }
    endResetModel();
}

void MasterSignerListModel::updateDeviceNeedPassphraseSentById(const QString &id, const bool needpassphrase)
{
    beginResetModel();
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(id, i.data()->id(), Qt::CaseInsensitive)){
            i.data()->setNeedPassphraseSent(needpassphrase);
        }
    }
    endResetModel();
}

void MasterSignerListModel::updateMasterSignerNeedXpubById(const QString &id, const bool value)
{
    beginResetModel();
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(id, i.data()->id(), Qt::CaseInsensitive)){
            i.data()->setNeedXpub(value);
        }
    }
    endResetModel();
}

void MasterSignerListModel::updateMasterSignerNeedXpubByXfp(const QString &xfp, const bool value)
{
    beginResetModel();
    foreach (QMasterSignerPtr i , d_ ){
        if(0 == QString::compare(xfp, i.data()->fingerPrint(), Qt::CaseInsensitive)){
            i.data()->setNeedXpub(value);
        }
    }
    endResetModel();
}

void MasterSignerListModel::resetMasterSignerNeedXpub()
{
    beginResetModel();
    foreach (QMasterSignerPtr i , d_ ){
        i.data()->setNeedXpub(false);
    }
    endResetModel();
}

QStringList MasterSignerListModel::getColdCardId() const
{
    QStringList ret;
    ret.clear();
    foreach (QMasterSignerPtr i , d_ ){
        if(i.data()->device() && 0 == QString::compare("coldcard", i.data()->device()->type(), Qt::CaseInsensitive)){
            ret << i.data()->id();
        }
    }
    return ret;
}

void MasterSignerListModel::requestSort(int role, int order)
{
    beginResetModel();
    if(d_.count() > 1){
        switch (role) {
        case master_signer_name_Role:
        {
            if(Qt::DescendingOrder == order){
                qSort(d_.begin(), d_.end(), sortMasterSignerByNameDescending);
            }
            else{
                qSort(d_.begin(), d_.end(), sortMasterSignerByNameAscending);
            }
        }
            break;
        default:
            break;
        }
    }
    endResetModel();
}

QList<QMasterSignerPtr> MasterSignerListModel::fullList() const
{
    return d_;
}

void MasterSignerListModel::cleardata()
{
    beginResetModel();
    d_.clear();
    endResetModel();
}

int MasterSignerListModel::signerCount() const
{
    return d_.count();
}

int MasterSignerListModel::signerSelectedCount() const
{
    int ret = 0;
    foreach (QMasterSignerPtr it, d_) {
        if(it.data()->checked()) { ret++;}
    }
    return ret;
}

nunchuk::PrimaryKey MasterSignerListModel::containPrimaryKey(const QString &fingerprint)
{
    for(nunchuk::PrimaryKey key: primaryKeys){
        if(key.get_master_fingerprint() == fingerprint.toStdString()){
            return key;
        }
    }
    return nunchuk::PrimaryKey();
}

bool sortMasterSignerByNameAscending(const QMasterSignerPtr &v1, const QMasterSignerPtr &v2)
{
    return (QString::compare((v1.data()->name()), (v2.data()->name())) < 0);
}

bool sortMasterSignerByNameDescending(const QMasterSignerPtr &v1, const QMasterSignerPtr &v2)
{
    return (QString::compare((v1.data()->name()), (v2.data()->name())) > 0);
}
