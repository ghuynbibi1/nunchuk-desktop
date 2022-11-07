
#ifndef STATE_ID_SCR_SIGN_IN_MANUALLY_H
#define STATE_ID_SCR_SIGN_IN_MANUALLY_H

#include <QObject>
#include <QVariant>
#include "ViewsDefines.h"
#include "ViewsEnums.h"

void SCR_SIGN_IN_MANUALLY_Entry(QVariant msg);
void SCR_SIGN_IN_MANUALLY_Exit(QVariant msg);
void EVT_ADD_PRIMARY_KEY_ACCOUNT_REQUEST_HANDLER(QVariant msg);
void EVT_PRIMARY_KEY_CHECK_USERNAME_REQUEST_HANDLER(QVariant msg);
void EVT_CHALLENGE_MESSAGE_REFRESH_REQUEST_HANDLER(QVariant msg);
void EVT_ADD_PRIMARY_KEY_ACCOUNT_SUCCEED_HANDLER(QVariant msg);

#endif // STATE_ID_SCR_SIGN_IN_MANUALLY_H
