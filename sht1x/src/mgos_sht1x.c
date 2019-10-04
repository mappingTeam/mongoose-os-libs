#include <stdint.h>
#include "mgos_sht1x.h"
#include "mgos_hal.h"
#include "mongoose.h"

#define MGOS_SHT_STARTUP_DELAY (11)
#define MGOS_SHT_CLOCK_HOLD_TIME_US (150)
#define MGOS_SHT_DATA_SETUP_TIME_US (200)
#define MGOS_SHT_DATA_HOLD_TIME_US (15)
#define MGOS_SHT_DATA_VALID_TIME_US (250)

#define HIGH (1)
#define LOW (0)

#ifndef IRAM
#define IRAM
#endif

enum BIT_ORDER
{
  MSBFIST = 0,
  LSBFIRST
};

struct mgos_sht
{
  int DATA;
  int SCK;
};

IRAM static int shiftIn(int _dataPin, int _clockPin, int _numBits)
{
  int ret = 0;
  int i;

  for (i = 0; i < _numBits; ++i)
  {
    mgos_gpio_write(_clockPin, HIGH);
    mgos_msleep(10);
    ret = ret * 2 + mgos_gpio_read(_dataPin);
    mgos_gpio_write(_clockPin, LOW);
  }

  return (ret);
}

IRAM static void shiftOut(uint8_t dataPin, uint8_t clockPin, enum BIT_ORDER bitOrder, uint8_t val)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    if (bitOrder == LSBFIRST)
      mgos_gpio_write(dataPin, !!(val & (1 << i)));
    else
      mgos_gpio_write(dataPin, !!(val & (1 << (7 - i))));

    mgos_gpio_write(clockPin, HIGH);
    mgos_gpio_write(clockPin, LOW);
  }
}

IRAM static void sendCommandSHT(int _command, int _dataPin, int _clockPin)
{
  int ack;

  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_set_mode(_clockPin, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(_dataPin, HIGH);
  mgos_gpio_write(_clockPin, HIGH);
  mgos_gpio_write(_dataPin, LOW);
  mgos_gpio_write(_clockPin, LOW);
  mgos_gpio_write(_clockPin, HIGH);
  mgos_gpio_write(_dataPin, HIGH);
  mgos_gpio_write(_clockPin, LOW);

  enum BIT_ORDER order = MSBFIST;
  shiftOut(_dataPin, _clockPin, order, _command);

  mgos_gpio_write(_clockPin, HIGH);
  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_INPUT);
  ack = mgos_gpio_read(_dataPin);
  if (ack != LOW)
  {
    LOG(LL_INFO, ("ACK error 0"));
  }
  mgos_gpio_write(_clockPin, LOW);
  ack = mgos_gpio_read(_dataPin);
  if (ack != HIGH)
  {
    LOG(LL_INFO, ("ACK error 1"));
  }
}

IRAM static void waitForResultSHT(int _dataPin)
{
  int i;
  int ack;

  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_INPUT);

  for (i = 0; i < 100; ++i)
  {
    mgos_msleep(10);
    ack = mgos_gpio_read(_dataPin);

    if (ack == LOW)
      break;
  }

  if (ack == HIGH)
  {
    LOG(LL_INFO, ("ACK error 2"));
  }
}

IRAM static int getData16SHT(int _dataPin, int _clockPin)
{
  int val;

  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_INPUT);
  mgos_gpio_set_mode(_clockPin, MGOS_GPIO_MODE_OUTPUT);
  val = shiftIn(_dataPin, _clockPin, 8);
  val *= 256;

  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(_dataPin, HIGH);
  mgos_gpio_write(_dataPin, LOW);
  mgos_gpio_write(_clockPin, HIGH);
  mgos_gpio_write(_clockPin, LOW);

  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_INPUT);
  val |= shiftIn(_dataPin, _clockPin, 8);

  return val;
}

IRAM static void skipCrcSHT(int _dataPin, int _clockPin)
{
  mgos_gpio_set_mode(_dataPin, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_set_mode(_clockPin, MGOS_GPIO_MODE_OUTPUT);

  mgos_gpio_write(_dataPin, HIGH);
  mgos_gpio_write(_clockPin, HIGH);
  mgos_gpio_write(_clockPin, LOW);
}

float mgos_sht1x_get_temperature_raw(int _dataPin, int _clockPin)
{
  int _val;

  int _gTempCmd = 0b00000011;

  sendCommandSHT(_gTempCmd, _dataPin, _clockPin);
  waitForResultSHT(_dataPin);
  _val = getData16SHT(_dataPin, _clockPin);
  skipCrcSHT(_dataPin, _clockPin);

  return (_val);
}

float mgos_sht1x_get_temperature_c(struct mgos_sht *sht)
{
  int _val;
  float _temperature;

  const float D1 = -40.0;
  const float D2 = 0.01;

  _val = mgos_sht1x_get_temperature_raw(sht->DATA, sht->SCK);

  _temperature = (_val * D2) + D1;

  return (_temperature);
}

float mgos_sht1x_get_temperature_f(struct mgos_sht *sht)
{
  int _val;
  float _temperature;

  const float D1 = -40.0;
  const float D2 = 0.018;

  _val = mgos_sht1x_get_temperature_raw(sht->DATA, sht->SCK);

  _temperature = (_val * D2) + D1;

  return (_temperature);
}

float mgos_sht1x_get_humidity(struct mgos_sht *sht)
{
  int _val;
  float _linearHumidity;
  float _correctedHumidity;
  float _temperature;

  const float C1 = -4.0;
  const float C2 = 0.0405;
  const float C3 = -0.0000028;
  const float T1 = 0.01;
  const float T2 = 0.00008;

  int _gHumidCmd = 0b00000101;

  sendCommandSHT(_gHumidCmd, sht->DATA, sht->SCK);
  waitForResultSHT(sht->DATA);
  _val = getData16SHT(sht->DATA, sht->SCK);
  skipCrcSHT(sht->DATA, sht->SCK);

  _linearHumidity = C1 + C2 * _val + C3 * _val * _val;

  _temperature = mgos_sht1x_get_temperature_c(sht);

  _correctedHumidity = (_temperature - 25.0) * (T1 + T2 * _val) + _linearHumidity;

  return (_correctedHumidity);
}

struct mgos_sht* mgos_sht1x_create(int dataPin, int clockPin)
{
  struct mgos_sht *sht = calloc(1, sizeof(*sht));
  if (sht == NULL)
    return NULL;
  memset(sht, 0, sizeof(struct mgos_sht));
  sht->DATA = dataPin;
  sht->SCK = clockPin;
  if (!mgos_gpio_set_mode(sht->DATA, MGOS_GPIO_MODE_INPUT) ||
      !mgos_gpio_set_pull(sht->DATA, MGOS_GPIO_PULL_UP) ||
      !mgos_gpio_set_mode(sht->SCK, MGOS_GPIO_MODE_OUTPUT))
  {
    mgos_sht1x_close(sht);
    return NULL;
  }
  return sht;
}

void mgos_sht1x_close(struct mgos_sht *sht)
{
  free(sht);
  sht = NULL;
}

bool mgos_sht1x_init(void)
{
  return true;
}