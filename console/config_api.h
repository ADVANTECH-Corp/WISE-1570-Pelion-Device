
#ifndef __CONFIG_API__H__
#define __CONFIG_API__H__

//*****************************************************************************
//                  Included files
//*****************************************************************************
#include "mbed.h"
#include "KVStore.h"
#include "kvstore_global_api.h"
#include "console.h"



//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//                  MACROs and needed structures
//*****************************************************************************
#define KV_MAX_BUFFER_LENGTH 64
#define KV_ROOT_PATH "/kv/"
#define KV_KEY_INIT "/kv/init"
#define KV_KEY_INIT_VALUE "1570"
#define KV_KEY_APN "/kv/apn"

#define CFG_ERR_CODE \
        X(eCfg_Ok,                      "OK"), \
        X(eCfg_Err_NotFound,            "Not found"), \
        X(eCfg_Err_Buf_Too_Small,       "Buffer too small"), \
        X(eCfg_Err_Argument,            "Invalid argument"), \
        X(eCfg_Err_KVSReset,            "Reset with kvstore failed"), \
        X(eCfg_Err_KVSSet,              "Set with kvstore failed"), \
        X(eCfg_Err_KVSGet,              "Get with kvstore failed"), \
        X(eCfg_Err_KVSGetInfo,          "Get info with kvstore failed"), \
        X(eCfg_Err_Failed,              "Failed"), \
        X(eCfg_Err_NotDefined,          "Not defined"), \
        X(eCfg_Err_End,                 NULL)

#define X(a, b) a
enum EConfigErrorCode{
    CFG_ERR_CODE
};
#undef X

//
// For debug
//
#define CONFIG_API_DEBUG

//*****************************************************************************
//                  Functions prototypes
//*****************************************************************************
int Config_iInit(void);
int Config_iUnInit(void);
int Config_iSetApn(char *_strApn);
int Config_iGetApn(char *_strApn);
int Config_iReset(void);
const char* Config_iErrorString(int _iErrorCode);



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif /* __CONFIG_API__H__ */

