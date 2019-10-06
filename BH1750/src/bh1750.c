#include "mgos.h"
#include "mgos_i2c.h"
#include "bh1750.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mgos_bh1750 {
  struct mgos_i2c * i2c;
  uint8_t i2caddr;
  uint8_t mode;
  uint8_t mtreg;
  float light_intensity_lx;
};

const float BH1750_CONV_FACTOR = 1.2;

static bool mgos_bh1750_cmd(struct mgos_bh1750 * sensor, uint16_t command) {
  uint8_t data[2];
  if (!sensor || !sensor->i2c)
    return false;

  data[0] = command >> 8;
  data[1] = command & 0xFF;
  if (!mgos_i2c_write(sensor->i2c, sensor->i2caddr, data, 2, true)) {
    LOG(LL_ERROR, ("[BH1750] Error: I2C Write error."));
    return false;
  }
  LOG(LL_INFO, ("[BH1750] INFO: I2C Write success."));
  return true;
}

bool mgos_bh1750_read_light_level(struct mgos_bh1750 * sensor, bool maxWait) {
  float intensity = -1;

  if (sensor->mode = UNCONFIGURED) {
    LOG(LL_ERROR, ("[BH1750] Error: Invalid mode."));
    return false;
  }

  mgos_bh1750_cmd(sensor, BH1750_POWER_ON);
  mgos_bh1750_cmd(sensor, sensor->mode);
  /**
   * Wait for measurement to be taken
   */
  switch (sensor->mode)
  {
  case ONE_TIME_LOW_RES_MODE:
    maxWait ? mgos_msleep(24 * sensor->mtreg/(uint8_t)BH1750_DEFAULT_MTREG) : mgos_msleep(16 * sensor->mtreg/(uint8_t)BH1750_DEFAULT_MTREG);
    break;

  case ONE_TIME_HIGH_RES_MODE:
  case ONE_TIME_HIGH_RES_MODE_2:
    maxWait ? mgos_msleep(180 * sensor->mtreg/(uint8_t)BH1750_DEFAULT_MTREG) : mgos_msleep(120 * sensor->mtreg/(uint8_t)BH1750_DEFAULT_MTREG);

  default:
    break;
  }

  int8_t data[2];
  unsigned int tmp = 0;
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, data, 2, true)) {
    LOG(LL_ERROR, ("Can't read sensor."));
    return false;
  }
  tmp = data[0] << 8;
  tmp |= data[1] & 0xFF;
  intensity = tmp;

  if (sensor->mtreg != BH1750_DEFAULT_MTREG) 
    intensity *= (float)((uint8_t)BH1750_DEFAULT_MTREG / (uint8_t) sensor->mtreg);
  if (sensor->mode == ONE_TIME_HIGH_RES_MODE_2 || sensor->mode == CONTINUOUS_HIGH_RES_MODE_2)
    intensity /= 2;

  intensity /= BH1750_CONV_FACTOR;
  sensor->light_intensity_lx = intensity;
  return true;
}

struct mgos_bh1750 * mgos_bh1750_create(struct mgos_i2c * i2c, uint8_t mode) {
  struct mgos_bh1750 * sensor;
  if (!i2c)
    return NULL;
  
  sensor = calloc(1, sizeof(struct mgos_bh1750));
  if (!sensor)
    return NULL;

  memset(sensor, 0, sizeof(struct mgos_bh1750));
  sensor->i2c = i2c;
  sensor->i2caddr = BH1750_DEFAULT_I2CADDR;

  sensor->mode = UNCONFIGURED;
  switch (mode)
  {
  case CONTINUOUS_HIGH_RES_MODE:
  case CONTINUOUS_HIGH_RES_MODE_2:
  case CONTINUOUS_LOW_RES_MODE:
  case ONE_TIME_HIGH_RES_MODE:
  case ONE_TIME_HIGH_RES_MODE_2:
  case ONE_TIME_LOW_RES_MODE:
  sensor->mode = mode;
  break;

  default:
    LOG(LL_ERROR, ("[BH1750] Error: Invalid mode."));
    free(sensor);
    sensor = NULL;
    return NULL;
  }
}

#ifdef __cplusplus
}
#endif