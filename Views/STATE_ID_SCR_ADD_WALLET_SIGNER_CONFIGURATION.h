
#ifndef STATE_ID_SCR_ADD_WALLET_SIGNER_CONFIGURATION_H
#define STATE_ID_SCR_ADD_WALLET_SIGNER_CONFIGURATION_H

#include <QObject>
#include <QVariant>
#include "ViewsDefines.h"
#include "ViewsEnums.h"

void SCR_ADD_WALLET_SIGNER_CONFIGURATION_Entry(QVariant msg);
void SCR_ADD_WALLET_SIGNER_CONFIGURATION_Exit(QVariant msg);
void EVT_SIGNER_CONFIGURATION_SELECT_MASTER_SIGNER_HANDLER(QVariant msg);
void EVT_SIGNER_CONFIGURATION_REMOVE_SIGNER_HANDLER(QVariant msg);
void EVT_SIGNER_CONFIGURATION_SELECT_REMOTE_SIGNER_HANDLER(QVariant msg);
void EVT_SIGNER_CONFIGURATION_MASTER_SIGNER_SEND_PASSPHRASE_HANDLER(QVariant msg);
void EVT_SIGNER_CONFIGURATION_TRY_REVIEW_HANDLER(QVariant msg);
void EVT_ADD_WALLET_SIGNER_CONFIGURATION_BACK_HANDLER(QVariant msg);
void EVT_ADD_WALLET_SIGNER_CONFIGURATION_REVIEW_HANDLER(QVariant msg);
void EVT_ADD_SIGNER_TO_WALLET_REQUEST_HANDLER(QVariant msg);

#endif // STATE_ID_SCR_ADD_WALLET_SIGNER_CONFIGURATION_H
