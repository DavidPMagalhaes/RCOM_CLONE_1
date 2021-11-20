#ifndef OPTIONS_H
#define OPTIONS_H

#include <time.h>

#include "dataProtocol.h"

struct linkLayer; 

struct PHYSICAL_OPTIONS
{
    int OPTION_NO_ALARMS;
    int OPTION_FER;
    int OPTION_FER_HEAD;
    int OPTION_FER_DATA;
};

struct PHYSICAL_OPTIONS CREATE_PHYSICAL_OPTIONS(time_t seed);

void PHYSICAL_PROTOCOL_OPTIONS(struct PHYSICAL_OPTIONS cmd_options);

void OPTIONS_GENERATE_FER(struct PHYSICAL_OPTIONS options, struct linkLayer *link);

#endif