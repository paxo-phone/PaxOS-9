#ifndef CLOCK_HPP
#define CLOCK_HPP

#ifdef ESP32
    #include <Arduino.h>
    #include "soc/rtc_wdt.h"
    #include "esp_heap_caps.h"
    #include <esp_task_wdt.h>
#endif


#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

#include <stdint.h>
#include <cstdint>

/**
  * @brief Retourne le nombre de microsecondes depuis une date fixée
  * @return Entier représentant le nombre de microsecondes depuis une date fixée
  * @remark Cette fonction est déjà présente sur l'esp32
  */
uint64_t os_millis(void);

/**
  * @brief Retourne le nombre de millisecondes depuis une date fixée
  * @return Entier représentant le nombre de microsecondes depuis une date fixée
  * @remark Cette fonction est déjà présente sur l'esp32
  */
uint64_t os_millis(void);

#else /* #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) */

extern unsigned long sleepStartTimeMillis;
extern long long sleepTimeCorrectionOffset = 0;

unsigned long os_millis();

#endif

#endif /* TIME_HPP */