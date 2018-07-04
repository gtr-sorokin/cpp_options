#ifndef _PORTFOLIO_H
#define _PORTFOLIO_H

#include<iostream>
#include<math.h>
#include <cstdlib>
#include "dt.h"
#include "norm.h"

#include "fxoptions.h"
#include "fxstraddle.h"
#include "log_entry.h"
#include "aux_classes.h"

class log_entry;
class order;
class c_quote;

class Portfolio {

 public:
  friend class log_entry;
  friend class c_cycle;
  Portfolio(FXOptionsCombination *);
  Portfolio();
  ~Portfolio(); 

  void LoadFXOptionsCombination(FXOptionsCombination * );
  void LoadFXStraddle(double, double, double, double, double, dt, dt, option_direction, option_ccypair, double, double);
  void LoadFXSpread(double, double, double, double, double, double, double, dt, dt, option_direction, option_ccypair, option_type, double, double);
  void SetThreshold(double);
  int GetNumLogEntries() const {return this->num_log_entries;}

  void CalculateSteps();
  void CalculateOrders();

  double RebalanceDeltaAtMarket(const dt &, const double, const double, const double, const double, bool);
  double RebalanceDeltaAtMarket(const c_quote &, bool);

  checkhit CheckIfOrderIsHit(const double, const double) const;

  bool RebalanceDeltaAtOrder(const dt &, const double, const double, const double, const double);
  bool RebalanceDeltaAtOrder(const c_quote &);

  double GetVolumeDrivenSlippage(double,double,double) const;
  int GetNumberOfHitOrders(order ** , int, double, checkhit) const;

  void PrintOrders() const;
  void PrintSteps() const;
  void CheckIfLogExtensionIsRequired();
  void WriteToFile(std::string) const;

  double GetInitialPrice() const {return this->initial_price;}
  double GetFinalPrice() const {return this->final_price;}
  void SetFinalPrice(double);

  double GetTotalPortfolioPnl() const {return this->log_entries[this->num_log_entries-1]->total_portf_pnl;}
  double GetTotalDeltaPnl() const {return this->log_entries[this->num_log_entries-1]->total_delta_pnl;}
  double GetTotalTradedC1Notional() const {return this->log_entries[this->num_log_entries-1]->total_traded_c1_notional;}

 public:
  FXOptionsCombination * straddle = NULL;

 private:
  static double GetStrikeForGivenDelta(double,double,double,double);

  bool CheckIfTooManyRebalancings();
  bool GetLastSpotChangeDirection();
  double GetStepValue(double, double, double);

  double last_reb_step = 0.0;
  double last_reb_step_pips = 0.0;

  double last_reb_spot = 0.0;
  double last_reb_delta_hedge=0.0; //total amount of delta hedge
  dt *last_reb_dt = NULL;
  unsigned int last_num_orders_hit = 0;
  double last_slippage_pips = 0.0;

  double max_step_pct = 1.15/100;
  double sunday_step_pct = 0.039/100;
  double USDthreshold=500.0;
  double fx_mult = 10000.0;

  double *top_steps = NULL;
  double *bottom_steps = NULL;
  unsigned int num_top_steps = 5;
  unsigned int num_bottom_steps = 5;
  double threshold_mult = 0.5; // 1 + 0.5 = 1.5 multiple

  order **top_orders = NULL;
  unsigned int num_top = 0;
  order **bottom_orders = NULL;
  unsigned int num_bottom = 0;

  unsigned int hit_orders = 0;
  unsigned int market_orders = 0;
  unsigned int total_orders = 0;

  log_entry **log_entries = NULL;
  unsigned int num_log_entries = 0;
  unsigned int max_num_log_entries = 200;

  double initial_price = 0.0;
  double final_price = 0.0;
};

#endif
