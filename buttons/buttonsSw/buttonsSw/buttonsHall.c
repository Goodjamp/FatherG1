#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"

#include "buttonsHall.h"

bool readButton(uint16_t buttonNumber, bool *rezRead)
{
    char c = getchar();
    printf("c = %c \n", c);
    *rezRead = (c == '1') ? (true) : (false);
    return true;
}

