#ifndef OPTIONS_H
#define OPTIONS_H

#include <time.h>
#include "dataProtocol.h"
#include "physicalProtocol.h"

struct linkLayer;

struct PHYSICAL_OPTIONS
{
    int OPTION_NO_ALARMS;
    int OPTION_FER;
    int OPTION_FER_HEAD;
    int OPTION_FER_DATA;
};

void CREATE_OPTIONS(int argc, char **argv, time_t seed);

int OPTION_IS_FLAG(char *arg);

void OPTIONS_GENERATE_FER(struct linkLayer *link, readInformationState *state, u_int8_t A, int *Nr);

int OPTIONS_ALARM();

#endif