#pragma once

#include <pebble.h>


typedef struct {
    int dollars;
    int cents;
} CurrencyAmount;

typedef char *(GetTxtCallback)(void);
typedef void (IncDecCallback)(void);

//! Save the calculator to persistent storage.
void calc_persist_store(void);

//! Read the calculator from persistent storage.
void calc_persist_read(void);