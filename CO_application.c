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
    char indexBuf[5];
    char* end = NULL;
    long index = 0;
    long value = 0;

    if (strncmp(line, "PDO", 3) != 0 || strlen(line) < 9 || line[7] != '=') {
        if (line[0] != '\0') {
            printf("Unknown command. Use PDO3800=<value> ... PDO380A=<value>.\n");
            fflush(stdout);
        }
        return;
    }

    memcpy(indexBuf, &line[3], 4);
    indexBuf[4] = '\0';

    errno = 0;
    index = strtol(indexBuf, &end, 16);
    if (errno != 0 || end != &indexBuf[4] || index < 0x3800 || index > 0x380A) {
        printf("Unknown PDO index. Use PDO3800 through PDO380A.\n");
        fflush(stdout);
        return;
    }

    errno = 0;
    value = strtol(&line[8], &end, 0);
    if (errno != 0 || end == &line[8] || *end != '\0') {
        printf("Invalid value format.\n");
        fflush(stdout);
        return;
    }

    CO_LOCK_OD(co->CANmodule);
    switch (index) {
        /* 0x3800:01 Message ID */
        case 0x3800:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3800 Message ID range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3800_digital_Inputs1[0] = (uint8_t)value;
            break;
        /* 0x3801:01 Close Cultivator Left */
        case 0x3801:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3801 Close Cultivator Left range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3801_digital_Inputs2[0] = (uint8_t)value;
            break;
        /* 0x3802:01 Close Cultivator Right */
        case 0x3802:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3802 Close Cultivator Right range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3802_digital_Inputs3[0] = (uint8_t)value;
            break;
        /* 0x3803:01 Setpoint Height Left */
        case 0x3803:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3803 Setpoint Height Left range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3803_digital_Inputs4[0] = (uint8_t)value;
            break;
        /* 0x3804:01 SetPoint Height Right */
        case 0x3804:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3804 SetPoint Height Right range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3804_digital_Inputs5[0] = (uint8_t)value;
            break;
        /* 0x3805:01 ActPos Height Left */
        case 0x3805:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3805 ActPos Height Left range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3805_digital_Inputs6[0] = (uint8_t)value;
            break;
        /* 0x3806:01 ACtPos Height Right */
        case 0x3806:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3806 ACtPos Height Right range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3806_digital_Inputs7[0] = (uint8_t)value;
            break;
        /* 0x3807:01 GreenDuty Left */
        case 0x3807:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3807 GreenDuty Left range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3807_digital_Inputs8[0] = (uint8_t)value;
            break;
        /* 0x3808:01 GreenDuty Right */
        case 0x3808:
            if (value < 0 || value > 255) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3808 GreenDuty Right range: 0..255.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3808_digital_Inputs9[0] = (uint8_t)value;
            break;
        /* 0x3809:01 LatError Left */
        case 0x3809:
            if (value < -32768 || value > 32767) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO3809 LatError Left range: -32768..32767.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x3809_digital_Inputs10[0] = (int16_t)value;
            break;
        /* 0x380A:01 LatError Right */
        case 0x380A:
            if (value < -32768 || value > 32767) {
                CO_UNLOCK_OD(co->CANmodule);
                printf("PDO380A LatError Right range: -32768..32767.\n");
                fflush(stdout);
                return;
            }
            OD_RAM.x380A_digital_Inputs11[0] = (int16_t)value;
            break;
        default:
            CO_UNLOCK_OD(co->CANmodule);
            printf("Unsupported PDO index.\n");
            fflush(stdout);
            return;
    }
    CO_UNLOCK_OD(co->CANmodule);

    CO_TPDOsendRequest(&co->TPDO[index - 0x3800L]);
    printf("TPDO 0x%04lX:01 set to %ld\n", index, value);
    fflush(stdout);
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

    /* 0x3000:01 Speed, PLC -> Linux (RPDO mapped to 0x3000:01) */
    uint8_t plc_out;
    CO_LOCK_OD(co->CANmodule);
    plc_out = OD_RAM.x3000_digital_Outputs1[0];
    CO_UNLOCK_OD(co->CANmodule);

    static uint8_t last_out = 0;

    if (plc_out != last_out) {
        printf("RPDO 0x3000:01 changed: 0x%02X\n", plc_out);
        fflush(stdout);
        last_out = plc_out;
    }

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
