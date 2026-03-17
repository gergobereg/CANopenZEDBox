#include "CO_application.h"
#include "OD.h"
#include "301/CO_PDO.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void app_processTerminalCommand(CO_t* co, const char* line) {
    char* end = NULL;
    long value = 0;

    if (strncmp(line, "PDO3800=", 8) == 0) {
        errno = 0;
        value = strtol(&line[8], &end, 0);
        if (errno == 0 && end != &line[8] && *end == '\0' && value >= 0 && value <= 255) {
            CO_LOCK_OD(co->CANmodule);
            OD_RAM.x3800_digital_Inputs1[0] = (uint8_t)value;
            CO_UNLOCK_OD(co->CANmodule);
            CO_TPDOsendRequest(&co->TPDO[0]);
            printf("TPDO 0x3800:01 set to %ld\n", value);
        } else {
            printf("Invalid PDO3800 value. Use 0..255.\n");
        }
        fflush(stdout);
    } else if (strncmp(line, "PDO3801=", 8) == 0) {
        errno = 0;
        value = strtol(&line[8], &end, 0);
        if (errno == 0 && end != &line[8] && *end == '\0' && value >= -32768 && value <= 32767) {
            CO_LOCK_OD(co->CANmodule);
            OD_RAM.x3801_digital_Inputs2[0] = (int16_t)value;
            CO_UNLOCK_OD(co->CANmodule);
            CO_TPDOsendRequest(&co->TPDO[1]);
            printf("TPDO 0x3801:01 set to %ld\n", value);
        } else {
            printf("Invalid PDO3801 value. Use -32768..32767.\n");
        }
        fflush(stdout);
    } else if (line[0] != '\0') {
        printf("Unknown command. Use PDO3800=<0..255> or PDO3801=<-32768..32767>.\n");
        fflush(stdout);
    }
}

CO_ReturnError_t app_programStart(uint16_t* bitRate, uint8_t* nodeId, uint32_t* errInfo) {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);

    (void)bitRate;
    (void)nodeId;
    (void)errInfo;

    if (flags >= 0) {
        (void)fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    return CO_ERROR_NO;
}

void app_communicationReset(CO_t* co) {
    (void)co;
}

void app_programEnd() {}


// USER APPLICATION
void app_programAsync(CO_t* co, uint32_t timer1usDiff) {
    static char cmdBuf[128];
    static size_t cmdLen = 0;
    char readBuf[64];
    ssize_t bytesRead = 0;

    (void)timer1usDiff;

    /* PLC -> Linux (RPDO mapped to 0x3000:01) */
    uint8_t plc_out;
    CO_LOCK_OD(co->CANmodule);
    plc_out = OD_RAM.x3000_digital_Outputs1[0];
    CO_UNLOCK_OD(co->CANmodule);

    /* Linux -> PLC (TPDO mapped to 0x3800:01 and 0x3801:01) */
    static uint8_t last_in = 0;
    static uint8_t last_out = 0;

    if (plc_out != last_out) {
        printf("RPDO 0x3000:01 changed: 0x%02X\n", plc_out);
        fflush(stdout);
        last_out = plc_out;
    }

    (void)last_in;

    while ((bytesRead = read(STDIN_FILENO, readBuf, sizeof(readBuf))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            char ch = readBuf[i];

            if (ch == '\r') {
                continue;
            }
            if (ch == '\n') {
                cmdBuf[cmdLen] = '\0';
                app_processTerminalCommand(co, cmdBuf);
                cmdLen = 0;
                continue;
            }
            if (cmdLen < (sizeof(cmdBuf) - 1U)) {
                cmdBuf[cmdLen++] = ch;
            } else {
                printf("Command too long.\n");
                fflush(stdout);
                cmdLen = 0;
            }
        }
    }

    if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        printf("Terminal read error.\n");
        fflush(stdout);
    }
}

void app_programRt(CO_t* co, uint32_t timer1usDiff) {
    (void)co;
    (void)timer1usDiff;
}
