#ifndef CS_MOS_LIBS_GP2Y10_INCLUDE_MGOS_GP2Y10_H_
#define CS_MOS_LIBS_GP2Y10_INCLUDE_MGOS_GP2Y10_H_

#include "mgos.h"
#include "mgos_adc.h"

struct mgos_GP2Y10;

struct mgos_GP2Y10* mgos_GP2Y10_create(int adcPin, int ledPin);
void mgos_GP2Y10_close(struct mgos_GP2Y10 * );
float mgos_GP2Y10_get_dust_density(struct mgos_GP2Y10 *);

#endif /* CS_MOS_LIBS_GP2Y10_INCLUDE_MGOS_GP2Y10_H_*/