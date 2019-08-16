#include "stdint.h"
#include "stddef.h"
#include "string.h"

#include "generalProtocol.h"


#define BUFF_SIZE 64
#define CHANNEL_CNT 0x7
#define CHANNEL_ALL 0xFF

typedef enum{
    GP_ADC                    = 0,
    GP_STOP                   = 1,
    GP_START_CLOCK_WISE       = 2,
    GP_START_CONTR_CLOCK_WISE = 3,
} GP_COMMAND;


#pragma pack(push, 1)
#define PROTOCOL_BUFF_SIZE (BUFF_SIZE - 4)
typedef struct GpCommand{
    uint8_t headr;
    uint8_t subcommand[PROTOCOL_BUFF_SIZE];
} GpCommand;

typedef struct {
    uint16_t size;
    uint16_t data[];
}GpADCSubcommand;

typedef union {
    GpCommand command;
    uint8_t buff[sizeof(GpCommand)];
} GpCommandBuff ;
#pragma pack(pop)

const GpInitCb *gpCbList = NULL;

void gpInit(const GpInitCb *gpCbIn){
    gpCbList = gpCbIn;
}

void gpDecode(uint8_t buff[],  uint32_t size)
{
    if(size < BUFF_SIZE);
    GpCommand *gpCommand = (GpCommand *)buff;
    switch(gpCommand->headr) {
        case GP_STOP:
            if (gpCbList->gpStopCommandCb != NULL) {
                 gpCbList->gpStopCommandCb(gpCommand->subcommand[0]);
            }
            break;
        case GP_START_CLOCK_WISE:
            if (gpCbList->gpStartClockWiseCommandCb != NULL) {
                 gpCbList->gpStartClockWiseCommandCb(gpCommand->subcommand[0]);
            }
            break;
        case GP_START_CONTR_CLOCK_WISE:
            if (gpCbList->gpStartContrClockWiseCommandCb != NULL) {
                 gpCbList->gpStartContrClockWiseCommandCb(gpCommand->subcommand[0]);
            }
            break;
        default: break;
    }
}

bool gpSendADC(uint16_t buff[], uint16_t size)
{
    GpCommandBuff command = {
        .command.headr = GP_ADC
    };
    GpADCSubcommand *gpADCSubcommand = (GpADCSubcommand *)command.command.subcommand;
    gpADCSubcommand->size = size;
    memcpy((uint8_t*)gpADCSubcommand->data, (uint8_t*)buff, size * 2);
    gpCbList->gpSendCb(command.buff, sizeof(command));
    return true;
}
