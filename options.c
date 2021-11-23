#include "options.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h> //For FER
#include <errno.h>
#include <unistd.h>
#include "dataProtocol.h"     // for linkLayer
#include "physicalProtocol.h" // for readInformationState

struct PHYSICAL_OPTIONS OPTIONS;

void CREATE_OPTIONS(int argc, char **argv, time_t seed)
{
    errno = 0;
    int flags_started = 0;
    for (int i = 2; i < argc; i++)
    {
        if (!strcmp(argv[i], "-noAlarms"))
        {
            OPTIONS.OPTION_NO_ALARMS = 1;
            flags_started = 1;
            printf("-noAlarms enabled\n");
        }
        else if (!strcmp(argv[i], "-FER"))
        {
            if (i + 2 >= argc)
            {
                printf("Wrong -FER syntax\n");
                exit(1);
            }
            OPTIONS.OPTION_FER = 1;
            OPTIONS.OPTION_FER_HEAD = (int)strtol(argv[i + 1], NULL, 10);
            OPTIONS.OPTION_FER_DATA = (int)strtol(argv[i + 2], NULL, 10);
            if (errno != 0)
            {
                printf("Passed a wrong value for -FER argument\n");
                exit(1);
            }
            i += 2;
            flags_started = 1;
            printf("-FER enabled with head odds %d and data odds %d\n", OPTIONS.OPTION_FER_HEAD, OPTIONS.OPTION_FER_DATA);
        }
        else if (!strcmp(argv[i], "-TPROP"))
        {
            if (i + 1 >= argc)
            {
                printf("Wrong -TPROP syntax\n");
                exit(1);
            }
            OPTIONS.OPTION_TPROP = 1;
            OPTIONS.OPTION_TPROP_MS = (int)strtol(argv[i + 1], NULL, 10);
            if (errno != 0)
            {
                printf("Passed a wrong value for -TPROP argument\n");
                exit(1);
            }
            i += 1;
            flags_started = 1;
            printf("-TPROP enabled with %d ms", OPTIONS.OPTION_TPROP_MS);
        }
        else
        {
            if (flags_started)
            {
                // We have a non flag after a flag
                printf("Call with arguments <port> <filename> [...options]  where <port> fulfills <condition>\n");
                exit(1);
            }
        }
    }
}

int OPTION_IS_FLAG(char *arg)
{
    if (!strcmp(arg, "-noAlarms"))
    {
        return 1;
    }
    else if (!strcmp(arg, "-FER"))
    {
        return 2;
    }
    else if (!strcmp(arg, "-TPROP"))
    {
        return 3;
    }
    return 0;
}

void OPTIONS_GENERATE_FER(struct linkLayer *link, readInformationState *state, u_int8_t A, int *Nr)
{
    int r;
    u_int8_t byte;

    if (!OPTIONS.OPTION_FER)
    {
        // Fer is disabled
        return;
    }
    if (*state != RI_INFORMATION_STOP)
    {
        // Fer is only used after receival of a valid information package
        return;
    }

    // Valid information package received. Altering it and returning a new state to simulate error
    // Errors for head
    if (OPTIONS.OPTION_FER_HEAD != 0)
    {
        for (int i = 1; i <= 3; i++)
        {
            // Head is beteeen indices 1 and 3 inclusively
            r = rand() % OPTIONS.OPTION_FER_HEAD;
            if (!r)
            {
                // 1 in a OPTIONS_FER_DATA chance of being a 0
                // We will totally switch the byte.
                link->frame.frame[i] = link->frame.frame[i] ^ 0xff;
            }
        }
    }

    // Errors for data
    if (OPTIONS.OPTION_FER_DATA != 0)
    {
        for (int i = 4; i < link->frame.frameUsedSize - 1; i++)
        {
            // Creating errors in the information and bcc2 bytes. Flag is ignored hence -1
            // Keep in mind we are creating errors in the stuffed message. Simulating errors in transmissions
            r = rand() % OPTIONS.OPTION_FER_DATA;
            if (!r)
            {
                // 1 in a OPTIONS_FER_DATA chance of being a 0
                // We will totally switch the byte.
                link->frame.frame[i] = rand() % 256;
            }
        }
    }

    // Generating new state
    (*state) = RI_START;
    for (int i = 0; i < link->frame.frameUsedSize; i++)
    {
        byte = link->frame.frame[i];
        (*state) = readInformationStateMachine(*state, A, byte, Nr);
        if (*state == RI_INFORMATION_STOP)
        {
            return;
        }
        if (*state == RI_RESET)
        {
            // Without this, stuff breaks. Better to have this
            // It will still time out
            return;
        }
    }
}

int OPTIONS_ALARM()
{
    return (!OPTIONS.OPTION_NO_ALARMS);
}

void OPTIONS_TPROP()
{
    if (OPTIONS.OPTION_TPROP)
    {
        usleep(OPTIONS.OPTION_TPROP_MS * 1000);
    }
}