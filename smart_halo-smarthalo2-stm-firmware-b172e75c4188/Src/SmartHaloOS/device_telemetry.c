/**
 * @defgroup   DEVICE_TELEMETRY device telemetry
 *
 * @brief      This file implements device telemetry.
 *
 * @author     Georg Nikodym
 * @date       2020
 */

#include <stdbool.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <time.h>

#include "device_telemetry.h"
#include "Shell.h"
#include "SystemUtilitiesTask.h"
#include "CommunicationTask.h"
#include "FSUtil.h"
#include "rtcUtils.h"

#define TMPLOG_LEN  10
#define NUM_ELEM    100

const char devlogFile[] = "devlog";

typedef struct {
    uint8_t         full;
    uint8_t         idx;
    uint8_t         size;
    telemetry_log_t entries[NUM_ELEM];
} telemetry_logBuf_t;

static telemetry_log_t    tmplog[TMPLOG_LEN];  // temp for early, preFS logs
static telemetry_logBuf_t logBuf = {
    .size = NUM_ELEM,
};
static bool initCalled;
static bool pendingUpload;

static uint8_t  sequenceNumber;

static const char *event2str(telemetry_event_type_t event_type);
static void        addLog2Ring(const telemetry_log_t *log);
static size_t      mkTelemetryMsg(char *msg, size_t sz, telemetry_log_t *log);
static size_t      mkCrashDumpMsg(const char *ts, char *msg, size_t sz, telemetry_log_t *log);

/**
 * @brief      Is the log empty?
 * @param[in]  log   The log
 * @return     True if the specified log is empty
 */
static inline bool isLogEmpty(const telemetry_log_t *log) {
    if (log->timestamp == 0 && log->type == eNOP && log->arg == 0) {
        return true;
    }
    return false;
}

/**
 * @brief      Initializes the device telemetry data structure
 * @details    Read any previously saved event logs from the filesystem. Re-play
 *             events collected before the FS was available into the log.
 */
void init_deviceTelemetry(void)
{
    initCalled = true;

    if (!readFile_SystemUtilities(devlogFile, &logBuf, sizeof(telemetry_logBuf_t))) {
        log_Shell("%s: readFile on devlog failed -- all good", __func__);
    }

    // the size of the data read in does not match what we are expecting/compiled for...
    // toss it
    if (logBuf.size != NUM_ELEM) {
        log_Shell("%s: logBuf.size %" PRIu8 " does not match %" PRIu8, __func__, logBuf.size, NUM_ELEM);
        memset(&logBuf, 0, sizeof(telemetry_logBuf_t));
        logBuf.size = NUM_ELEM;
        unlink(devlogFile);
    }

    for (int i = 0; !isLogEmpty(&tmplog[i]) && i < TMPLOG_LEN; i++) {
        addLog2Ring(&tmplog[i]);
    }
}

/**
 * @brief   Get time from RTC, convert to time_t
 * @return  time_t time as of now
 */
static inline time_t rtc2time(void)
{
    extern RTC_HandleTypeDef hrtc;

    RTC_TimeTypeDef sTime1;
    RTC_DateTypeDef sDate1;

    HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);
    struct tm nowtm = {
        .tm_wday = sDate1.WeekDay,
        .tm_mon  = sDate1.Month - 1,
        .tm_mday = sDate1.Date,
        .tm_year = sDate1.Year + 100,
        .tm_hour = sTime1.Hours,
        .tm_min  = sTime1.Minutes,
        .tm_sec  = sTime1.Seconds,
    };

    return mktime(&nowtm);
}

/**
 * @brief      Logs an event to the device telemetry log
 *
 * @param[in]  event_type  The event type
 * @param[in]  arg         The argument
 */
void log_deviceTelemetry(telemetry_event_type_t event_type, uint32_t arg)
{
    uint32_t now = rtc2time();  // XXX Y2038 bug, does anyone care?
    uint8_t soc = getStateOfCharge_SystemUtilities();
    telemetry_log_t log = {
        .timestamp = now,
        .type = event_type,
        .arg = arg,
        .uploaded = false,
        .soc = soc
    };

    // Early boot days, no FS, no tasks
    if (!isFSMounted_SystemUtilities()) {
        static int tmpidx;
        if (tmpidx == TMPLOG_LEN - 1) {
            printf("%s: temporary log would overflow\n", __func__);
            return;
        }
        tmplog[tmpidx++] = log;
        return;
    }

    addLog2Ring(&log);
    pendingUpload = true;
}

/**
 * @brief      Adds a log entry to the ring buffer.
 * @details    Adds an event log entry to the ring buffer. Certain event
 *             types will trigger the writing of the ring buffer to the
 *             filesystem.
 * @param      log   The log entry.
 */
static void addLog2Ring(const telemetry_log_t *log)
{
    if (!initCalled) {
        init_deviceTelemetry();
    }

    logBuf.entries[logBuf.idx++] = *log;

    if (logBuf.idx == NUM_ELEM) {
        logBuf.full = true;
        logBuf.idx = 0;
    }

    // on certain events where we will be intentionally stopping or
    // resetting the processor, we assume that we still have a working
    // filesystem...
    switch (log->type) {
    case eSLEEP:
    case eWAKEUP:
    case eHALT:
    case eREBOOT:
        store_deviceTelemetry();
    default:
        break;
    }
}

/**
 * @brief   string table for telemetry_event_type_t
 * @details Must match order of definitions in the enum
 */
static const char *eventStringTable[] = {
    "eNOP",
    "eSLEEP",
    "eWAKEUP",
    "eCRASH",
    "eREBOOT",
    "eBOOTREASON",
    "eHALT",
    "ePAIR",
    "eUNPAIR",
    "eCONNECT",
    "eDISCONNECT",
    "eSOFTWD",
    "eSTACKOVF",
    "eSTACKHW_0",   // see SHTaskUtils.h to convert to name
    "eSTACKHW_1",
    "eSTACKHW_2",
    "eSTACKHW_3",
    "eSTACKHW_4",
    "eSTACKHW_5",
    "eSTACKHW_6",
    "eSTACKHW_7",
    "eSTACKHW_8",
    "eSTACKHW_9",
    "eSTACKHW_A",
    "eSTACKHW_B",
    "eSTACKHW_C",
    "eSTACKHW_D",
    "eSTACKHW_E",
    "eWWDG",
    "eCRASHADDR",
    "eCRASHD0",
    "eCRASHD1",
    "eCRASHD2",
    "eCRASHD3",
    "eIWDG",
    "eSOFTCRASH",
    "eLASTTASK",
    "eSKIPWWDG",
    "eI2CERROR",
};
static const uint32_t eventStringTableSize = sizeof(eventStringTable) / sizeof(eventStringTable[0]);

/**
 * @brief      Translate the event type to a string
 * @param[in]  event_type  The event type
 * @return     a pointer to the name of the event type
 */
static const char *event2str(telemetry_event_type_t event_type)
{
    if (event_type > eventStringTableSize)
        return "unhandled event type";
    return eventStringTable[event_type];
}

/**
 * @brief      For a circular array of NUM_ELEM elements, return next index
 * @param[in]  idx   The current index
 * @return     Next index
 */
static inline int nextIndex(int idx)
{
    if (idx + 1 == NUM_ELEM)
        return 0;
    else
        return idx + 1;
}

/**
 * @brief      Dumps the device log to the console
 */
void dump_deviceTelemetry(void)
{
    telemetry_log_t log;
    char str[64];
    int start = 0;

    if (!initCalled) init_deviceTelemetry();

    log_Shell("Event log\n\tentry size: %u", sizeof(telemetry_log_t));

    if (logBuf.full) {
        start = logBuf.idx;
    }

    log_Shell("\tlogBuf.full: %s, start: %d\n", logBuf.full ? "true" : "false", start);

    for (int i = start, count = 0; count < NUM_ELEM; i = nextIndex(i), count++) {
        log = logBuf.entries[i];
        if (isLogEmpty(&log)) break;
        mkTelemetryMsg(str, sizeof(str), &log);
        log_Shell("LOG: %s%s", str, log.uploaded ? ", uploaded" : "");
    }
}

/**
 * @brief      Stores the event logs to the filesystem
 */
void store_deviceTelemetry(void)
{
    if (!writeFile_SystemUtilities(devlogFile, &logBuf, sizeof(telemetry_logBuf_t))) {
        log_Shell("%s: writeFile failed!!!", __func__);
    }
}

/**
 * @brief      Finds an oldest unsent log entry
 * @details    Since this scan is not free (worst case is a full O(n)),
 *             set pendingUpload flag to false if nothing is found.
 *             Callers can consult this flag to avoid wasting time.
 * @return     Returns a pointer to the oldest, unset log entry
 *             or NULL if everything has been marked sent.
 */
static telemetry_log_t *findOldestUnsent(void)
{
    int start = 0;

    if (logBuf.full) {
        start = logBuf.idx;
    }
    for (int i = start, count = 0; count < NUM_ELEM; i = nextIndex(i), count++) {
        if (isLogEmpty(&logBuf.entries[i])) break;
        if (!logBuf.entries[i].uploaded) {
            return &logBuf.entries[i];
        }
    }

    pendingUpload = false;
    return NULL;
}

static inline char *mktimestamp(time_t t, char *ts, size_t len)
{
    struct tm logtm;
    gmtime_r(&t, &logtm);
    snprintf(ts, len, "%02d:%02d:%02d", logtm.tm_hour, logtm.tm_min, logtm.tm_sec);

    return ts;
}

/**
 * @brief      Uploads the oldest, unsent entry from the event log
 */
void upload_deviceTelemetry(void)
{
    if (!pendingUpload) return;

    telemetry_log_t *log = findOldestUnsent();

    if (!log) return;

    size_t len;
    struct {
        uint8_t type;
        char    text[64];
    } __attribute__((packed)) msg;

    msg.type = BLE_NOTIFY_LOG;
    len = mkTelemetryMsg(msg.text, sizeof(msg.text), log);
    len += 1;  // add one for for msg type

    log_Shell("%s: msg.text: %s, msg len: %u", __func__, msg.text, len);

    sequenceNumber++;  // wrapping is OK, this number is only used to spot
                       // missed events

    if (!sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_NOTIFY,
                                    len, &msg)) {
        log_Shell("%s: sendData failed!!!", __func__);
    }
    log->uploaded = true;
}

/**
 * @brief   Simple string checker
 * @details Make sure that the string contains plain old printable ASCII characters
 *          and that there's a NULL byte before the end.
 *          
 *          Used when passed a random pointer to memory that _might_ contain
 *          a useful string.
 * 
 * @param   p     Pointer to string to be checked.
 * @param   len   Length of the buffer p points to.
 * 
 * @return  true for a printable string
 */
static bool chkString(const char *p, size_t len)
{
    if (!p) return false;
    for (int i = 0; i < len && p[i]; i++) {
        if (p[i] < ' ' || p[i] > 'z') return false;
    }
    return true;
}

/**
 * @brief   Create a message string from a log structure
 * @details Fill buffer msg of size sz with the human readable form of
 *          the log structure.
 *          
 *          Called by both the uploading function and the dumplog shell
 *          function.
 *          
 *          Sequence number is managed by upload_devicetelemetry().
 * 
 * @param   msg Buffer to full
 * @param   sz  Size of msg
 * @param   log Log structure
 *
 * @return  Number of bytes written to buffer, including terminating NULL
 */
static size_t mkTelemetryMsg(char *msg, size_t sz, telemetry_log_t *log)
{
    size_t len;

    if (!msg || !log) return 0;

    char ts[10];
    mktimestamp(log->timestamp, ts, sizeof(ts));

    if (log->type >= eCRASHD0 && log->type <= eCRASHD3) {
        return mkCrashDumpMsg(ts, msg, sz, log);
    }
    len = snprintf(msg, sz, "%s|%02x|%u|%s", ts, sequenceNumber,
                   log->soc, event2str(log->type));
    if ((log->type == eSTACKOVERFLOW && chkString((const char *)log->arg, 16))
        || (log->type == eLASTTASK && chkString((const char *)log->arg, LASTTASK_STRING_LEN))) {
        len += snprintf(msg + len, sz - len, "|%s", (const char *)log->arg);
    } else {
        len += snprintf(msg + len, sz - len, "|0x%" PRIx32,
                        log->arg);
    }
    if (len >= sz) {
        len = sz;  // text was truncated
    } else {
        len += 1;  // one for NULL terminator
    }

    return len;
}

static size_t mkCrashDumpMsg(const char *ts, char *msg, size_t sz, telemetry_log_t *log)
{
    size_t len;
    int startidx = (log->type - eCRASHD0) * 4;

    len = snprintf(msg, sz, "%s|%02x|%u|%s|",
                   ts, sequenceNumber, log->soc, event2str(log->type));

    if (startidx > 16) {
        len += snprintf(msg + len, sz - len, "array overflow");
    } else {
        len += snprintf(msg + len, sz - len, "%08lx,%08lx,%08lx,%08lx",
                        crashDumpData[startidx], crashDumpData[startidx + 1],
                        crashDumpData[startidx + 2], crashDumpData[startidx + 3]);
    }
    if (len >= sz) {
        len = sz;   // text was truncated
    } else {
        len += 1;   // add one for NULL terminator
    }

    return len;
}