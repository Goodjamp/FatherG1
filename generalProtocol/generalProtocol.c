#include "stdint.h"
#include "stddef.h"
#include "string.h"

#include "generalProtocol.h"


#define BUFF_SIZE 64
#define CHANNEL_ALL 0xFF
#define SET_DATA_8_BIT(X)  X = (X & (uint8_t)(0xFF << 2)) | (uint8_t)(COMMAND_DATA_FLAG_8_BIT_SIZE);
#define SET_DATA_16_BIT(X) X = (X & (uint8_t)(0xFF << 2)) | (uint8_t)(COMMAND_DATA_FLAG_16_BIT_SIZE);
#define SET_DATA_32_BIT(X) X = (X & (uint8_t)(0xFF << 2)) | (uint8_t)(COMMAND_DATA_FLAG_32_BIT_SIZE);


typedef enum{
    GP_DATA                   = 0,
    GP_STOP                   = 1,
    GP_START_CLOCK_WISE       = 2,
    GP_START_CONTR_CLOCK_WISE = 3,
    GP_START_AUTO_SWITCHER    = 4,
} GP_COMMAND;

#pragma pack(push, 1)
#define PROTOCOL_BUFF_SIZE (BUFF_SIZE - 1)
typedef struct {
    uint8_t  channel;
    uint8_t  flags;
    uint16_t cnt;
    uint8_t  data[];
}GpDataSubcommand;

typedef struct {
    uint8_t  channel;
}GpStartClockWiseSubcommand;

typedef struct {
    uint8_t  channel;
}GpStartContrClockWiseSubcommand;

typedef struct {
    uint8_t  channel;
}GpStoptSubcommand;

typedef struct {
    uint8_t  channel;
    uint16_t offTime;
    uint16_t onTime;
    uint32_t cnt;
}GpStartAutoSwitcher;

typedef struct GpCommand{
    uint8_t headr;
    union {
        GpDataSubcommand                dataSubComand;
        GpStartClockWiseSubcommand      startClockWiseSubcommand;
        GpStartContrClockWiseSubcommand startContrClockWiseSubcommand;
        GpStoptSubcommand               stoptSubcommand;
        GpStartAutoSwitcher             startAutoSwitcherSubcommand;
        uint8_t          buffSubComand[PROTOCOL_BUFF_SIZE];
    };
} GpCommand;

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
                 gpCbList->gpStopCommandCb(gpCommand->stoptSubcommand.channel);
            }
            break;
        case GP_START_CLOCK_WISE:
            if (gpCbList->gpStartClockWiseCommandCb != NULL) {
                 gpCbList->gpStartClockWiseCommandCb(gpCommand->startClockWiseSubcommand.channel);
            }
            break;
        case GP_START_CONTR_CLOCK_WISE:
            if (gpCbList->gpStartContrClockWiseCommandCb != NULL) {
                 gpCbList->gpStartContrClockWiseCommandCb(gpCommand->startContrClockWiseSubcommand.channel);
            }
            break;
        case GP_START_AUTO_SWITCHER:
            if (gpCbList->gpStartAutoSwitcherCommandCb != NULL) {
                 gpCbList->gpStartAutoSwitcherCommandCb(gpCommand->startAutoSwitcherSubcommand.channel,
                                                        gpCommand->startAutoSwitcherSubcommand.offTime,
                                                        gpCommand->startAutoSwitcherSubcommand.onTime,
                                                        gpCommand->startAutoSwitcherSubcommand.cnt);
            }
            break;
        default: break;
    }
}

bool gpSendDataCommand(uint8_t data[],  uint16_t cnt, COMMAND_DATA_FLAG flags, uint8_t channel)
{
    GpCommandBuff dataCommand = {
        .command.headr = GP_DATA
    };
    dataCommand.command.dataSubComand.channel = channel;
    dataCommand.command.dataSubComand.cnt = cnt;
    switch(flags) {
    case COMMAND_DATA_FLAG_8_BIT_SIZE:
        SET_DATA_8_BIT(dataCommand.command.dataSubComand.flags);
        break;
    case COMMAND_DATA_FLAG_16_BIT_SIZE:
        SET_DATA_16_BIT(dataCommand.command.dataSubComand.flags);
        break;
    case COMMAND_DATA_FLAG_32_BIT_SIZE:
        SET_DATA_32_BIT(dataCommand.command.dataSubComand.flags);
        break;
    default:
        return false;
    }
    memcpy((uint8_t*)dataCommand.command.dataSubComand.data, (uint8_t*)data, cnt);
    gpCbList->gpSendCb(dataCommand.buff, sizeof(dataCommand));
    return true;
}
