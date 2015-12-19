#pragma once

#include <pebble.h>


typedef struct {
    int dollars;
    int cents;
} CurrencyAmount;

typedef char *(GetTxtCallback)(void);
typedef void (IncDecCallback)(void);

void calc_inc_bill_dollars(void);
void calc_dec_bill_dollars(void);
void calc_inc_bill_cents(void);
void calc_dec_bill_cents(void);
void calc_inc_tip_percent(void);
void calc_dec_tip_percent(void);
void calc_inc_num_splitting(void);
void calc_dec_num_splitting(void);

char *calc_get_bill_dollars_txt(void);
char *calc_get_bill_cents_txt(void);
char *calc_get_tip_percent_txt(void);
char *calc_get_num_splitting_txt(void);
char *calc_get_tip_txt(void);
char *calc_get_total_txt(void);
char *calc_get_total_per_person_txt(void);

//! Save the calculator to persistent storage.
void calc_persist_store(void);

//! Read the calculator from persistent storage.
void calc_persist_read(void);