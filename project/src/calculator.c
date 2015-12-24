#include <pebble.h>

#include "calculator.h"

#define MIN_BILL_DOLLARS 1
#define MIN_TIP_PERCENT 1
#define MIN_NUM_SPLITTING 1
#define MAX_BILL_DOLLARS 999
#define MAX_TIP_PERCENT 40
#define MAX_NUM_SPLITTING 9
#define DEFAULT_TIP_PERCENT 15
#define DEFAULT_NUM_SPLITTING 1
#define DEFAULT_BILL (CurrencyAmount){10,00}
#define PERSIST_VERSION 1
#define PERSIST_KEY_VERSION 100
#define PERSIST_KEY_BILL 200
#define PERSIST_KEY_TIP_PERCENT 300

static CurrencyAmount bill;
static CurrencyAmount tip;
static CurrencyAmount total;
static CurrencyAmount total_per_person;
static int tip_percent;
static int num_splitting;


// Round up if remainder >= 0.5; only use with UNSIGNED integers
static int divide_and_round(int dividend, int divisor) {
  return (dividend + (divisor / 2)) / divisor;
}


static int currency_amount_get_in_cents(CurrencyAmount amount) {
  return 100*amount.dollars + amount.cents;
}


static void currency_amount_set_from_cents(CurrencyAmount *amount, int total_cents) {
  amount->dollars = total_cents / 100;
  amount->cents = total_cents % 100;
}


void calc_update_totals(void) {
  int bill_in_cents = currency_amount_get_in_cents(bill);

  int tip_in_cents = divide_and_round(bill_in_cents * tip_percent, 100);
  currency_amount_set_from_cents(&tip, tip_in_cents);

  int total_in_cents = bill_in_cents + tip_in_cents;
  currency_amount_set_from_cents(&total, total_in_cents);

  int total_per_person_in_cents = divide_and_round(total_in_cents, num_splitting);
  currency_amount_set_from_cents(&total_per_person, total_per_person_in_cents);
}


void calc_reset_to_defaults(void) {
  bill = DEFAULT_BILL;
  tip_percent = DEFAULT_TIP_PERCENT;
  num_splitting = DEFAULT_NUM_SPLITTING;
  calc_update_totals();
}


// ************************************************ persistent storage ************************************************


void calc_persist_store(void) {
  persist_write_int(PERSIST_KEY_VERSION, PERSIST_VERSION);
  persist_write_int(PERSIST_KEY_TIP_PERCENT, tip_percent);
  persist_write_data(PERSIST_KEY_BILL, &bill, sizeof(bill));
}


void calc_persist_read(void) {
  if(persist_exists(PERSIST_KEY_BILL)) {
    persist_read_data(PERSIST_KEY_BILL, &bill, sizeof(bill));
    tip_percent = persist_read_int(PERSIST_KEY_TIP_PERCENT);
    num_splitting = DEFAULT_NUM_SPLITTING;
    calc_update_totals();
  } else {
    calc_reset_to_defaults();
  }
}


// ********************************************* GetTxtCallback callbacks *********************************************


char *calc_get_bill_dollars_txt(void) {
  static char s_buffer[4];
  snprintf(s_buffer, sizeof(s_buffer), "%d", bill.dollars);
  return s_buffer;
}


char *calc_get_bill_cents_txt(void) {
  static char s_buffer[3];
  snprintf(s_buffer, sizeof(s_buffer), "%02d", bill.cents);
  return s_buffer;
}


char *calc_get_tip_percent_txt(void) {
  static char s_buffer[3];
  snprintf(s_buffer, sizeof(s_buffer), "%d", tip_percent);
  return s_buffer;
}


char *calc_get_num_splitting_txt(void) {
  static char s_buffer[2];
  snprintf(s_buffer, sizeof(s_buffer), "%d", num_splitting);
  return s_buffer;
}


char *calc_get_tip_txt(void) {
  static char s_buffer[9];
  snprintf(s_buffer, sizeof(s_buffer), "%d.%02d", tip.dollars, tip.cents);
  return s_buffer;
}


char *calc_get_total_txt(void) {
  static char s_buffer[9];
  snprintf(s_buffer, sizeof(s_buffer), "%d.%02d", total.dollars, total.cents);
  return s_buffer;
}


char *calc_get_total_per_person_txt(void) {
  static char s_buffer[9];
  snprintf(s_buffer, sizeof(s_buffer), "%d.%02d", total_per_person.dollars, total_per_person.cents);
  return s_buffer;
}


// ******************************************* CalcManipCallback callbacks ********************************************

void calc_manip_bill_dollars(int delta) {
  if(bill.dollars + delta > MAX_BILL_DOLLARS) {
    bill.dollars = MIN_BILL_DOLLARS;
  } else if(bill.dollars + delta < MIN_BILL_DOLLARS) {
    bill.dollars = MAX_BILL_DOLLARS;
  } else {
    bill.dollars += delta;
  }
}


void calc_manip_bill_cents(int delta) {
  if(bill.cents + delta > 99) {
    bill.cents = 0;
  } else if(bill.cents + delta < 0) {
    bill.cents = 99;
  } else {
    bill.cents += delta;
  }
}


void calc_manip_tip_percent(int delta) {
  if(tip_percent + delta > MAX_TIP_PERCENT) {
    tip_percent = MIN_TIP_PERCENT;
  } else if(tip_percent + delta < MIN_TIP_PERCENT) {
    tip_percent = MAX_TIP_PERCENT;
  } else {
    tip_percent += delta;
  }
  calc_update_totals();
}


void calc_manip_num_splitting(int delta) {
  if(num_splitting + delta > MAX_NUM_SPLITTING) {
    num_splitting = MIN_NUM_SPLITTING;
  } else if(num_splitting + delta < MIN_NUM_SPLITTING) {
    num_splitting = MAX_NUM_SPLITTING;
  } else {
    num_splitting += delta;
  }
  calc_update_totals();
}


// **************************************************** debugging *****************************************************
// TODO: REMOVE
//char *calc_get_bill_dollars_txt(void) {
//  return "444";
//}
//char *calc_get_bill_cents_txt(void) {
//  return "44";
//}
//char *calc_get_tip_percent_txt(void) {
//  return "40";
//}
//char *calc_get_num_splitting_txt(void) {
//  return "4";
//}
//char *calc_get_tip_txt(void) {
//  return "344.44";
//}
//char *calc_get_total_txt(void) {
//  return "1344.44";
//}
//char *calc_get_total_per_person_txt(void) {
//  return "1344.44";
//}