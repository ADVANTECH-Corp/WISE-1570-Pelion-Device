
#ifndef __COMMAND_SET__H__
#define __COMMAND_SET__H__

//*****************************************************************************
//                  Included files
//*****************************************************************************
#include "mbed.h"
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
#define W1570_CMD_SET_RETERR(cond, err_code, string, ...) \
            do {\
                    if(cond) { \
                        MSG_print(string, ## __VA_ARGS__); \
                        return err_code; \
                    } \
            } while (0);

//
// Error messages
//
#define CFG_API_ERR_ENABLE
#if defined(CFG_API_ERR_ENABLE)
#define CFG_API_ERR_MSG(err_code) \
            do {\
                    MSG_print("%s \n\r", Config_iErrorString(err_code)); \
            } while (0);     
#else
#define CFG_API_ERR_MSG(err_code) do{}while(0);
#endif // CFG_API_ERR_ENABLE


//
// These macros are response messages we expected as default
//
#define INVALID_MSG         "Invalid value"
#define INVALID_PARA_MSG    "Invalid parameter"
#define COMPLETE_MSG        "OK"
#define FAILED_MSG          "Failed"

#if 0
enum EParaSupported{
    ePara_OnlySet,
    ePara_OnlyGet,
    ePara_Both
};
#endif

#define COMMAND_SET \
        X(eCmdSet_setapn, "setapn", Cmd_iSetApn, \
                                        "setapn\n\r" \
                                        " Ex. setapn your_apn_string \n\r"), \
        X(eCmdSet_getapn, "getapn", Cmd_iGetApn, \
                                        "getapn\n\r" \
                                        " Ex. getapn \n\r"), \
        X(eCmdSet_Total, NULL, NULL, NULL)


#define X(cmd, cmd_name, func, info) cmd
enum EMoteCommandSet{
    COMMAND_SET
};
#undef X



//*****************************************************************************
//                  Functions prototypes
//*****************************************************************************
int Cmd_iInit(void);



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_SET__H__ */

