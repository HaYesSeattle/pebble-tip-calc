#include <pebble.h>
#include <math.h>

#include "calculator.h"

#define MIN_BILL_DOLLARS 1
#define MIN_TIP_PERCENT 1
#define MIN_NUM_SPLITTING 1
#define MAX_BILL_DOLLARS 999
#define MAX_TIP_PERCENT 40
#define MAX_NUM_SPLITTING 9
#define DEFAULT_TIP_PERCENT 34
#define DEFAULT_NUM_SPLITTING 4
#define DEFAULT_BILL (CurrencyAmount){444,44}
#define PERSIST_VERSION 1
#define PERSIST_KEY_VERSION 100
#define PERSIST_KEY_BILL 200
#define PERSIST_KEY_TIP_PERCENT 300
#define PERSIST_KEY_NUM_SPLITTING 400

static CurrencyAmount bill;
static CurrencyAmount tip;
static CurrencyAmount total;
static CurrencyAmount total_per_person;
static int tip_percent;
static int num_splitting;


static int currency_amount_get_as_cents(CurrencyAmount amount) {
  return 100*amount.dollars + amount.cents;
}


static void currency_amount_set_from_cents(CurrencyAmount *amount, int total_cents) {
  amount->dollars = total_cents / 100;
  amount->cents = total_cents % 100;
}


static void update_totals(void) {
  int bill_c = currency_amount_get_as_cents(bill);

  int tip_c = ceil(((double)bill_c * (double)tip_percent) / 100.0);
  currency_amount_set_from_cents(&tip, tip_c);

  int total_c = bill_c + tip_c;
  currency_amount_set_from_cents(&total, total_c);

  int total_per_person_c = ceil((double)total_c / (double)num_splitting);
  currency_amount_set_from_cents(&total_per_person, total_per_person_c);
}


void calc_persist_store(void) {
  persist_write_int(PERSIST_KEY_VERSION, PERSIST_VERSION);
  persist_write_int(PERSIST_KEY_BILL, tip_percent);
  persist_write_int(PERSIST_KEY_BILL, num_splitting);
  persist_write_data(PERSIST_KEY_BILL, &bill, sizeof(bill));
}


void calc_persist_read(void) {
  if (persist_exists(PERSIST_KEY_BILL)) {
    persist_read_data(PERSIST_KEY_BILL, &bill, sizeof(bill));
    tip_percent = persist_read_int(PERSIST_KEY_TIP_PERCENT);
    num_splitting = persist_read_int(PERSIST_KEY_NUM_SPLITTING);
  } else {
    bill = DEFAULT_BILL;
    tip_percent = DEFAULT_TIP_PERCENT;
    num_splitting = DEFAULT_NUM_SPLITTING;
  }
}


// *** GetTxtCallback callbacks ***


char *calc_get_bill_dollars_txt(void) {
  static char s_buffer[4];
  snprintf(s_buffer, sizeof(s_buffer), "%3d", bill.dollars);
  return s_buffer;
}


char *calc_get_bill_cents_txt(void) {
  static char s_buffer[3];
  snprintf(s_buffer, sizeof(s_buffer), "%02d", bill.cents);
  return s_buffer;
}


char *calc_get_tip_percent_txt(void) {
  static char s_buffer[3];
  snprintf(s_buffer, sizeof(s_buffer), "%2d", tip_percent);
  return s_buffer;
}


char *calc_get_num_splitting_txt(void) {
  static char s_buffer[2];
  snprintf(s_buffer, sizeof(s_buffer), "%d", num_splitting);
  return s_buffer;
}


char *calc_total_txt(void) {
  static char s_buffer[9];
  snprintf(s_buffer, sizeof(s_buffer), "%4d.%2d/", total.dollars, total.cents);
  return s_buffer;
}


char *calc_total_per_person_txt(void) {
  static char s_buffer[9];
  snprintf(s_buffer, sizeof(s_buffer), "%4d.%2d/", total_per_person.dollars, total_per_person.cents);
  return s_buffer;
}


// *** IncDecCallback callbacks ***


void calc_inc_bill_dollars(void) {
  bill.dollars ++;
  if (bill.dollars > MAX_BILL_DOLLARS) {
    bill.dollars = MIN_BILL_DOLLARS;
  }
  update_totals();
}


void calc_dec_bill_dollars(void) {
  bill.dollars --;
  if (bill.dollars < MIN_BILL_DOLLARS) {
    bill.dollars = MAX_BILL_DOLLARS;
  }
  update_totals();
}


void calc_inc_bill_cents(void) {
  bill.cents ++;
  if (bill.cents > 99) {
    bill.cents = 0;
  }
  update_totals();
}


void calc_dec_bill_cents(void) {
  bill.cents --;
  if (bill.cents < 0) {
    bill.cents = 99;
  }
  update_totals();
}


void calc_inc_tip_percent(void) {
  tip_percent ++;
  if (tip_percent > MAX_TIP_PERCENT) {
    tip_percent = MIN_TIP_PERCENT;
  }
  update_totals();
}


void calc_dec_tip_percent(void) {
  tip_percent --;
  if (tip_percent < MIN_TIP_PERCENT) {
    tip_percent = MAX_TIP_PERCENT;
  }
  update_totals();
}


void calc_inc_num_splitting(void) {
  num_splitting ++;
  if (num_splitting > MAX_NUM_SPLITTING) {
    num_splitting = MIN_NUM_SPLITTING;
  }
  update_totals();
}


void calc_dec_num_splitting(void) {
  num_splitting --;
  if (num_splitting < MIN_NUM_SPLITTING) {
    num_splitting = MAX_NUM_SPLITTING;
  }
  update_totals();
}
