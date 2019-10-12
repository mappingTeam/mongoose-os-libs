#include "mgos.h"
#include "mgos_i2c.h"
#include "bh1750.h"

#ifdef __cplusplus
extern "C"
{
#endif

const float BH1750_CONV_FACTOR = 1.2;

struct mgos_bh1750
{
  struct mgos_i2c * i2c;
  uint8_t i2caddr;
  uint8_t mode;
  uint8_t MTreg;
  float light_intensity;
};

/**
 * Private functions
 */

static bool mgos_bh1750_cmd(struct mgos_bh1750 * sensor, uint8_t cmd) {
  if (!sensor || !sensor->i2c)
    return false;

  uint8_t data[1];
  data[0] = cmd;

  if (!mgos_i2c_write(sensor->i2c, sensor->i2caddr, data, 1, true)) {
    LOG(LL_ERROR, ("[BH1750] ERROR: I2C write error"));
    return false;
  }
  LOG(LL_INFO, ("[BH1750] INFO: Write success"));

  return true;
}

static bool mgos_bh1750_configure(struct mgos_bh1750 * sensor, uint8_t mode) {
  switch (mode) {
    case CONTINUOUS_HIGH_RES_MODE:
    case CONTINUOUS_HIGH_RES_MODE_2:
    case CONTINUOUS_LOW_RES_MODE:
    case ONE_TIME_HIGH_RES_MODE:
    case ONE_TIME_HIGH_RES_MODE_2:
    case ONE_TIME_LOW_RES_MODE:

    if (!mgos_bh1750_cmd(sensor, mode)) {
      LOG(LL_ERROR, ("[BH1750] ERROR: Cannot set mode"));
      return false;
    }
    sensor->mode = mode;
    mgos_msleep(10);
    return true;

    default:
    LOG(LL_ERROR, ("[BH1750] ERROR: Invalid mode"));
    return false;
  }
}

/**
 * Public functions
 */
bool mgos_bh1750_set_MtReg(struct mgos_bh1750 *sensor, uint8_t MTreg) {
  if (MTreg <= 31 || MTreg > 254) {
    LOG(LL_ERROR, ("[BH1750] ERROR: MTreg out of range"));
    return false;
  }
  if (!mgos_bh1750_cmd(sensor, (0b01000 << 3) | (MTreg >> 5)) |
      !mgos_bh1750_cmd(sensor, (0b011 << 5 )  | (MTreg & 0b11111)) |
      !mgos_bh1750_cmd(sensor, sensor->mode))
  {
    LOG(LL_ERROR, ("[BH1750] Error: Cannot set MTreg"));
    return false;
  }
  sensor->MTreg = MTreg;
  return true;
}

struct mgos_bh1750 * mgos_bh1750_create(struct mgos_i2c * i2c, uint8_t i2caddr, uint8_t mode) {
  struct mgos_bh1750 * sensor;

  if (!i2c)
    return NULL;

  sensor = calloc(1, sizeof(struct mgos_bh1750));
  if (!sensor) {
    LOG(LL_ERROR, ("[BH1750] ERROR: Cannot allocate memory"));
    return NULL;
  }

  memset(sensor, 0, sizeof(struct mgos_bh1750));
  sensor->i2caddr = i2caddr;
  sensor->i2c = i2c;
  sensor->mode = UNCONFIGURED;
  sensor->MTreg = BH1750_DEFAULT_MTREG;

  if (!mgos_bh1750_configure(sensor, mode)) {
    LOG(LL_ERROR, ("[BH1750] Error: Cannot init BH1750"));
    free(sensor);
    sensor = NULL;
    return NULL;
  }
  return sensor;
}

void mgos_bh1750_destroy(struct mgos_bh1750 * sensor) {
  free(sensor);
  sensor = NULL;
}

float mgos_bh1750_read_light_level(struct mgos_bh1750 * sensor, bool maxWait) {
  if (sensor->mode == UNCONFIGURED) {
    LOG(LL_ERROR, ("[BH1750] ERROR: Device is not configured, mode = 0"));
    return -2;
  }
  if (!mgos_bh1750_cmd(sensor, sensor->mode)) {
    LOG(LL_ERROR, ("[BH1750] ERROR: Cannot send reading mode"));
    return false;
  }
  switch(sensor->mode) {
    case ONE_TIME_LOW_RES_MODE:
    maxWait ? mgos_msleep(24 * sensor->MTreg/(uint8_t)BH1750_DEFAULT_MTREG) : mgos_msleep(16 * sensor->MTreg/(uint8_t)BH1750_DEFAULT_MTREG);
    break;

    case ONE_TIME_HIGH_RES_MODE:
    case ONE_TIME_HIGH_RES_MODE_2:
    maxWait ? mgos_msleep(180 * sensor->MTreg/(uint8_t)BH1750_DEFAULT_MTREG) : mgos_msleep(120 * sensor->MTreg/(uint8_t)BH1750_DEFAULT_MTREG);
  }
  uint8_t returnedData[2];
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, returnedData, 2, true)) {
    LOG(LL_ERROR, ("[BH1750] ERROR: Cannot read light level"));
    return -1;
  }
  float level = returnedData[0] * 256 + returnedData[1];
  if (sensor->MTreg != BH1750_DEFAULT_MTREG)
    level *= (float)((uint8_t) BH1750_DEFAULT_MTREG / (float)sensor->MTreg);
  if (sensor->mode == ONE_TIME_HIGH_RES_MODE_2 ||
      sensor->mode == CONTINUOUS_HIGH_RES_MODE_2)
    level /= 2;

  /* Convert raw value to lx */
  level /= BH1750_CONV_FACTOR;
  sensor->light_intensity = level;
  return level;
}

bool mgos_bh1750_init(void) {
  return true;
}

#ifdef __cplusplus
}
#endif