#include "mgos_GP2Y10.h"
#include <stdint.h>
#include "mgos_hal.h"
#include "mongoose.h"

/**
 * LED pulse cycle = 10 ms
 * LED pulse width = 0.32 ms
 */
#define DELAY_BEFORE_SAMPLING (0.28)
#define DELAY_AFTER_SAMPLING (0.04)
#define SLEEP_TIME (9.68)

#define HIGH (1)
#define LOW (0)
#define VREF (3.3)

struct mgos_GP2Y10
{
  int ADC_PIN;
  int LED_PIN;
};

enum ActiveMode {
  ACTIVE_HIGH = 0,
  ACTIVE_LOW = 1
};
uint8_t active_mode = ACTIVE_HIGH;

#ifdef __cplusplus
extern "C"
{
#endif

float mgos_GP2Y10_get_dust_density(struct mgos_GP2Y10 *gp2y10)
{
  mgos_gpio_write(gp2y10->LED_PIN, HIGH^active_mode);
  mgos_usleep(DELAY_BEFORE_SAMPLING * 1000);
  int adc_value = mgos_adc_read(gp2y10->ADC_PIN);
  mgos_usleep(DELAY_AFTER_SAMPLING * 1000);
  mgos_gpio_write(gp2y10->LED_PIN, LOW^active_mode);
  mgos_usleep(SLEEP_TIME * 1000);
  float voltage = (float)adc_value * ((float)VREF / 1024.0);
  float dust_density = 0.17 * voltage - 0.1;
  return dust_density;
}

struct mgos_GP2Y10 * mgos_GP2Y10_create(int adcPin, int ledPin)
{
  struct mgos_GP2Y10 *gp2y10 = calloc(1, sizeof(*gp2y10));
  if (!gp2y10) {
    LOG(LL_ERROR, ("[GP2Y10] Unable to allocate memory"));
    return NULL;
  }
  memset(gp2y10, 0, sizeof(struct mgos_GP2Y10));
  gp2y10->ADC_PIN = adcPin;
  gp2y10->LED_PIN = ledPin;

  if (!mgos_gpio_set_mode(gp2y10->ADC_PIN, MGOS_GPIO_MODE_INPUT) ||
      !mgos_gpio_set_mode(gp2y10->LED_PIN, MGOS_GPIO_MODE_OUTPUT))
  {
    mgos_GP2Y10_close(gp2y10);
    LOG(LL_ERROR, ("[GP2Y10] Set pin mode failed"));
    return NULL;
  }
  if (!mgos_adc_enable(gp2y10->ADC_PIN)) {
    mgos_GP2Y10_close(gp2y10);
    LOG(LL_ERROR, ("[GP2Y10] Unable to enable ADC"));
    return NULL;
  }
  return gp2y10;
}

void mgos_GP2Y10_close(struct mgos_GP2Y10 *gp2y10)
{
  free(gp2y10);
  gp2y10 = NULL;
}

bool mgos_GP2Y10_init(void)
{
  return true;
}

#ifdef __cplusplus
}
#endif