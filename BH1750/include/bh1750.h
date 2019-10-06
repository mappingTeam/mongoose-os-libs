#include "mgos.h"
#include "mgos_i2c.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BH1750_DEFAULT_I2CADDR (0x23)
#define BH1750_POWER_DOWN (0x00)
#define BH1750_POWER_ON (0x01)
#define BH1750_RESET (0x07)
#define BH1750_DEFAULT_MTREG (69)

#define UNCONFIGURED (0)
// Measurement at 1 lux resolution. Measurement time is approx 120ms.
#define CONTINUOUS_HIGH_RES_MODE (0x10)
// Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
#define CONTINUOUS_HIGH_RES_MODE_2 (0x11)
// Measurement at 4 lux resolution. Measurement time is approx 16ms.
#define CONTINUOUS_LOW_RES_MODE (0x13)
// Measurement at 1 lux resolution. Measurement time is approx 120ms.
#define ONE_TIME_HIGH_RES_MODE (0x20)
// Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
#define ONE_TIME_HIGH_RES_MODE_2 (0x21)
// Measurement at 4 lux resolution. Measurement time is approx 16ms.
#define ONE_TIME_LOW_RES_MODE (0x23)

struct mgos_bh1750;
struct mgos_bh1750 * mgos_bh1750_create(struct mgos_i2c *i2c, uint8_t i2caddr);
void mgos_bh1750_destroy(struct mgos_bh1750 *sensor);
bool mgos_bh1750_get_light_instensity(struct mgos_bh1750 *sensor);
bool mgos_bh1750_init(void);dd

#ifdef __cplusplus
}
#endif