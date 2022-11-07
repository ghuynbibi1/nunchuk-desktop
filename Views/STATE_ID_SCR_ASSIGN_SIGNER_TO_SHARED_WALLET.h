
#ifndef STATE_ID_SCR_ASSIGN_SIGNER_TO_SHARED_WALLET_H
#define STATE_ID_SCR_ASSIGN_SIGNER_TO_SHARED_WALLET_H

#include <QObject>
#include <QVariant>
#include "ViewsDefines.h"
#include "ViewsEnums.h"

void SCR_ASSIGN_SIGNER_TO_SHARED_WALLET_Entry(QVariant msg);
void SCR_ASSIGN_SIGNER_TO_SHARED_WALLET_Exit(QVariant msg);
void EVT_ASSIGN_SIGNER_TO_SHARED_REQUEST_HANDLER(QVariant msg);
void EVT_ASSIGN_SIGNER_TO_SHARED_WALLET_BACK_REVIEW_WALLET_HANDLER(QVariant msg);
void EVT_ASSIGN_SIGNER_TO_SHARED_BACK_WALLET_CONFIG_HANDLER(QVariant msg);

#endif // STATE_ID_SCR_ASSIGN_SIGNER_TO_SHARED_WALLET_H
