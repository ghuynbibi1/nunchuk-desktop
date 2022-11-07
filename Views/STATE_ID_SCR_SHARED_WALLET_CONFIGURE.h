
#ifndef STATE_ID_SCR_SHARED_WALLET_CONFIGURE_H
#define STATE_ID_SCR_SHARED_WALLET_CONFIGURE_H

#include <QObject>
#include <QVariant>
#include "ViewsDefines.h"
#include "ViewsEnums.h"

void SCR_SHARED_WALLET_CONFIGURE_Entry(QVariant msg);
void SCR_SHARED_WALLET_CONFIGURE_Exit(QVariant msg);
void EVT_SHARED_WALLET_FINALIZE_WALLET_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CONFIGURE_CANCEL_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CONFIGURE_EXPORT_COLDCARD_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CONFIGURE_EXPORT_QRCODE_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CONFIGURE_DELETE_SWL_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CONFIGURE_EXPORT_BSMS_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CONFIGURE_BACK_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_ADD_SIGNER_REQUEST_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_UTXO_OUTPUT_HANDLER(QVariant msg);
void EVT_SHARED_WALLET_CHANGE_ADDRESS_HANDLER(QVariant msg);

#endif // STATE_ID_SCR_SHARED_WALLET_CONFIGURE_H
