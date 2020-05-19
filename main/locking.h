#pragma once

#ifndef _WINSTON_LOCKING_H_
#define _WINSTON_LOCKING_H_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Helps lock access to crytical parts of the system.
class Locking {
 public:
  Locking();
  // Lock access to the I2C bus.
  bool lockI2C(const char* who);
  // Unlock access to the I2C bus.
  bool unlockI2C(const char* who);
 private:
  SemaphoreHandle_t i2c_handle_;
};

#endif /* _WINSTON_LOCKING_H_ */
