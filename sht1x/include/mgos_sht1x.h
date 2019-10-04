#ifndef CS_MOS_LIBS_SHT_INCLUDE_MGOS_SHT_H_
#define CS_MOS_LIBS_SHT_INCLUDE_MGOS_SHT_H_

#include "mgos.h"
#include <math.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mgos_sht;

struct mgos_sht * mgos_sht1x_create(int dataPin, int clockPin);
void mgos_sht1x_close(struct mgos_sht *);
float mgos_sht1x_get_temperature_c(struct mgos_sht *);
float mgos_sht1x_get_temperature_f(struct mgos_sht *);
float mgos_sht1x_get_humidity(struct mgos_sht *);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CS_MOS_LIBS_SHT_INCLUDE_MGOS_SHT_H_ */