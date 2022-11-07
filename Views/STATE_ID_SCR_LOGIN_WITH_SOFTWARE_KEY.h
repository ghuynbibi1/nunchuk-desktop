
#ifndef STATE_ID_SCR_LOGIN_WITH_SOFTWARE_KEY_H
#define STATE_ID_SCR_LOGIN_WITH_SOFTWARE_KEY_H

#include <QObject>
#include <QVariant>
#include "ViewsDefines.h"
#include "ViewsEnums.h"

void SCR_LOGIN_WITH_SOFTWARE_KEY_Entry(QVariant msg);
void SCR_LOGIN_WITH_SOFTWARE_KEY_Exit(QVariant msg);
void EVT_PRIMARY_KEY_SIGNIN_ACCOUNT_REQUEST_HANDLER(QVariant msg);
void EVT_LOGIN_WITH_SOFTWARE_KEY_BACK_HANDLER(QVariant msg);
void EVT_LOGIN_WITH_SOFTWARE_KEY_SUCCEED_HANDLER(QVariant msg);

#endif // STATE_ID_SCR_LOGIN_WITH_SOFTWARE_KEY_H
