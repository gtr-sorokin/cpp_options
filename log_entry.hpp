#ifndef _LOG_ENTRY_HPP
#define _LOG_ENTRY_HPP

#include <iostream>
#include <math.h>
#include <cstdlib>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include "fxoptionscombination.h"
#include "fxstraddle.h"
#include "fxspread.h"

#include "portfolio.h"
#include "log_entry.h"
#include "fxoptionscombination.hpp"


log_entry::log_entry(const Portfolio & myportf){

  this->reb_step = myportf.last_reb_step;
  this->reb_step_pips = myportf.last_reb_step_pips;

  this->reb_spot = myportf.last_reb_spot;
  
  this->reb_dt = new dt(*(myportf.last_reb_dt));
  this->reb_num_orders_hit = myportf.last_num_orders_hit;

  this->reb_delta_hedge = myportf.last_reb_delta_hedge;
  this->reb_num_orders_hit = myportf.last_num_orders_hit;

  this->straddle = FXOptionsCombination::DynamicCaster(myportf.straddle);
  this->reb_slippage_pips = myportf.last_slippage_pips;

  this->reb_delta_unhedged = -(this->reb_delta_hedge + this->straddle->GetDeltaC1Amount());
  this->reb_opt_price = this->straddle->GetUSDPrice();

  if(myportf.num_log_entries==0){
    this->reb_opt_price = myportf.GetInitialPrice();
  }

  if(myportf.num_log_entries>0){
    FXOptionsCombination * prev_straddle = FXOptionsCombination::DynamicCaster((myportf.log_entries[myportf.num_log_entries-1]->straddle));
    double spot_chg = (this->reb_spot-myportf.log_entries[myportf.num_log_entries-1]->reb_spot);

    this->reb_spot_change_pips = spot_chg * myportf.fx_mult;
    
    if(straddle->GetOptionCcyPair()==XXXUSD){
      this->reb_delta_pnl = myportf.log_entries[myportf.num_log_entries-1]->reb_delta_hedge * spot_chg;
    }
    else {
      this->reb_delta_pnl = myportf.log_entries[myportf.num_log_entries-1]->reb_delta_hedge * spot_chg / this->reb_spot;
    }

    this->total_delta_pnl = myportf.log_entries[myportf.num_log_entries-1]->total_delta_pnl + this->reb_delta_pnl;

    //this->reb_opt_price_change = straddle->GetOptionDir()*(straddle->GetUSDPrice()-prev_straddle->GetUSDPrice());
    this->reb_opt_price_change = straddle->GetOptionDir()*(straddle->GetUSDPrice()-myportf.log_entries[myportf.num_log_entries-1]->reb_opt_price);

    this->reb_portf_pnl = this->reb_opt_price_change + this->reb_delta_pnl;

    this->total_portf_pnl = myportf.log_entries[myportf.num_log_entries-1]->total_portf_pnl + this->reb_portf_pnl;

    double time_chg = this->reb_dt->CalculateNumberOfYears(*(myportf.log_entries[myportf.num_log_entries-1]->reb_dt));

    this->opt_delta_imp = prev_straddle->GetUSDDelta(spot_chg);
    this->opt_gamma_imp = prev_straddle->GetUSDGamma(spot_chg);
    this->opt_theta_imp = prev_straddle->GetUSDTheta(time_chg);
    this->opt_xgamma_imp = this->reb_opt_price_change - (this->opt_delta_imp+this->opt_gamma_imp+this->opt_theta_imp);

    this->total_traded_c1_notional = myportf.log_entries[myportf.num_log_entries-1]->total_traded_c1_notional + fabs(this->reb_delta_hedge);

    delete prev_straddle;
  }
}

log_entry::log_entry(const log_entry & my_entry){
  this->reb_dt = new dt(*(my_entry.reb_dt));
  this->reb_spot = my_entry.reb_spot;
  this->reb_delta_hedge = my_entry.reb_delta_hedge;
  this->reb_num_orders_hit = my_entry.reb_num_orders_hit;

  this->straddle = FXOptionsCombination::DynamicCaster(my_entry.straddle);

  this->reb_step = my_entry.reb_step;
  this->reb_step_pips = my_entry.reb_step_pips;
  this->reb_slippage_pips = my_entry.reb_slippage_pips;

  this->reb_delta_unhedged = my_entry.reb_delta_unhedged;
  this->reb_opt_price = my_entry.reb_opt_price;
  this->reb_spot_change_pips = my_entry.reb_spot_change_pips;

  this->reb_delta_pnl = my_entry.reb_delta_pnl;
  this->total_delta_pnl = my_entry.total_delta_pnl;
  this->reb_opt_price_change = my_entry.reb_opt_price_change;
  this->reb_portf_pnl = my_entry.reb_portf_pnl;
  this->total_portf_pnl = my_entry.total_portf_pnl;

  this->opt_delta_imp = my_entry.opt_delta_imp;
  this->opt_gamma_imp = my_entry.opt_gamma_imp;
  this->opt_theta_imp = my_entry.opt_theta_imp;
  this->opt_xgamma_imp = my_entry.opt_xgamma_imp;
  this->total_traded_c1_notional = my_entry.total_traded_c1_notional;
}

log_entry::~log_entry(){
  if(this->reb_dt!=NULL){
    delete this->reb_dt; 
    this->reb_dt=NULL;
  }
  if(this->straddle!=NULL){
    delete straddle;
    this->straddle=NULL;
  }
}

#endif
