///
/// \file       Shell.c
/// \brief      [Source file]
///
/// \author     NOVO
///
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <CommunicationTask.h>
#include "usart.h"
#include "ShellComPort.h"
#include "DebugConsole.h"
#include "BootLoaderImport.h"
#include "semphr.h"
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <Shell.h>
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "semphr.h"
#include "reboot.h"
#include "FSUtil.h"
#include "crc.h"
#include "PersonalityTask.h"
#include "GraphicsTask.h"
#include "HaloLedsDriver.h"
#include "SHTimers.h"
#include "BoardRev.h"
#include "Power.h"
#include "device_telemetry.h"
#include "rtc.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/* Messages */
static const char PROMPT[] = "\r\nShell>";
static const char HELPMSG[]= "\r\nEnter 'help' for help.\n\n";
static const char SYNTAXSHELL[] = "\r\nError: Invalid syntax for: %s\n";
static const char INVCMD[] = "\r\nError: No such command: %s\n";

typedef const struct {
    char *cmd;                  /* command name user types, ie. GO  */
    int   min_args;             /* min num of args command accepts  */
    int   max_args;             /* max num of args command accepts  */
    void (*func)(int, char **); /* actual function to call          */
    char *description;          /* brief description of command     */
} CmdShell;

void shellHelp (int argc, char **argv);
void shellVersion (int argc, char **argv);
void shellSerial (int argc, char **argv);
void shellDebug (int argc, char **argv);
void shellTest (int argc, char **argv);
static void shellHalt(int ac, char **av);
static void shellHang(int ac, char **av);
static void shellReboot(int ac, char **av);
static void shellCrash(int ac, char **av);
static void shellWwdg(int ac, char **av);
static void shellForceGolden(int ac, char **av);
static void shellListFiles(int ac, char **av);
static void shellDf(int ac, char **av);
static void shellRm(int ac, char **av);
static void shellCrc(int ac, char **av);
static void shellFormat(int ac, char **av);
static void shellXxd(int ac, char **av);
static void shellBLEdebug(int ac, char **av);
static void shellPower(int ac, char **av);
static void shellWatchdog(int ac, char **av);
static void shellSleep(int ac, char **av);
static void shellTime(int ac, char **av);
static void shellTimers(int ac, char **av);
static void shellTimerstats(int ac, char **av);
static void shellBoardRev(int ac, char **av);
static void shellBLEStatus(int ac, char **av);
static void shellDumpLog(int ac, char **av);
static void shellSetDate(int ac, char **av);
static void shellSetLanguage(int ac, char **argv);
static void shellDate(int ac, char **av);
static void shellWatch(int ac, char **av);
static void shellStack(int ac, char **av);
static void shellDisableIWDG(int ac, char **av);

const CmdShell CmdShellTab[] =
{
    {"?",           0, 0, shellHelp,                        "Display this help message"},
    {"help",        0, 0, shellHelp,                        "Display this help message"},
    {"version",     0, 0, shellVersion,                     "Display the FW version and the shell version"},
    {"serial",      0, 0, shellSerial,                      "Display serial ID"},
    {"getID",       0, 0, readDeviceID_DebugConsole,        "Get the device ID"},
    {"getLight",    0, 0, readPhotoSensor_DebugConsole,     "Get the light sensor value"},
    {"getAcc",      0, 0, readAccSensor_DebugConsole,       "Get the Accelerometer sensor value"},
    {"getMag",      0, 0, readMagSensor_DebugConsole,       "Get the Magnetometer sensor value"},
    {"getTemp",     0, 0, readTempSensor_DebugConsole,      "Get the Temperature sensor value"},
    {"getSoc",      0, 0, readStateOfCharge_DebugConsole,   "Get the State of Charge"},
    {"anim",        1, 25, animation_DebugConsole,          "Set an animation on the HALOLED, Type and values"},
    {"animOff",     0, 0, animationOff_DebugConsole,        "Turn off Halo Animations"},
    {"touch",       0, 0, startTouchTest_DebugConsole,      "Start a touch test or skip a part"},
    {"touchCal",    13, 13, calibrateTouch_DebugConsole,    "Send a touch calibration"},
    {"swipe",       1, 1, swipeTest_DebugConsole,           "Send test swipe"},
    {"tap",         2, 2, tapTest_DebugConsole,             "Send test tap"},
    {"release",     0, 0, releaseTest_DebugConsole,         "Send test release"},
    {"sound",       0, 0, soundTest_DebugConsole,           "Play the sound test"},
    {"leds",        1, 3, ledsTest_DebugConsole,            "Display LEDs in RGB, stage and optional brightness and led to disable"},
    {"front",       1, 2, frontTest_DebugConsole,           "Change the front light brightness percentage"},
    {"oled",        0, 2, oledTest_DebugConsole,            "show white OLED Screen, or edit it"},
    {"hardware",    0, 1, hardwareTest_DebugConsole,        "Display hardware test results"},
    {"pin",         3, 3, pinTest_DebugConsole,             "10 pulses on a pin at inputed period"},
    {"test",        0, 1, shellTest,                        "Enter test mode"},
    {"debug",       0, 1, shellDebug,                       "Restart the debug port"},
    {"halt",        0, 0, shellHalt,                        "Put the device into standby mode"},
    {"hang",        0, 1, shellHang,                        "Exercise IWDG or software watchdog by hanging, optional argument sets ignoreWatchdog flag"},
    {"reboot",      0, 0, shellReboot,                      "Reboot the device"},
    {"crash",       0, 1, shellCrash,                       "Crash the firmware with something illegal"},
    {"forceGolden", 0, 0, shellForceGolden,                 "Set the forceGolden flag and reboot"},
    {"ls",          0, 0, shellListFiles,                   "List files on the external flash FS"},
    {"df",          0, 1, shellDf,                          "Report bytes used/bytes free on the external flash FS"},
    {"rm",          1, 1, shellRm,                          "Remove a file from the external flash FS"},
    {"crc",         1, 1, shellCrc,                         "Calculate the CRC of a file on the external flash FS, updating metadata"},
    {"format",      1, 1, shellFormat,                      "Format the external flash FS"},
    {"xxd",         1, 2, shellXxd,                         "Hexdump filename [optional -l for little endian"},
    {"bledebug",    0, 1, shellBLEdebug,                    "Toggle the xxd dumping of BLE packets"},
    {"power",       0, 1, shellPower,                       "Query or set device power mode"},
    {"watchdog",    0, 1, shellWatchdog,                    "Query or set watchdog"},
    {"sleep",       1, 1, shellSleep,                       "Sleep for N seconds"},
    {"time",        0, 0, shellTime,                        "Print time in ticks"},
    {"timers",      0, 0, shellTimers,                      "Print time remaining for timers"},
    {"timerstats",  0, 0, shellTimerstats,                  "Print some timer stats"},
    {"boardRev",    0, 0, shellBoardRev,                    "Print the hardware board revision"},
    {"blestatus",   0, 0, shellBLEStatus,                   "Print stats from the BLE driver"},
    {"dumplog",     0, 0, shellDumpLog,                     "Dump log"},
    {"setdate",     2, 2, shellSetDate,                     "Set time and date"},
    {"setLang",     1, 1, shellSetLanguage,                 "Set language 0-4, english, french, spanish, german"},
    {"date",        0, 0, shellDate,                        "Print time and date"},
    {"w",           0, 0, shellWatch,                       "Toggle watching the clock"},
    {"stack",       0, 0, shellStack,                       "Print highwater marks for task stacks"},
    {"wwdg",        0, 2, shellWwdg,                        "Induce a window watchdog reset, optional argument sets ignoreWatchdog flag"},
    {"disablewd",   0, 0, shellDisableIWDG,                 "Disable IWDG and reboot"},
};

const int NUM_CMDSHELL = sizeof(CmdShellTab) / sizeof(CmdShell);

static SemaphoreHandle_t xTx_mutex = NULL;
static StaticSemaphore_t xTx_mutex_buffer;
static TimerHandle_t     ShellDebugTimer;
static StaticTimer_t     ShellDebugTimerBuffer;
static void              ShellDebugTimerCB(TimerHandle_t timer);

bool bDebugStart = true;
bool bTestStart  = false;
bool bBLEdebug   = false;

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////

static void removeBsDel(uint8_t Buffer[]);
int         makeArgv(uint8_t *cmdline, uint8_t *argv[]);
bool        commandInterpreter(uint8_t *Buffer);

/**
 * @brief Initialize the shell
 */
bool init_Shell(void)
{
    if (xTx_mutex == NULL) {
        xTx_mutex = xSemaphoreCreateMutexStatic(&xTx_mutex_buffer);
        configASSERT(xTx_mutex);
        xSemaphoreGive(xTx_mutex);
    }

    if (ShellDebugTimer == NULL) {
        ShellDebugTimer =
            xTimerCreateStatic("ShellDebugTimer", 30000, pdFALSE, NULL,
                               ShellDebugTimerCB, &ShellDebugTimerBuffer);
        xTimerStart(ShellDebugTimer, 200);
    }

    init_ShellComPort();

    return true;
}

static void ShellDebugTimerCB(TimerHandle_t timer)
{
    log_Shell("%s: nobody cares... turning off debug output", __func__);
    bDebugStart = false;
}

/**
 * @brief Interpret new shell data
 * @details Analyze shell data received and decide whether it's ready to respond
 */
bool task_Shell(void)
{
    bool     bStatus = false;
    uint8_t *data;

    if (!bTestStart) {
        if (manageEcho_ShellComPort() == false) return false;
    }
    if (isDataReady_ShellComPort(&bStatus) == false) return false;

    if (bStatus == true) {
        if (getBufferHandle_ShellComPort(&data) == false) return false;
        if (commandInterpreter(data) == false) return false;
        if (clearRXBuffer_ShellComPort() == false) return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief      ShellDebug
////////////////////////////////////////////////////////////////////////////////
void shellDebug(int argc, char **argv)
{
    if (argc < 2) {
        bDebugStart ^= 1;
    } else {
        bDebugStart = atoi(argv[1]);
    }
    if (xTimerIsTimerActive(ShellDebugTimer)) {
        xTimerStop(ShellDebugTimer, 200);
    }
}

/*! @brief Tell the bootloader to halt the device
 */
static void shellHalt(int ac, char **av)
{
    BF->halt = 1;
    reboot(eRebootShellHalt);
}

/*! @brief Induce a watchdog reset by hanging.

    If there is an argument, then set the ignoreWatchdog flag
    If the IWDG is still enabled, it is expected that the watchdog
    reset will be from hardware. Otherwise, it is expected that the
    software watchdog will handle the reset. See shellWwdg() for
    a function to exercise the WWDG.
*/
static void shellHang(int ac, char **av)
{
    if (ac > 1) BF->ignoreWatchdog = 1;
    for (;;) asm("nop");
}

/*! @brief Normal reboot
 */
static void shellReboot(int ac, char **av)
{
    reboot(eRebootShellReboot);
}

// _not_ static so that compiler doesn't inline it inside shellCrash()
void alignBug(time_t *t)
{
    uint32_t i = 0;
    uint64_t t64 = 0x2122232425262728;
    *t = t64;
    // i = *t | 2728;
    log_Shell("part of t is: %lx", i);
}

// buffer pre-filled with some goo
char p[] = "0123456789abcdef012345";

/*! @brief   Try to induce a crash
 *  @details There are several possible ways to crash (controlled
 *           by #ifdef). The first is a simple stack overflow/
 *           over-write.
 *           The second computes a pointer (address 0xffffffe8)
 *           and does some reading and writing.
 *           The last is an unaligned dereference of a 64-bit
 *           integer.
 */
static void shellCrash(int ac, char **av)
{
// #define NEGATIVE_ADDRESS
#ifdef STACK_SMASH
    char buf[60480];

    // mash the stack -- we expect to crash pretty quickly
    memset(buf + 128, 0, sizeof(buf) - 128);
    printf("%s\n", buf);
#elif defined(NEGATIVE_ADDRESS)
    uint32_t *p = NULL;
    log_Shell("p - 6: %p", p - 6);
    *(p - 6) = 42;              // experimentally, crash happens here
    uint32_t v = *(p - 6);      // crash fault points here
    // should not get here
    log_Shell("Expected something bad to happen...");
    log_Shell("Dereferencing *(p-6): %" PRIu32, v);
#else
    if (ac > 1) BF->ignoreWatchdog = 1;
    // Make unaligned pointer
    void *v = p + 3;

    log_Shell("p: %lx, v: %lx", (uint32_t)p, (uint32_t)v);
    alignBug((time_t*)v); // should not return

    log_Shell("Should have crashed... but apparently we didn't");
    log_Shell("Expecting p: 012('&%%$#\"!bcdef012345, have p: %s", p);
#endif
}

/*! @brief      Hang the system to induce a window watchdog reset
 *  @details    Induce the watchdog reset by stopping the
 *              FreeRTOS scheduler and entering a tight loop
 *              to wait for the watchdog reset.
 *              
 *              Use `wwdg 1` to induce the watchdog but tell the
 *              bootloader to _not_ downgrade to golden.
 *              
 *              Use `wwdg 1 1` to test wwdg skipping by allowing the
 *              FreeRTOS scheduler to run periodically. This tests
 *              the WWDG skipping logic.
 */
static void shellWwdg(int ac, char **av)
{
    if (ac == 2) BF->ignoreWatchdog = 1;

    if (ac > 2) {
        // Give freertos a little time to run
        for (;;) {
            vTaskSuspendAll();
            for (int i = 0; i < 1000000; i++) asm("nop");
            xTaskResumeAll();
            for (int i = 0; i < 100000; i++) asm("nop");
        }
    } else {
        vTaskSuspendAll();
        for (;;) asm("nop");
    }
}

/*! @brief  Tell the bootloader to re-install the Golden FW
 */
static void shellForceGolden(int ac, char **av)
{
    BF->forceGolden = 1;
    reboot(eRebootShellForceGolden);
}

/*! @brief  List all the files
 */
static void shellListFiles(int ac, char **av)
{
    spiffs_DIR              d;
    struct spiffs_dirent    de;
    fsMeta_t                *meta = (fsMeta_t *)&de.meta;

    if (!opendir("/", &d)) {
        printf("WTF!\n");
    }
    printf("Size       CRC      Name\n");
    while (readdir(&d, &de)) {
        printf("%10d %08lx %s\n", de.size, meta->crc, de.name);
    }

    closedir(&d);
    return;
}

/*! @brief  Simulate df command, accepts an optional -k
 */
static void shellDf(int ac, char **av)
{
    uint32_t    total, used;
    bool        kflag = false;
    fsinfo_FSUtil(&total, &used);

    if (ac > 1 && strcmp(av[1], "-k") == 0) {
        kflag = true;
        total /= 1024;
        used /= 1024;
    }

    printf("%lu", used);
    printf(kflag ? "K " : " bytes ");
    printf("used of %lu", total);
    printf(kflag ? "K " : " bytes ");
    printf("total\n");
    return;
}
/*! @brief  Remove a file
 */
static void shellRm(int ac, char **av)
{
    int     ret;

    // shouldn't happen because wrapper insists on an argument
    if (ac != 2) {
        printf("WTF!\n");
        return;
    }

    if ((ret = unlink(av[1]))) {
        printf("unlink returned error: %d\n", ret);
    }
    return;
}

/*! @brief  Print the CRC of a file
 */
static void shellCrc(int ac, char **av)
{
    spiffs_file fh;
    spiffs_stat statbuf;
    fsMeta_t    *meta = (fsMeta_t *)&statbuf.meta;
    uint32_t    crc = 0;
    uint32_t    start, end;

    if (ac != 2) {
        printf("crc: Needs a file argument\n");
    }

    // start = HAL_GetTick();
    start = xTaskGetTickCount();
    fh = open(av[1], O_RDWR, 0);
    if (fh < 0) {
        printf("crc: open of %s failed with %d\n", av[1], fh);
        return;
    }

    fstat(fh, &statbuf);
    crc = fcrc_FSUtil(fh);
    if (crc != meta->crc) {
        meta->crc = crc;
        printf("crc: updating metadata\n");
        int ret = fupdateMeta_FSUtil(fh, meta);
        if (ret < 0) {
            printf("fupdateMeta failed with %d\n", ret);
        }
    }

    close(fh);

    // end = HAL_GetTick();
    end = xTaskGetTickCount();

    printf("%s: %lx\n", av[1], crc);
    printf("Elapsed time: %lu", end - start);

    return;
}

static void shellFormat(int ac, char **av)
{
    if (ac != 2 || strcmp(av[1], "--very-sure")) {
        printf("format: requires --very-sure argument\n");
        return;
    }
    fsFormat_SystemUtilities();
}

/*! @brief  Print a line of xxd(hexdump) style data

    xxd outputs data like kind of like this:

    00001160: 4152 4d2e 6174 7472 6962 7574 6573 2030  ARM.attributes 0

    This function prints the data portion only, assuming
    that the caller has already printed whatever offset
    prefix they may have wanted.

    The littleEndian argument is there to emulate the byte-order
    weirdness that one sees when using the 'od -x' command in
    *nix command lines. (Probably only interesting to grey-beards
    like me.)

    @param  buf pointer to some bytes
    @param  littleEndian if true, present 16bits as littleEndian
    @param  len number of bytes buf points to -- must be <= than 16
*/
static void xxdLine(void *buf, bool littleEndian, uint32_t len)
{
    uint16_t   *p = (uint16_t *)buf;
    char       *cp = (char *)buf;
    char       ascii[17];          // 16 ASCII characters + null
    int        asciiIdx = 0;
    char       hextmp[5];
    char       hexout[41];         // (4 hex digits + 1 space) * 8 + null
    char       *hexp = hexout;

    // quietly avoid the crash
    if (len > 16)
        return;

    memset(ascii, 0, sizeof(ascii));
    memset(hexout, 32, 40);        // pre-fill with spaces
    hexout[40] = 0;                // terminate

    while (len > 0) {
        // special case, only one byte left because odd number
        if (len == 1) {
            sprintf(hextmp, "%02x  ", *cp);
            memcpy(hexp, hextmp, 4);
            ascii[asciiIdx] = (*cp > 32 || *cp > 0x80) ? *cp : '.';
            len--;
            break;
        }
        sprintf(hextmp, "%04lx", littleEndian ? *p : __REV16(*p));
        memcpy(hexp, hextmp, 4);
        for (int i = 0; i < 2; i++) {
            ascii[asciiIdx] = (*cp > 32 && *cp < 0x80) ? *cp : '.';
            asciiIdx++; cp++;
        }
        p++;
        hexp += 5;
        len -= 2;
    }
    printf("%s %s\n", hexout, ascii);
}

/*! @brief  Print a buffer in xxd(hexdump) style

            Only runs if bBLEdebug is true (toggled
            by `bledebug` shell command)

    @param  buf pointer to a buffer
    @param  len length of buffer
*/
void xxd_Shell(void *buf, uint32_t len)
{
    char *   p      = (char *)buf;
    uint32_t offset = 0;
    int      i;

    if (!bBLEdebug) return;

    for (i = len; i > 0; i -= 16, p += 16, offset += 16) {
        printf("%08lx: ", offset);
        xxdLine(p, false, i >= 16 ? 16 : i);
    }
}

static void shellXxd(int ac, char **av)
{
    char *      file = av[1], buf[16];
    int32_t     readlen;
    bool        littleEndian = false;
    uint32_t    offset       = 0;
    spiffs_file fh;

    if (ac == 3) {
        if (strcmp(av[1], "-l")) {
            printf("xxd: invalid option: %s\n", av[1]);
            return;
        } else {
            file         = av[2];
            littleEndian = true;
        }
    }

    fh = open(file, O_RDONLY, 0);
    if (fh < 0) {
        printf("xxd: failed to open file: %s, returned: %d\n", file, fh);
        return;
    }
    do {
        memset(buf, 0, sizeof(buf));
        readlen = read(fh, buf, sizeof(buf));
        printf("%08lx: ", offset);
        xxdLine(buf, littleEndian, readlen);
        offset += sizeof(buf);
    } while (readlen == sizeof(buf));
    close(fh);
}

void shellBLEdebug(int ac, char **av)
{
    if (ac < 2) {
        bBLEdebug ^= 1;
    } else {
        bBLEdebug = atoi(av[1]);
    }
}

/*! brief   Report or set power saving state

            With no argument, simply report the current
            power state.
            With a 0 or a 1, set the power on or off accordingly.
            If turning on the power, disable automatic power management.
*/
static void shellPower(int ac, char **av)
{
    extern volatile bool    powerState;
    extern bool             disableAutoPower;
    bool                    onoff;

    if (ac == 2) {
        onoff            = atoi(av[1]) ? true : false;
        disableAutoPower = onoff;
        setState_Power(onoff);
    }

    printf("Power mode: %s\n", powerState ? "on" : "off / powersave");
}

/*! brief   Report or set disable watchdog flag

            With no argument, simply report the current disable watchdog flag.

            With a 0 or a 1, set the disable watchdog flag on or off
            accordingly. Reboot required for flag to take effect.
*/
static void shellWatchdog(int ac, char **av)
{
    if (ac == 2) {
        BF->disableWatchdog = atoi(av[1]) ? true : false;
    }

    // printf("Watchdog: %lx\n", BF->all);
    printf("Watchdog: %s\n", BF->disableWatchdog ? "disabled" : "enabled");
}

static void shellSleep(int ac, char **av)
{
    int sleepTime = atoi(av[1]) * 1000;

    printf("Sleep tick:   %ld\n", xTaskGetTickCount());
    vTaskDelay(sleepTime);
    printf("Wake-up tick: %ld\n", xTaskGetTickCount());
}

static void shellTime(int ac, char **av)
{
    printf("Ticks: %ld\n", xTaskGetTickCount());
}

static void shellTimers(int ac, char **av)
{
    timeLeft_SHTimers();
}

extern uint32_t graphicTaskCBGiveCount;
extern uint32_t graphicTaskCBTakeCount;
extern uint32_t graphicTaskCBOvertimeCount;
extern uint32_t graphicTaskCBTotalTime;
extern uint32_t graphicTaskCBTotalOvertime;
extern uint32_t graphicTaskCBMinTime;
extern uint32_t graphicTaskCBMaxTime;
extern uint32_t graphicTaskCBFirstTick;    // tick of first CB
extern uint32_t graphicTaskCBFirstClock;   // clocks.animation of first CB
extern uint32_t graphicTaskCBAnimClock;    // independent tracking of clocks.animation
extern uint32_t graphicTaskBacklog;
extern uint32_t lostTicks;
extern uint32_t commTaskCount;

static void shellTimerstats(int ac, char **av)
{
    printf("Ticks:                      %10ld\n", xTaskGetTickCount());
    printf("graphicTaskCBGiveCount:     %10lu\n", graphicTaskCBGiveCount);
    printf("graphicTaskCBTakeCount:     %10lu\n", graphicTaskCBTakeCount);
    printf("graphicTaskCBAnimClock:     %10ld\n", graphicTaskCBAnimClock);
    printf("graphicTaskCBOvertimeCount: %10lu\n", graphicTaskCBOvertimeCount);
    printf("graphicTaskCBAvgTime:       %10lu\n", graphicTaskCBTotalTime / graphicTaskCBTakeCount);
    printf("graphicTaskCBAvgOvertime:   %10lu\n", graphicTaskCBTotalOvertime / graphicTaskCBOvertimeCount);
    printf("graphicTaskCBMinTime:       %10lu\n", graphicTaskCBMinTime);
    printf("graphicTaskCBMaxTime:       %10lu\n", graphicTaskCBMaxTime);
    printf("graphicTaskCBFirstTick:     %10lu\n", graphicTaskCBFirstTick);
    printf("graphicTaskCBFirstClock:    %10lu\n", graphicTaskCBFirstClock);
    printf("graphicTaskBacklog:         %10ld\n", graphicTaskBacklog);
    printf("lostTicks:                  %10ld\n", lostTicks);
    printf("commTaskCount:              %10ld\n", commTaskCount);
}

static void shellBoardRev(int ac, char **av)
{
    printf("Board Revision: %d\n", get_BoardRev());
}

static void shellBLEStatus(int ac, char **av)
{
    printBLEStatus_BLEDriver();
}

static void shellDumpLog(int ac, char **av)
{
    dump_deviceTelemetry();
}
static void shellSetLanguage(int ac, char **argv)
{
    uint8_t code = atoi(argv[1]);
    if(code == 0)
        setLocale_SystemUtilities(english);
    else if(code == 1)
        setLocale_SystemUtilities(french);
    else if(code == 2)
        setLocale_SystemUtilities(german);
    else if(code == 3)
        setLocale_SystemUtilities(spanish);
    else
        log_Shell("Invalid value");
}
static void shellSetDate(int ac, char **av)
{
    const char *errstr = "Error parsing %s, sscanf returned %d\n"
                         "Enter date and time in precisely this form: yy-mm-dd hh:mm:ss";
    extern RTC_HandleTypeDef hrtc;

    RTC_TimeTypeDef sTime1;
    RTC_DateTypeDef sDate1;

    unsigned y, mon, d, h, min, s;
    int ret;

    ret = sscanf(av[1], "%u-%02u-%02u", &y, &mon, &d);
    if (ret != 3) {
        log_Shell(errstr, "date", ret);
        return;
    }
    ret = sscanf(av[2], "%02u:%02u:%02u", &h, &min, &s);
    if (ret != 3) {
        log_Shell(errstr, "time", ret);
        return;
    }
    sDate1.Year = y > 2000 ? y - 2000 : y;
    sDate1.Month = mon;
    sDate1.Date = d;
    sTime1.Hours = h;
    sTime1.Minutes = min;
    sTime1.Seconds = s;
    HAL_RTC_SetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);
}

static void shellDate(int ac, char **av)
{
    extern RTC_HandleTypeDef hrtc;

    RTC_TimeTypeDef sTime1;
    RTC_DateTypeDef sDate1;

    HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);

    uint32_t ms =
        (((float)(sTime1.SecondFraction - sTime1.SubSeconds)) /
                   ((float)(sTime1.SecondFraction + 1)) * 1000);

    log_Shell("20%02d %02d %02d %02d:%02d:%02d.%03ld",
              sDate1.Year, sDate1.Month, sDate1.Date,
              sTime1.Hours, sTime1.Minutes, sTime1.Seconds, ms);
}

bool watch = false;
static void shellWatch(int ac, char **av)
{
    watch ^= 1;
}

static void shellStack(int ac, char **av)
{
    printTasks_SHTaskUtils();
}

static void shellDisableIWDG(int ac, char **av)
{
    BF->disableWatchdog = 0;
    BF->ignoreWatchdog = 1;
    disableWatchdog_WatchdogTask();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief      ShellTest, special mode only byte responses
////////////////////////////////////////////////////////////////////////////////
void shellTest (int argc, char **argv)
{
    extern bool disableAutoPower;

    if (argc < 2) {
        bTestStart ^= 1;
    } else {
        bTestStart = atoi(argv[1]) ? true : false;
    }
    disableAutoPower = bTestStart;
    setState_Power(bTestStart);
    setTestMode_DebugConsole(bTestStart);
    startLEDTest_GraphicsTask(0,255,255);
    testOLEDByTurningOff_GraphicsTask();
    enableEarlyHalt_SystemUtilities();
}

void shellVersion (int argc, char **argv)
{
    uint8_t version[4];
    memcpy(version, getVersion_CommunicationTask(), 4);

    if(bTestStart){
        printf("%c%c%c%c", version[0],version[1],version[2],version[3]);
        printf("%c%c%c%c", MCB->version.blVersionMajor, MCB->version.blVersionMinor,
                MCB->version.blVersionRevision, MCB->version.blVersionCommit);
    }else{
        printf("\r\n");
        printf("Shell : %d.%d",
                SHELL_VERSION_MAJOR,
                SHELL_VERSION_MINOR);
        printf("\r\n");
        printf("FW : %d.%d.%d.%d",version[0],version[1],version[2],version[3]);
        printf("\r\n");
        printf("BL: %d.%d.%d.%d\n",
                MCB->version.blVersionMajor, MCB->version.blVersionMinor,
                MCB->version.blVersionRevision, MCB->version.blVersionCommit);
        printf("\r\n");
    }
}

void shellSerial (int argc, char **argv)
{
    if(bTestStart){
        for(int i=0; i<8; i++){
            printf("%c", MCB->serial[i]);
        }
    }else{
        printf("\r\n");
        printf("Serial : %.8s", MCB->serial);
        printf("\r\n");
    }
}

void shellHelp (int argc, char **argv)
{
    int index;

    (void)argc;
    (void)argv;

    printf("\r\n");
    printf("    CMD     - Description\r\n");
    printf("    ---------------------\r\n");
    for (index = 0; index < NUM_CMDSHELL; index++)
    {
        //printf(HELPFORMATSHELL,
        printf("    ");
        printf(CmdShellTab[index].cmd);
        printf(" - ");
        printf(CmdShellTab[index].description);
        printf("\r\n");
    }
}

static void removeBsDel (uint8_t Buffer[])
{
    int pos = 0;
    int k;
    while (*(Buffer+pos) != 0)
    {
        switch (*(Buffer+pos))
        {
            case 0x08:      /* Backspace */
            case 0x7F:      /* Delete */

                if (pos == 0)
                {
                    k = 0;
                    while (*(Buffer+k) != 0)
                    {
                        *(Buffer+k) = *(Buffer+k+1);
                        k++;
                    }
                    *(Buffer+k+1) = 0;
                    pos = -1;
                }

                if (pos > 0)
                {
                    k = pos;
                    while (*(Buffer+k) != 0)
                    {
                        *(Buffer+k-1) = *(Buffer+k+1);
                        k++;
                    }
                    *(Buffer+k+1) = 0;
                    pos = pos - 2;
                }
                break;
        }
        pos++;
    }
}

int makeArgv (uint8_t *cmdline, uint8_t *argv[])
{
    int argc, i, in_text;

    /*
     * Break cmdline into strings and argv
     * It is permissible for argv to be NULL, in which case
     * the purpose of this routine becomes to count args
     */

    argc = 0;
    i = 0;
    in_text = false;

    while (cmdline[i] != '\0')  /* getline() must place 0x00 on end */
    {
        if (((cmdline[i] == ' ') || (cmdline[i] == '\t')) )
        {
            if (in_text)
            {
                /* end of command line argument */
                cmdline[i] = '\0';
                in_text = false;
            }
            else
            {
                /* still looking for next argument */

            }
        }
        else
        {
            /* got non-whitespace character */
            if (in_text)
            {
            }
            else
            {
                /* start of an argument */
                in_text = true;
                if (argc < MAX_ARGS)
                {
                    if (argv != NULL)
                        argv[argc] = &cmdline[i];
                    argc++;
                }
                else
                    /*return argc;*/
                    break;
            }

        }
        i++;    /* proceed to next character */
    }
    if (argv != NULL)
        argv[argc] = NULL;
    return argc;
}

bool commandInterpreter(uint8_t * Buffer)
{
    int argc;

    uint8_t *argv[MAX_ARGS + 1];    /* One extra for NULL terminator */

    if (Buffer == NULL) return false;

    removeBsDel(Buffer);

    argc = makeArgv(Buffer,argv);

    if (argc)
    {
        int i;
        for (i = 0; i < NUM_CMDSHELL; i++)
        {
            if (strcasecmp(CmdShellTab[i].cmd,(char const *)argv[0]) == 0)
            {
                if (((argc-1) >= CmdShellTab[i].min_args) && ((argc-1) <= CmdShellTab[i].max_args))
                {
                    CmdShellTab[i].func(argc,(char**)argv);
                    goto END;
                }
                else
                {
                    printf(SYNTAXSHELL,argv[0]);
                    goto END;
                }
            }
        }
        printf(INVCMD,argv[0]);
        printf(HELPMSG);
    }
    END:
    if(!bTestStart) printf(PROMPT);

    return true;
}

/**
 * @brief Send to the UART Console
 * @details Take a string and print it via UART
 */
int log_Shell(const char* format,...){
    bool bResponse = false;

    if (!bDebugStart || bTestStart) return CONSOLE_SUCCESS;

    if (xTx_mutex) {
        if (xSemaphoreTake(xTx_mutex, 50) == pdTRUE) {
            static char message[CONSOLE_MAX_DATA_SIZE];
            int len;
            va_list list;
            va_start(list, format);
            len = vsnprintf(message, CONSOLE_MAX_DATA_SIZE, format, list);
#if SHELL_SEND_NEW_LINE
            if (len < (CONSOLE_MAX_DATA_SIZE - 3)) {
                message[len++] = '\r';
                message[len++] = '\n';
                message[len++] = 0;
            } else {
                len = CONSOLE_MAX_DATA_SIZE;
            }
#endif
            va_end(list);
            HAL_UART_Write_DMA(&DEBUG_PORT, (uint8_t *)message, len, &bResponse);
            xSemaphoreGive(xTx_mutex);
        } else {
            return CONSOLE_TX_BUSY;
        }
    } else
        return CONSOLE_BAD_PARAMETER;

    return CONSOLE_SUCCESS;
}

