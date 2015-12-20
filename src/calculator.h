#pragma once

#include <pebble.h>


typedef struct {
    int dollars;
    int cents;
} CurrencyAmount;

typedef char *(GetTxtCallback)(void);
typedef void (CalcManipCallback)(int);

void calc_manip_bill_dollars(int);
void calc_manip_bill_cents(int);
void calc_manip_tip_percent(int);
void calc_manip_num_splitting(int);
void calc_update_totals(void);

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