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
    int OPTION_TPROP;
    int OPTION_TPROP_MS;
    int OPTIONS_PACKET_LOSS;
    int OPTIONS_PACKET_LOSS_ODD;
};

void CREATE_OPTIONS(int argc, char **argv, time_t seed);

int OPTION_IS_FLAG(char *arg);

void OPTIONS_GENERATE_FER(struct linkLayer *link, readInformationState *state, u_int8_t A, int *Nr);

int OPTIONS_ALARM();

void OPTIONS_TPROP();

int OPTIONS_PACKET_LOSS();

#endif