#include "QWalletServicesTag.h"
#include "ViewsEnums.h"
#include "Servers/Draco.h"
#include "Servers/Byzantine.h"
#include "WalletModel.h"
#include "Premiums/QGroupWallets.h"
#include "Premiums/QKeyRecovery.h"
#include "Premiums/QInheritancePlan.h"

QWalletServicesTag::QWalletServicesTag(int mode) : QSwitchAPI(mode)
  , m_keyRecovery(QKeyRecoveryPtr(new QKeyRecovery()))
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterType<InheritanceEnum>("NUNCHUCKTYPE", 1, 0, "INHERITANCE");
}

QWalletServicesTag::~QWalletServicesTag()
{

}

QString QWalletServicesTag::passwordToken() const
{
    return m_passwordToken;
}

bool QWalletServicesTag::requestVerifyPassword(const QString &password, const int action)
{
    clearToken();
    QString token = "";
    QString errormsg = "";
    bool ret = Draco::instance()->verifyPasswordToken(password, action, token, errormsg);
    if(!ret){
        emit verifyPasswordTokenAlert(errormsg);
    }
    else{
        m_passwordToken = token;
    }
    return ret;
}

bool QWalletServicesTag::requestLockDownVerifyPassword(const QString &password)
{
    return requestVerifyPassword(password,(int)TARGET_ACTION::EMERGENCY_LOCKDOWN);
}

bool QWalletServicesTag::requestRecoverKeyVerifyPassword(const QString &password)
{
    return requestVerifyPassword(password,(int)TARGET_ACTION::DOWNLOAD_KEY_BACKUP);
}

bool QWalletServicesTag::requestUpdateSecurityQuestionPassword(const QString &password)
{
    return requestVerifyPassword(password,(int)TARGET_ACTION::UPDATE_SECURITY_QUESTIONS);
}

bool QWalletServicesTag::requestServerKeyVerifyPassword(const QString &password)
{
    return requestVerifyPassword(password,(int)TARGET_ACTION::UPDATE_SERVER_KEY);
}

bool QWalletServicesTag::requestInheritancePlanVerifyPassword(const QString &password)
{
    return requestVerifyPassword(password,(int)TARGET_ACTION::UPDATE_INHERITANCE_PLAN);
}

bool QWalletServicesTag::requestDeleteWalletVerifyPassword(const QString &password)
{
    return requestVerifyPassword(password,(int)TARGET_ACTION::DELETE_WALLET);
}

bool QWalletServicesTag::RequestConfirmationCodeEmergencyLockdown()
{
    QWalletPtr w = ServiceSetting::instance()->walletInfoPtr();
    if (w.isNull()) return false;
    QString errormsg;
    QJsonObject output;

    QJsonObject body;
    body["period_id"] = m_period_id;
    body["group_id"] = m_mode == USER_WALLET ? "" : w->groupId();
    body["wallet"] = w->id();
    QJsonObject data;
    data["nonce"] = Draco::instance()->randomNonce();
    data["body"] = body;
    setConfirmCodeNonceBody(data);
    bool ret = Draco::instance()->RequestConfirmationCode("EMERGENCY_LOCKDOWN", data, output, errormsg);
    if (ret) {
        DBG_INFO << output;
        m_code_id = output["code_id"].toString();
    }
    return ret;
}

bool QWalletServicesTag::verifyConfirmationCode(const QString &code)
{
    DBG_INFO << code;
    QString errormsg;
    QJsonObject output;
    bool ret = Draco::instance()->VerifyConfirmationCode(m_code_id, code, output, errormsg);
    if (ret) {
        DBG_INFO << output;
        m_confirmToken = output["token"].toString();
        emit confirmCodeVerified();
    } else {
        emit answerErrorAlert(errormsg);
    }
    return ret;
}

QVariantList QWalletServicesTag::securityQuestions()
{
    return m_questions.toVariantList();
}

void QWalletServicesTag::setQuestions(const QJsonArray &questions)
{
    m_questions = questions;
    emit securityQuestionChanged();
}


bool QWalletServicesTag::CreateSecurityQuestionsAnswered()
{
    QJsonObject output;
    QString errormsg;
    bool ret = Draco::instance()->secQuesGet(output, errormsg);
    if (ret) {
        QJsonArray questions_answered;
        for (auto q : output["questions"].toArray()) {
            auto q_js = q.toObject();
            bool is_answered = q_js["is_answered"].toBool();
            if (is_answered) {
                questions_answered.append(q);
            }
        }
        ReqiredSignaturesInfo required_signatures_info = reqiredSignaturesInfo();
        int required_answers = required_signatures_info.required_answers == 0 ? 1 : required_signatures_info.required_answers;
        QList<int> rIndex;
        rIndex.clear();
        for (int i = 0; i < questions_answered.size(); i++) {
            if (i < required_answers) {
                rIndex.append(i);
            }
            else {
                break;
            }
        }
        QJsonArray questions_required;
        m_quesAnswers.clear();
        for (auto index : rIndex) {
            auto ques = questions_answered[index].toObject();
            questions_required.append(ques);
            SecurityQuestion answer;
            answer.id = ques["id"].toString();
            answer.question = ques["question"].toString();
            answer.is_answered = ques["is_answered"].toBool();
            m_quesAnswers.append(answer);
            DBG_INFO << ques;
        }
        setQuestions(questions_required);
    }
    return ret;
}

void QWalletServicesTag::secQuesAnswer(const QString &id, const QString &answer)
{
    for (int i = 0; i < m_quesAnswers.size(); i ++) {
        if (m_quesAnswers.at(i).id.localeAwareCompare(id) == 0) {
            if (!qUtils::strCompare(m_quesAnswers[i].answer, answer)) {
                m_quesAnswers[i].is_changed = true;
            }
            m_quesAnswers[i].answer = answer;
        }
    }
}

bool QWalletServicesTag::secQuesAnswer()
{
    int correct_answer = 0;
    QString errormsg = "";
    QString token = "";
    bool ret = Draco::instance()->secQuesAnswer(questionsAndAnswers(), token, correct_answer, errormsg);
    if (!ret) {
        emit answerErrorAlert(errormsg);
        return false;
    } else {
        m_secQuesToken = token;
        if (reqiredSignaturesInfo().required_answers == 0) {
            return correct_answer;
        } else {
            return m_quesAnswers.size() == correct_answer && correct_answer != 0;
        }
    }
}

QJsonArray QWalletServicesTag::questionsAndAnswers() const
{
    QJsonArray questions;
    for (auto v : m_quesAnswers) {
        QJsonObject question;
        question["question_id"] = v.id;
        question["answer"] =  v.answer;
        questions.append(question);
    }
    return questions;
}

QVariantList QWalletServicesTag::periods()
{
    return m_periods.toVariantList();
}

bool QWalletServicesTag::createLockdownPeriods()
{
    QJsonArray periods;
    QString errormsg;
    bool ret {false};
    if (m_mode == USER_WALLET) {
        ret = Draco::instance()->lockdownPeriods(periods, errormsg);
    } else if (m_mode == GROUP_WALLET) {
        ret = Byzantine::instance()->lockdownPeriods(periods, errormsg);
    }
    m_periods = periods;
    if (!ret) {
        lockdownPeriodsAlert(errormsg);
        return false;
    } else {
        if (periods.size() == 0) {
            errormsg = "There no period";
            lockdownPeriodsAlert(errormsg);
            return false;
        }
    }
    return true;
}

bool QWalletServicesTag::lockdownRequired(const QString &period_id)
{
    QWalletPtr w = ServiceSetting::instance()->walletInfoPtr();
    if (w.isNull()) return false;
    m_period_id = period_id;
    DBG_INFO << w->id() << m_period_id;
    QJsonObject output;
    QString errormsg;
    bool ret {false};
    if (m_mode == USER_WALLET) {
        ret = Draco::instance()->lockdownRequiredSignatures(m_period_id, w->id(), output, errormsg);
    } else if (m_mode == GROUP_WALLET) {
        ret = Byzantine::instance()->lockdownRequiredSignatures(w->groupId(), m_period_id, w->id(), output, errormsg);
    }
    else{}
    if (ret) {
        QJsonObject resultObj = output["result"].toObject();
        ServiceSetting::instance()->servicesTagPtr()->setReqiredSignatures(resultObj);
    }
    return ret;
}

bool QWalletServicesTag::lockdownByAnswerSecQues()
{
    QWalletPtr w = ServiceSetting::instance()->walletInfoPtr();
    if (w.isNull()) return false;
    QString errormsg;
    QString until_time;
    bool ret {false};
    if (m_mode == USER_WALLET) {
        ret = Draco::instance()->lockdownByAnswerSecQues(m_passwordToken, m_secQuesToken, m_period_id, w->id(), until_time, errormsg);
    }
    else if (m_mode == GROUP_WALLET) {
        ret = Byzantine::instance()->lockdownByAnswerSecQues(w->groupId(), m_passwordToken, m_secQuesToken, m_period_id, w->id(), until_time, errormsg);
    }
    else{}

    if (ret) {
        setUntilTime(until_time);
    }
    return ret;
}

bool QWalletServicesTag::lockdownByConfirmationCode()
{
    QWalletPtr w = ServiceSetting::instance()->walletInfoPtr();
    if (w.isNull()) return false;
    QString errormsg;
    QString until_time;
    bool ret {false};
    if (m_mode == USER_WALLET) {
    }
    else if (m_mode == GROUP_WALLET) {
        ret = Byzantine::instance()->lockdownByConfirmationCode(m_passwordToken, confirmToken(), confirmCodeNonceBody(), until_time, errormsg);
    }
    else{}
    if (ret) {
        setUntilTime(until_time);
    }
    setConfirmCodeNonceBody({}); // Clear
    return ret;
}

bool QWalletServicesTag::lockdownBySignDummyTx()
{
    QWalletPtr w = ServiceSetting::instance()->walletInfoPtr();
    if (w.isNull()) return false;
    QStringList signatures = m_signatures.values(); //==> Signatures sau khi sign dummy tx
    QString errormsg;
    QString until_time;
    bool ret {false};
    if (m_mode == USER_WALLET) {
        ret = Draco::instance()->lockdownBySignDummyTx(signatures, m_passwordToken, m_period_id, w->id(), until_time, errormsg);
    }
    else if (m_mode == GROUP_WALLET) {
        ret = Byzantine::instance()->lockdownBySignDummyTx(w->groupId(), signatures, m_passwordToken, m_period_id, w->id(), until_time, errormsg);
    }
    else{}
    if (ret) {
        setUntilTime(until_time);
    }
    return ret;
}

nunchuk::Wallet QWalletServicesTag::getWallet() const
{
    return m_wallet;
}

QString QWalletServicesTag::untilTime() const
{
    return m_untilTime;
}

void QWalletServicesTag::setUntilTime(QString untilTime)
{
    if (m_untilTime == untilTime)
        return;

    m_untilTime = untilTime;
    emit untilTimeChanged();
}

void QWalletServicesTag::clearClaimAnInheritance()
{
    DBG_INFO << m_inheritanceClaimPlan;
    setInheritanceClaimPlan({});
}

bool QWalletServicesTag::inheritanceCheck(const QString& magic, const QString& environment)
{
    QJsonObject result;
    QString errormsg;
    bool ret = Draco::instance()->inheritanceCheck(magic, environment, result, errormsg);
    if (ret) {
        DBG_INFO << result;
        setInheritanceCheckStatus(result);
        bool is_paid    = result["is_paid"].toBool();
        if (!is_paid) {
            emit notPaidAlert();
        }
    }
    return ret;
}

int QWalletServicesTag::inheritanceDownloadBackup(const QString &magic, const QString &backup_password, const QString& backup_password_two)
{
    QStringList backup_keys;
    backup_keys.clear();
    if (!backup_password.isEmpty()) {
        backup_keys.push_back(backup_password);
    }
    if (!backup_password_two.isEmpty()) {
        backup_keys.push_back(backup_password_two);
    }
    DBG_INFO << magic;
    QJsonObject result;
    QString errormsg;
    int response_code = DRACO_CODE::RESPONSE_OK;
    bool ret = Draco::instance()->inheritanceDownloadBackup(magic, backup_keys, response_code, result, errormsg);
    DBG_INFO << ret << errormsg;
    if (ret) {
        QJsonArray keys = result["keys"].toArray();
        master_signers.clear();
        single_signers.clear();
        if (keys.size() != backup_keys.size()) {
            // ERROR: Missing Backup Passwords
            AppModel::instance()->showToast(0, "Missing Backup Passwords", EWARNING::WarningType::ERROR_MSG);
            return DRACO_CODE::INHERITANCE_801;
        }

        auto downloadKey = [this](const QJsonObject& key, const QString& backup_key) {
            QWarningMessage msg;
            QString base64 = key["key_backup_base64"].toString();
            QByteArray ba;
            ba.append(base64);
            QByteArray base64bin = QByteArray::fromBase64(ba);
            std::vector<unsigned char> base64vec(base64bin.begin(), base64bin.end());
            QString key_name = key["key_name"].toString();
            QString derivation_path = key["derivation_path"].toString();
            auto master_signer_ptr = bridge::ImportTapsignerMasterSigner(base64vec, backup_key, key_name, false, msg);
            if (master_signer_ptr) {
                msg.resetWarningMessage();
                auto master_signer = master_signer_ptr->originMasterSigner();
                // update tag
                master_signer.set_tags({nunchuk::SignerTag::INHERITANCE});
                bridge::UpdateMasterSigner(master_signer, msg);
                master_signers.push_back(master_signer);
                single_signers.push_back(bridge::GetSignerFromMasterSigner(QString::fromStdString(master_signer.get_id()), derivation_path, msg));
            }
        };

        for(QJsonValue js : keys) {
            QJsonObject key = js.toObject();
            // Try with all backup_keys (Backup Password)
            for (auto backup_key : backup_keys) {
                downloadKey(key, backup_key);
            }
        }
        if (keys.size() != master_signers.size()) {
        // ERROR: Backup Password is incorrect
        // Remove master_singer
            AppModel::instance()->showToast(0, STR_CPP_115, EWARNING::WarningType::ERROR_MSG);
            for (auto m : master_signers) {
                bridge::nunchukDeleteMasterSigner(QString::fromStdString(m.get_id()));
            }
            return DRACO_CODE::INHERITANCE_801;
        }

        inheritanceClaimStatus(magic);
    }
    else {
        switch (response_code) {
        case DRACO_CODE::INHERITANCE_801:
            // show popup: Invalid phrase. Please try again
            AppModel::instance()->showToast(0, STR_CPP_115, EWARNING::WarningType::ERROR_MSG);
            break;
        case DRACO_CODE::INHERITANCE_802:
            // show popup: It looks like the original subscription has expired for more than 3 years.
            //             To claim the inheritance, please reactivate the plan first by visiting nunchuk.io/claim.
            emit isExpiredAlert();
            break;
        case DRACO_CODE::INHERITANCE_803:
            // show popup: Your inheritance plan has not been activated yet. Please try again later.
            emit hasNotBeenActivatedYetAlert();
            break;
        default:
            AppModel::instance()->showToast(response_code, errormsg, EWARNING::WarningType::EXCEPTION_MSG);
            break;
        }
    }
    return response_code;
}

bool QWalletServicesTag::inheritanceClaimRequest(const nunchuk::Wallet wallet, const nunchuk::Transaction txSigned, const QString& magic)
{
    QJsonObject result;
    QString errormsg;
    bool ret = Draco::instance()->inheritanceClaimRequest(magic, QString::fromStdString(txSigned.get_psbt()), result, errormsg);
    DBG_INFO << QString::fromStdString(txSigned.get_psbt());
    DBG_INFO << result;
    if(ret){
        //HANDLE RESULT
        QJsonObject transaction = result["transaction"].toObject();
        QString status = transaction.value("status").toString();
        QString psbt = transaction.value("psbt").toString();
        QString wallet_local_id = transaction.value("wallet_local_id").toString();
        if (status == "PENDING_CONFIRMATION" ||
                status == "CONFIRMED"){
            QWarningMessage _msg;
            bridge::nunchukImportPsbt(wallet_local_id, psbt, _msg);
            QString id = transaction.value("id").toString();
            QString hex = transaction.value("hex").toString();
            QString reject_msg = transaction.value("reject_msg").toString();
            bridge::nunchukUpdateTransaction(QString::fromStdString(wallet.get_id()), QString::fromStdString(txSigned.get_txid()), id, hex, reject_msg, _msg);
        }
        QJsonObject current = m_inheritanceClaimPlan;
        current["balance"] = 0;
        setInheritanceClaimPlan(current); // reset
        emit inheritanceInfoChanged();
    }
    return ret;
}

bool QWalletServicesTag::inheritanceClaimStatus(const QString& magic)
{
    QWarningMessage msg;
    QJsonObject body;
    body["magic"] = magic;

    QJsonObject data;
    data["nonce"] = Draco::instance()->randomNonce();
    data["body"]  = body;
    QJsonDocument doc(data);
    QString user_data(doc.toJson());
    QString messages_to_sign = qUtils::GetHealthCheckMessage(user_data,msg); // user_data in json string

    QStringList authos;
    for (auto signer : single_signers) {
        QString signature = bridge::SignHealthCheckMessage(signer, messages_to_sign, msg);
        QString authorization = qUtils::CreateRequestToken(signature, QString::fromStdString(signer.get_master_fingerprint()), msg);
        authos.append(authorization);
    }

    QJsonObject result;
    QString errormsg;
    bool ret = Draco::instance()->inheritanceClaimStatus(data, authos, result, errormsg);
    DBG_INFO << result;
    if(ret){
        //HANDLE RESULT
        setInheritanceClaimPlan(result);
    }
    return ret;
}

bool QWalletServicesTag::inheritanceCreateTx(const QJsonObject& data, const QStringList& authos)
{
    QJsonObject result;
    QString errormsg;
    DBG_INFO << data << authos;
    bool ret = Draco::instance()->inheritanceCreateTx(data, authos, result, errormsg);
    if(ret){
        QJsonObject transaction = result["transaction"].toObject();
        QString psbt = transaction["psbt"].toString();
        QString sub_amount = QString("%1").arg(result["sub_amount"].toDouble());
        QString fee = QString("%1").arg(result["fee"].toDouble());
        QString fee_rate = QString("%1").arg(result["fee_rate"].toDouble());
        qint64 tx_fee = qUtils::QAmountFromValue(fee); // fee in BTC
        qint64 tx_fee_rate = qUtils::QAmountFromValue(fee_rate); // fee_rate in BTC
        qint64 tx_sub_amount = qUtils::QAmountFromValue(sub_amount); // sub amount in BTC
        //HANDLE RESULT
        // decode transaction
        mInheritance.wallet = nunchuk::Wallet(false);
        QWarningMessage msg;
        mInheritance.tx = qUtils::DecodeTx(mInheritance.wallet, psbt, tx_sub_amount, tx_fee, tx_fee_rate, msg);
        if((int)EWARNING::WarningType::NONE_MSG == msg.type()){
            QTransactionPtr trans = bridge::convertTransaction(mInheritance.tx, QString::fromStdString(mInheritance.wallet.get_id()));
            if(trans){
                AppModel::instance()->setTransactionInfo(trans);
                QList<uint> states = QEventProcessor::instance()->getCurrentStates();
                if(!states.isEmpty() && states.last() == E::STATE_ID_SCR_INHERITANCE_WITHDRAW_BALANCE){
                    DBG_INFO << "Entry here ";
                    QEventProcessor::instance()->sendEvent(E::EVT_INHERITANCE_CONFIRM_TRANSACTION_REQUEST);
                }
            }
        }
        else{
            AppModel::instance()->showToast(msg.code(), msg.what(), (EWARNING::WarningType)msg.type());
        }
    }
    return ret;
}

void QWalletServicesTag::setInheritanceAddress(const QString& to_wallet_id)
{
    QWalletPtr ptr = AppModel::instance()->walletList()->getWalletById(to_wallet_id);
    if (!ptr) return;
    ServiceSetting::instance()->setWalletInfo(ptr);
    if(AppModel::instance()->walletInfo()){
        QString wallet_id = AppModel::instance()->walletInfo()->id();
        QStringList addrs = bridge::nunchukGetUnusedAddresses(wallet_id, false);
        if (addrs.size() > 0) {
            mInheritance.m_destinationAddress = addrs.first();
        } else {
            mInheritance.m_destinationAddress = bridge::nunchukGenNewAddresses(wallet_id,false);
        }
        DBG_INFO << mInheritance.m_destinationAddress;
    }
}

void QWalletServicesTag::setInheritanceAddressNewTransaction(const QString &address)
{
    mInheritance.m_destinationAddress = address;
}

bool QWalletServicesTag::inheritanceCreateDraftTransaction(double fee_rate)
{
    const constexpr auto NONE_MSG = (int)EWARNING::WarningType::NONE_MSG;
    QJsonObject inheritance = m_inheritanceClaimPlan["inheritance"].toObject();

    QJsonObject body;
    body["magic"] = inheritance["magic"].toString();
    body["address"] = mInheritance.m_destinationAddress;
    body["fee_rate"] = qUtils::QValueFromAmount(fee_rate);

    QJsonObject data;
    data["nonce"] = Draco::instance()->randomNonce();
    data["body"]  = body;
    QJsonDocument doc(data);
    QString user_data(doc.toJson());
    QWarningMessage msg;
    QString messages_to_sign = qUtils::GetHealthCheckMessage(user_data, msg);
    QStringList authos;
    for (auto signer : single_signers) {
        QString signature = bridge::SignHealthCheckMessage(signer, messages_to_sign, msg);
        QString authorization = qUtils::CreateRequestToken(signature, QString::fromStdString(signer.get_master_fingerprint()), msg);
        authos.append(authorization);
    }

    if (NONE_MSG != msg.type()) {
        AppModel::instance()->showToast(msg.code(), msg.what(), (EWARNING::WarningType)msg.type());
        return false;
    }
    return inheritanceCreateTx(data, authos);
}

bool QWalletServicesTag::inheritanceSignTransaction()
{
    QJsonObject inheritance = m_inheritanceClaimPlan["inheritance"].toObject();
    DBG_INFO << inheritance;
    QWarningMessage msgwarning;
    nunchuk::Transaction tx = mInheritance.tx;
    for(auto master_signer : master_signers) {
        tx = bridge::SignTransaction(mInheritance.wallet,
                                                        tx,
                                                        nunchuk::Device(master_signer.get_id()),
                                                        msgwarning);
    }

    if ((int)EWARNING::WarningType::NONE_MSG == msgwarning.type()) {
        QTransactionPtr trans = bridge::convertTransaction(tx, "");
        AppModel::instance()->setTransactionInfo(trans);
        return inheritanceClaimRequest(mInheritance.wallet,
                                tx,
                                inheritance["magic"].toString());
    } else {
        AppModel::instance()->showToast(msgwarning.code(), msgwarning.what(), (EWARNING::WarningType)msgwarning.type());
    }
    return false;
}

void QWalletServicesTag::additionalGetWalletConfig()
{
    QJsonObject config;
    if (m_mode == USER_WALLET) {
        config = Draco::instance()->assistedGetWalletConfig();
        setWalletConfig(config);
    } else if (m_mode == GROUP_WALLET) {
        QJsonObject wallet_config = Byzantine::instance()->assistedGetWalletConfig();
        QString slug = CLIENT_INSTANCE->slug();
        slug = slug.remove("_testnet");
        DBG_INFO << slug;
        DBG_INFO << wallet_config;
        config = wallet_config;//wallet_config[slug].toObject();
    }
    setWalletConfig(config);
    DBG_INFO << config;
}

QVariant QWalletServicesTag::inheritanceInfo() const
{
    static const int64_t COIN = 100000000;
    QJsonObject inheritance = m_inheritanceClaimPlan["inheritance"].toObject();
    QMap<QString,QVariant> maps;
    double balance = m_inheritanceClaimPlan["balance"].toDouble();//BTC
    int balanceSats = balance * COIN;
    QLocale locale(QLocale::English);
    if((int)AppSetting::Unit::SATOSHI == AppSetting::instance()->unit()){
        maps["balance"] = locale.toString(balanceSats);//sats
    }
    else{
        maps["balance"] = locale.toString(balance, 'f', qUtils::Precision(balance));
    }
    maps["balanceSats"] = balanceSats;
    maps["balanceCurrency"] = qUtils::currencyLocale(balanceSats);
    maps["note"] = inheritance["note"].toString();
    return QVariant::fromValue(maps);
}

QStringList QWalletServicesTag::listSetuped() const
{
    return m_listSetuped;
}

void QWalletServicesTag::setListSetuped()
{
    QStringList setuped {};
    for(auto wallet_id : WalletsMng->wallets()) {
        if (auto w = AppModel::instance()->walletListPtr()->getWalletById(wallet_id)) {
            if (auto plan = w->inheritancePlanPtr()) {
                if (w->isGroupWallet()) {
                    if (plan->IsActived()) {
                        setuped << wallet_id;
                    }
                }
                else {
                    setuped << wallet_id;
                }
            }
        }
    }
    if (m_listSetuped == setuped)
        return;
    m_listSetuped = setuped;
    DBG_INFO << m_mode << setuped;
    emit listSetupedChanged();
}

QStringList QWalletServicesTag::listLockdown() const
{
    return m_listLockdown;
}

void QWalletServicesTag::setListLockdown()
{
    QStringList setuped {};
    for(auto wallet_id : WalletsMng->wallets()) {
        if (auto w = AppModel::instance()->walletListPtr()->getWalletById(wallet_id)) {
            if (auto dash = w->dashboard()) {
                if (m_mode == GROUP_WALLET) {
                    auto hasPermission = dash->role() == "MASTER" || dash->role() == "ADMIN";
                    if (hasPermission) {
                        setuped << wallet_id; // for GroupWallet
                    }
                }
                else {
                    setuped << wallet_id;
                }
            }
            else {
                setuped << wallet_id;
            }
        }
    }
    if (m_listLockdown == setuped)
        return;
    m_listLockdown = setuped;
    emit listGroupWChanged();
}

QStringList QWalletServicesTag::listLocked() const
{
    return m_listLocked;
}

void QWalletServicesTag::setListLocked()
{
    QStringList setuped {};
    for(auto wallet_id : WalletsMng->wallets()) {
        if (auto w = AppModel::instance()->walletListPtr()->getWalletById(wallet_id)) {
            if (auto dash = w->dashboard()) {
                auto hasPermission = dash->role() == "MASTER" || dash->role() == "ADMIN";
                if (dash->isLocked() && hasPermission) {
                    setuped << wallet_id;
                }
            }
        }
    }
    if (m_listLocked == setuped)
        return;
    m_listLocked = setuped;
    emit listLockedChanged();
}

QStringList QWalletServicesTag::listPolicy() const
{
    return m_listPolicy;
}

void QWalletServicesTag::setListPolicy()
{
    QStringList setuped {};
    for(auto wallet_id : WalletsMng->wallets()) {
        if (auto w = AppModel::instance()->walletListPtr()->getWalletById(wallet_id)) {
            if (auto dash = w->dashboard()) {
                auto hasPermission = dash->role() == "KEYHOLDER" || dash->role() == "MASTER" || dash->role() == "ADMIN";
                if (w->isPro() && hasPermission) {
                    setuped << wallet_id;
                }
                else if (w->isUserWallet()) {
                    setuped << wallet_id;
                }
            } else {
                setuped << wallet_id;
            }
        }
    }
    if (m_listPolicy == setuped)
        return;
    m_listPolicy = setuped;
    emit listPolicyChanged();
}

QStringList QWalletServicesTag::list2FA() const
{
    return m_list2FA;
}

void QWalletServicesTag::setList2FA()
{
    QStringList setuped {};
    for(auto wallet_id : WalletsMng->wallets()) {
        if (auto w = AppModel::instance()->walletListPtr()->getWalletById(wallet_id)) {
            if (auto dash = w->dashboard()) {
                if (w->isGroupWallet()) {
                    auto hasPermission = dash->role() == "MASTER";
                    if (hasPermission) {
                        setuped << wallet_id;
                    }
                }
                else {
                    setuped << wallet_id;
                }
            } else {
                setuped << wallet_id;
            }
        }
    }
    if (m_list2FA == setuped)
        return;
    m_list2FA = setuped;
}

QString QWalletServicesTag::secQuesToken() const
{
    return m_secQuesToken;
}

void QWalletServicesTag::clearToken()
{
    m_passwordToken = "";
    m_secQuesToken = "";
    m_confirmToken = "";
}

QVariant QWalletServicesTag::inheritanceCheckStatus() const
{
    return QVariant::fromValue(m_inheritanceCheckStatus);
}

void QWalletServicesTag::setInheritanceCheckStatus(const QJsonObject &status)
{
    if (m_inheritanceCheckStatus == status)
        return;

    m_inheritanceCheckStatus = status;
    emit inheritanceCheckStatusChanged();
}

QVariant QWalletServicesTag::inheritanceClaimPlan() const
{
    return QVariant::fromValue(m_inheritanceClaimPlan);
}

void QWalletServicesTag::setInheritanceClaimPlan(const QJsonObject &claim)
{
    if (m_inheritanceClaimPlan == claim)
        return;

    m_inheritanceClaimPlan = claim;
    emit inheritanceClaimPlanChanged();
}

QVariant QWalletServicesTag::walletConfig() const
{
    return QVariant::fromValue(m_walletConfig);
}

void QWalletServicesTag::setWalletConfig(const QJsonObject &config)
{
    if (m_walletConfig == config)
        return;

    m_walletConfig = config;

    int total_remaining = 0;
    for (const QString &key : m_walletConfig.keys()) {
        if (m_walletConfig[key].isObject()) {
            QJsonObject subObj = m_walletConfig[key].toObject();
            total_remaining += subObj["remaining_wallet_count"].toInt();
        }
    }
    m_walletConfig["remaining_wallet_count"] = total_remaining;
    emit walletConfigChanged();
}

QVariant QWalletServicesTag::reqiredSignatures() const
{
    return QVariant::fromValue(m_reqiredSignatures);
}

QJsonObject QWalletServicesTag::reqiredSignaturesJs() const
{
    return m_reqiredSignatures;
}

void QWalletServicesTag::setReqiredSignatures(const QJsonObject &reqired)
{
    if (m_reqiredSignatures == reqired)
        return;

    m_reqiredSignatures = reqired;
    emit reqiredSignaturesChanged();
}

ReqiredSignaturesInfo QWalletServicesTag::reqiredSignaturesInfo() const
{
    ReqiredSignaturesInfo reqired_signature {};
    reqired_signature.type = required_signatures_type[m_reqiredSignatures["type"].toString()];
    reqired_signature.required_signatures = m_reqiredSignatures["required_signatures"].toInt();
    reqired_signature.required_answers = m_reqiredSignatures["required_answers"].toInt();
    reqired_signature.required_confirmation_codes = m_reqiredSignatures["required_confirmation_codes"].toInt();
    return reqired_signature;
}

void QWalletServicesTag::clearBufferPeriodCountdown()
{
    QJsonObject data = m_inheritanceClaimPlan;
    data["buffer_period_countdown"] = {};
    DBG_INFO << data;
    setInheritanceClaimPlan(data);
}

void QWalletServicesTag::clearInheritance()
{
    QJsonObject data = m_inheritanceClaimPlan;
    data["inheritance"] = {};
    DBG_INFO << data;
    setInheritanceClaimPlan(data);
}

QJsonObject QWalletServicesTag::confirmCodeNonceBody() const
{
    return m_confirmCodeRequestBody;
}

void QWalletServicesTag::setConfirmCodeNonceBody(const QJsonObject& nonceBody)
{
    m_confirmCodeRequestBody = nonceBody;
}

QString QWalletServicesTag::confirmToken() const
{
    return m_confirmToken;
}

void QWalletServicesTag::ConfigServiceTag()
{
    setListSetuped();
    setListLockdown();
    setListLocked();
    setListPolicy();
    setList2FA();
}

QString QWalletServicesTag::code_id() const
{
    return m_code_id;
}

void QWalletServicesTag::setCode_id(const QString &code_id)
{
    m_code_id = code_id;
}

QKeyRecoveryPtr QWalletServicesTag::keyRecoveryPtr() const
{
    return m_keyRecovery;
}

QVariant QWalletServicesTag::keyRecovery() const
{
    return QVariant::fromValue(keyRecoveryPtr().data());
}
