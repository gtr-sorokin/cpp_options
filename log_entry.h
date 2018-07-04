#ifndef _LOG_ENTRY_H
#define _LOG_ENTRY_H

#include<iostream>
#include<math.h>
#include <cstdlib>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include "fxoptionscombination.h"
#include "fxstraddle.h"
#include "fxspread.h"
#include "portfolio.h"


class Portfolio;

class log_entry {
 public:
  friend class Portfolio;
  log_entry(const Portfolio&);
  log_entry(const log_entry&);
  ~log_entry();

 private:
  //these parameters are calculated at creation
  dt * reb_dt;
  double reb_spot = 0.0;
  double reb_delta_hedge = 0.0;
  unsigned int reb_num_orders_hit = 0;

  FXOptionsCombination * straddle = NULL;

  double reb_step = 0.0;
  double reb_step_pips = 0.0;
  double reb_slippage_pips = 0.0;

  double reb_delta_unhedged = 0.0;
  double reb_opt_price = 0.0;
  double reb_spot_change_pips=0.0;

  double reb_delta_pnl = 0.0;
  double total_delta_pnl = 0.0;

  double reb_opt_price_change = 0.0;

  double reb_portf_pnl = 0.0;
  double total_portf_pnl = 0.0;

  double opt_delta_imp = 0.0;
  double opt_gamma_imp = 0.0;
  double opt_theta_imp = 0.0;
  double opt_xgamma_imp = 0.0;

  double total_traded_c1_notional = 0.0;
};

#endif
