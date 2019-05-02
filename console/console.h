
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

//*****************************************************************************
//                 Included files
//*****************************************************************************
#include "mbed.h"



//*****************************************************************************
//
// For building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//                 MACROs and needed structures
//*****************************************************************************
#define CONSOLE_BAUDRATE            115200
#define CONSOLE_DELAY_TIME            5
#define CONSOLE_MAX_CMD_SET_NUM        1
#define CONSOLE_MAX_BUFFER_SIZE     128
extern Serial *g_pConsole;
#define MSG_print g_pConsole->printf
#define CONSOLE_CLRSCR MSG_print("\033[2J\033[0;0H")
#define RETERR_IF_TRUE(cond, err_code) {if(cond) return err_code;}
#define RETERR_SHOWMSG(err_code, string, ...) \
            do {\
                    MSG_print("EN:%03d Line:%04d ErrMsg:", err_code, __LINE__); \
                    MSG_print(string, ## __VA_ARGS__); \
                    return err_code; \
            } while (0);

//
// For debug
//            
//#define CONSOLE_DEBUG
#ifdef CONSOLE_DEBUG
#define DBG_print(s, args...) \
        do { \
            MSG_print(s, ##args); \
        } \
        while(0)
#else
#define DBG_print(...) do{}while(0);
#endif

//
// Command set format
//
typedef int (*pfCmdFunc)(char *_strCmd);
typedef int (*pfCmdArgFunc)(char *_strCmd, void *_vArg);
typedef struct _TCmdSet{
    pfCmdFunc pfRunCmd; // pointer to function for running command
}TCmdSet;

typedef struct _TCmd{
    const char *strName; // Name of command
    const char *strHelp; // Information about how to use this command
}TCmd;

#define CONSOLE_ERR_CODE \
        X(eConsole_Ok,                      "OK"), \
        X(eConsole_Err_Failed,              "Failed"), \
        X(eConsole_Err_BufIsFull,           "Command set is full"), \
        X(eConsole_Err_UnKnowPara,          "Unknown parameter or value"), \
        X(eConsole_Err_UnKnowCmd,           "Unknown command"), \
        X(eConsole_Err_Arg,                 "Illegal argument"), \
        X(eConsole_Err_Value,               "Invalid value"), \
        X(eConsole_Err_End,                 NULL)

#define X(a, b) a
enum EConsoleErrorCode{
    CONSOLE_ERR_CODE
};
#undef X



//*****************************************************************************
//            Functions prototypes
//*****************************************************************************
int Console_iInit(void);
int Console_iUnInit(void);
int Console_iGetString(char *_pcBuf, unsigned int _uiBufLen);
int Console_iPutChar(char _cChar);
int Console_iRunCmd(char *_strCmd);
int Console_iRegCmdSet(TCmdSet *_ptCmdSet);



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* __CONSOLE_H__ */

