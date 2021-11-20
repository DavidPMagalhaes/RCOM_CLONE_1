
#include "options.h"
#include "time.h" //For FER
#include <stdlib.h>
#include "dataProtocol.h"

struct PHYSICAL_OPTIONS CREATE_PHYSICAL_OPTIONS(time_t seed)
{
    struct PHYSICAL_OPTIONS options;
    options.OPTION_FER = 0;
    options.OPTION_FER_DATA = 0;
    options.OPTION_FER_HEAD = 0;
    options.OPTION_NO_ALARMS = 0;
    srand(seed);
    return options;
}

void
OPTIONS_GENERATE_FER(struct PHYSICAL_OPTIONS options, struct linkLayer *link)
{
    int r;
    // Errors for head
    if (options.OPTION_FER_HEAD != 0)
    {
        for (int i = 1; i <= 3; i++)
        {
            // Head is beteeen indices 1 and 3 inclusively
            r = rand() % options.OPTION_FER_HEAD;
            if (!r)
            {
                // 1 in a OPTIONS_FER_DATA chance of being a 0
                // We will totally switch the byte.
                link->frame.frame[i] = link->frame.frame[i] ^ 0xff;
            }
        }
    }

    // Errors for data
    if (options.OPTION_FER_DATA != 0)
    {
        for (int i = 4; i < link->frame.frameUsedSize - 1; i++)
        {
            // Creating errors in the information and bcc2 bytes. Flag is ignored hence -1
            // Keep in mind we are creating errors in the stuffed message. Simulating errors in transmissions
            r = rand() % options.OPTION_FER_DATA;
            if (!r)
            {
                // 1 in a OPTIONS_FER_DATA chance of being a 0
                // We will totally switch the byte.
                link->frame.frame[i] = link->frame.frame[i] ^ 0xff;
            }
        }
    }
}