#include "CO_application.h"
#include "OD.h"
#include "301/CO_PDO.h"

#include <stdio.h>

CO_ReturnError_t app_programStart(uint16_t* bitRate, uint8_t* nodeId, uint32_t* errInfo) {
    (void)bitRate;
    (void)nodeId;
    (void)errInfo;
    return CO_ERROR_NO;
}

void app_communicationReset(CO_t* co) {
    (void)co;
}

void app_programEnd() {}


// USER APPLICATION
void app_programAsync(CO_t* co, uint32_t timer1usDiff) {
    (void)timer1usDiff;

    /* PLC -> Linux (RPDO mapped to 0x3000:01) */
    uint8_t plc_out;
    CO_LOCK_OD(co->CANmodule);
    plc_out = OD_RAM.x3000_digital_Outputs1[0];
    CO_UNLOCK_OD(co->CANmodule);

    /* Linux -> PLC (TPDO mapped to 0x3800:01) */
    static uint8_t last_in = 0;
    static uint8_t last_out = 0;
    if (plc_out != last_out) {
        printf("RPDO 0x3000:01 changed: 0x%02X\n", plc_out);
        fflush(stdout);
        last_out = plc_out;
    }
    if (plc_out != last_in) {
        CO_LOCK_OD(co->CANmodule);
        OD_RAM.x3800_digital_Inputs1[0] = plc_out;
        CO_UNLOCK_OD(co->CANmodule);

        /* Request TPDO send */
        CO_TPDOsendRequest(&co->TPDO[0]);
        last_in = plc_out;
    }
}

void app_programRt(CO_t* co, uint32_t timer1usDiff) {
    (void)co;
    (void)timer1usDiff;
}
