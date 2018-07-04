#ifndef _PORTFOLIO_HPP
#define _PORTFOLIO_HPP

#include <iostream>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include "fxstraddle.h"
#include "log_entry.h"
#include "portfolio.h"
#include <algorithm>
#include "fxoptionscombination.hpp"
#include <cerrno>
#include <cstring>


Portfolio::Portfolio(FXOptionsCombination * mystraddle){
  this->straddle = FXOptionsCombination::DynamicCaster(mystraddle);

  this->log_entries = new log_entry*[this->max_num_log_entries];

  if(this->straddle->GetOptionCcyPair()==XXXUSD){
    this->fx_mult=10000.0;
  }
  else{
    this->fx_mult=100.0;
  }
}

Portfolio::Portfolio(){
  this->log_entries = new log_entry*[this->max_num_log_entries];
}

Portfolio::~Portfolio(){
  delete this->last_reb_dt;
  this->last_reb_dt=NULL;

  delete straddle; 
  straddle=NULL;

  if(top_steps!=NULL){
    delete[] top_steps;
    top_steps = NULL;
  }
  if(bottom_steps!=NULL){
    delete[] bottom_steps;
    bottom_steps = NULL;
  }

  if(this->num_top>0)
    for(int i=0; i<this->num_top;i++){
      delete this->top_orders[i];
      this->top_orders[i]=NULL;
    }
  delete[] this->top_orders;
  this->top_orders = NULL;

  if(this->num_bottom>0)
    for(int i=0; i<this->num_bottom;i++){
      delete this->bottom_orders[i];
      this->bottom_orders[i]=NULL;
    }
  delete[] this->bottom_orders;
  this->bottom_orders = NULL;

  for(int i=0; i<this->num_log_entries;i++){
    delete this->log_entries[i];
    this->log_entries[i]=NULL;
  }
  delete[] this->log_entries;
  this->log_entries=NULL;
}

void Portfolio::WriteToFile(std::string filename) const {

  FILE * pFile;
  pFile = fopen(filename.c_str(),"w");

  if(pFile==NULL){
    perror("Error opening file");
    printf("Error code opening file: %d\n",errno);
    printf("Error opening file: %s\n",strerror(errno));
    exit(-1);
  }
  else{

    fprintf(pFile,"%23s ","dt");

    fprintf(pFile,"%8s %6s %5s %4s %2s %9s %8s ","spot","chg","step","slpg","ht","delta","unhedged");
    //"% 7.0f % 8.0f % 7.0f % 7.0f % 8.0f "
    fprintf(pFile,"%7s %8s %7s %7s %8s ","de_pnl","de_total","op_chg","po_pnl","po_total");
    //"% 7.0f % 7.0f % 7.0f % 7.0f % 7.0f\n"
    fprintf(pFile,"%7s %7s %7s %7s %7s\n","o_delta","o_gamma","o_theta","o_xgm","op_prc");

    for(int i=0; i<this->GetNumLogEntries();i++){
      fprintf(pFile,"%d-%02d-%02d %02d:%02d:%02d.%03d ",
	      log_entries[i]->reb_dt->GetYear(),
	      log_entries[i]->reb_dt->GetMonth(),
	      log_entries[i]->reb_dt->GetDay(),
	      log_entries[i]->reb_dt->GetHour(),
	      log_entries[i]->reb_dt->GetMinute(),
	      log_entries[i]->reb_dt->GetSecond(),
	      log_entries[i]->reb_dt->GetMillisecond());

      fprintf(pFile,"%8.5f %6.1f %5.1f %4.1f %2d % 9.0f % 8.0f ",
	      log_entries[i]->reb_spot,
	      log_entries[i]->reb_spot_change_pips,
	      log_entries[i]->reb_step_pips,
	      log_entries[i]->reb_slippage_pips,
	      log_entries[i]->reb_num_orders_hit,
	      log_entries[i]->reb_delta_hedge,
	      log_entries[i]->reb_delta_unhedged);

      fprintf(pFile,"% 7.0f % 8.0f % 7.0f % 7.0f % 8.0f ",
	      log_entries[i]->reb_delta_pnl,
	      log_entries[i]->total_delta_pnl,
	      log_entries[i]->reb_opt_price_change,
	      log_entries[i]->reb_portf_pnl, 
	      log_entries[i]->total_portf_pnl);

      fprintf(pFile,"% 7.0f % 7.0f % 7.0f % 7.0f % 7.0f\n",
	      log_entries[i]->opt_delta_imp,
	      log_entries[i]->opt_gamma_imp,
	      log_entries[i]->opt_theta_imp,
	      log_entries[i]->opt_xgamma_imp,
	      log_entries[i]->reb_opt_price);
    }
    fclose(pFile);
  }
}


checkhit Portfolio::CheckIfOrderIsHit(const double bid,const double offer) const {
  if(this->num_top>0){
    if(this->top_orders[0]->fxrate<=offer){ return tophit; }
  }  
  if (this->num_bottom>0){
    if(this->bottom_orders[0]->fxrate>=bid){ return bottomhit; }
  }
  return nohit;
}

void Portfolio::LoadFXOptionsCombination(FXOptionsCombination * mystraddle){
  if(this->straddle!=NULL){delete this->straddle;} 
  this->straddle = FXOptionsCombination::DynamicCaster(mystraddle);

  this->initial_price = this->straddle->GetUSDPrice();
  
  if(this->straddle->GetOptionCcyPair()==XXXUSD){
    this->fx_mult=10000.0;
  }
  else{
    this->fx_mult=100.0;
  }
}

//double K, double V, double S, double F, double notional, dt mat, dt cur, option_direction mydir, option_ccypair mycp, double, double
void Portfolio::LoadFXStraddle(double K, double V, double S, double F, double notional, dt mat, dt cur, option_direction odir, option_ccypair ocp, double linear_delta_width, double min_ytm){
  if(this->straddle!=NULL){delete this->straddle;} 
  this->straddle = new FXStraddle(K,V,S,F,notional,mat,cur,odir,ocp, linear_delta_width,min_ytm);
  this->initial_price = this->straddle->GetUSDPrice();

  if(this->straddle->GetOptionCcyPair()==XXXUSD){
    this->fx_mult=10000.0;
  }
  else{
    this->fx_mult=100.0;
  }
}

double Portfolio::GetStrikeForGivenDelta(double forward, double delta, double vol, double ytm){
  return forward * exp(0.5 * vol * vol * ytm - vol * sqrt(ytm)*getinversenorm(delta));
}

void Portfolio::LoadFXSpread(double Kbuy, double strike_delta, double Vbuy, double dV, double S, double F, double notionalbuy, dt mat, dt cur, option_direction odir, option_ccypair ocp, option_type otype, double linear_delta_width, double min_ytm){

  if(this->straddle!=NULL){delete this->straddle;} 

  //  if(otype==call)
  // strike_delta = (strike_delta>0)?strike_delta:-strike_delta;
  //else
  //  strike_delta = (strike_delta>0)?-strike_delta:strike_delta;

  if(otype==put){
    strike_delta = 1.0-strike_delta;
  }

  double ytm = mat.CalculateNumberOfYears(cur);
  ytm = (ytm<0)?0:ytm;

  double Ksell = GetStrikeForGivenDelta(F,strike_delta,Vbuy+dV,ytm);

  this->straddle = new FXSpread(Kbuy,Ksell,Vbuy,dV,S,F,notionalbuy,mat,cur,odir,ocp,otype,linear_delta_width,min_ytm);
  this->initial_price = this->straddle->GetUSDPrice();

  if(this->straddle->GetOptionCcyPair()==XXXUSD){
    this->fx_mult=10000.0;
  }
  else{
    this->fx_mult=100.0;
  }  
}

void Portfolio::SetThreshold(double threshold){
  this->USDthreshold=threshold;
}

void Portfolio::SetFinalPrice(double last_quote){
  this->final_price = this->straddle->GetUSDPayout(last_quote);
}

double Portfolio::GetVolumeDrivenSlippage(double order_size, double quote_size, double slippage = 0.005/100) const {

  //each next quote_size increases the slippage by 'slippage'; the first quote_size has no slippage; 
  if(order_size<=quote_size){
    return 0.0;
  }
  else{ // order_size>quote_size
    int round_i = floor(order_size/quote_size);
    double *sizes = new double[round_i];
    double *slippages = new double[round_i];
    double wa_slippage = 0;

    //std::cout<<"round_i "<<round_i<<std::endl;

    for(int i = 0; i<floor(order_size/quote_size);i++){
      sizes[i]=quote_size;
      slippages[i]=slippage*i;
      wa_slippage+=sizes[i]*slippages[i];
      //std::printf("%4d %10.10f %14.14f\n",i+1,sizes[i],slippages[i]);
    }

    double acc_size = quote_size * (double)(round_i);
    //std::cout<<"acc_size "<<acc_size<<std::endl;
    //std::printf("%4.1f %10.10f %14.14f\n",max_i,(order_size-acc_size),(slippages[round_i-1]+slippage*(order_size-acc_size)/quote_size));

    wa_slippage+=(order_size-acc_size)*(slippages[round_i-1]+slippage*(order_size-acc_size)/quote_size);
    wa_slippage=wa_slippage/order_size;
    
    delete sizes;
    sizes=NULL;
    delete slippages;
    slippages=NULL;

    return wa_slippage;
  }
}

int Portfolio::GetNumberOfHitOrders(order ** orders, int num_orders, double quote, checkhit myhit) const {
  
  int num_hit = 0;
  for (int i=0; i<num_orders; i++){
    //myhit: tophit=+1; bottomhit=-1;
    if((orders[i]->fxrate)*myhit<=quote*myhit)
      num_hit++;
  }
  return num_hit;
}

bool Portfolio::RebalanceDeltaAtOrder(const c_quote & myquote){
  return this->RebalanceDeltaAtOrder(*(myquote.quote_dt),myquote.bid, myquote.bidsize, myquote.offer, myquote.offersize);
}

bool Portfolio::RebalanceDeltaAtOrder(const dt & qtime, const double bid, const double bidsize, const double offer, const double offersize){
  
  checkhit myhit = this->CheckIfOrderIsHit(bid,offer);

  int num_hit_orders = 0; 
  double traded_delta = 0.0;
  double fill_rate = 0.0;
  double volslippage = 0.0;

  double quote = 0.0;
  double quotesize = 0.0;
  int num_orders=0;
  order ** relevant_orders = NULL;

  if(myhit==tophit){ //offer>=top_order
    quote = offer; 
    quotesize = offersize;
    num_orders = this->num_top;
    relevant_orders = new order*[num_orders];
    for(int j=0; j<num_orders; j++){
      relevant_orders[j] = new order(*(this->top_orders[j]));
    }
  }
  else if (myhit==bottomhit){ //bid<=bottom_order
    quote = bid; 
    quotesize = bidsize;
    num_orders = this->num_bottom;
    relevant_orders = new order*[num_orders];
    for(int j=0; j<num_orders; j++){
      relevant_orders[j] = new order(*(this->bottom_orders[j]));
    }
  }

  if((myhit==tophit)||(myhit==bottomhit)){
    num_hit_orders = this->GetNumberOfHitOrders(relevant_orders,num_orders,quote,myhit);

    if(num_hit_orders==1){
      //(double order_size, double quote_size, double slippage = 0.005/100)
      volslippage = quote*this->GetVolumeDrivenSlippage(fabs(relevant_orders[0]->deltac1amt),quotesize);
      fill_rate = relevant_orders[0]->fxrate+volslippage*myhit;
      traded_delta = relevant_orders[0]->deltac1amt;
    }
    else{
      for (int i = 0; i<num_hit_orders; i++){
	traded_delta+=relevant_orders[i]->deltac1amt;
      }
      volslippage = quote*this->GetVolumeDrivenSlippage(fabs(traded_delta),quotesize);
      fill_rate = quote+volslippage*myhit;
    }

    this->last_num_orders_hit = num_hit_orders;
    this->last_slippage_pips = volslippage*this->fx_mult;
    this->last_reb_delta_hedge+=traded_delta;
    this->last_reb_spot = fill_rate;
    delete this->last_reb_dt;
    this->last_reb_dt = new dt(qtime);

    this->straddle->UpdateSpotDT(this->last_reb_spot,*(this->last_reb_dt));

    this->CheckIfLogExtensionIsRequired();
    this->log_entries[this->num_log_entries] = new log_entry(*this);
    this->num_log_entries++;

    //prepare for next iteration
    this->CalculateSteps();
    this->CalculateOrders();

    if(relevant_orders!=NULL){
      for(int j=0; j<num_orders; j++){
	delete relevant_orders[j];
	relevant_orders[j]=NULL;
      }
      delete[] relevant_orders;
      relevant_orders=NULL;
    }

    this->hit_orders+=num_hit_orders;

    return true;
  }
  else{
    return false;
  }
}

double Portfolio::RebalanceDeltaAtMarket(const c_quote & myquote, bool ifdelta100pct=false){
  return this->RebalanceDeltaAtMarket(*(myquote.quote_dt),myquote.bid,myquote.bidsize,myquote.offer,myquote.offersize, ifdelta100pct);
}

double Portfolio::RebalanceDeltaAtMarket(const dt & qtime, const double bid, const double bidsize, const double offer, const double offersize, bool ifdelta100pct=false){

  double mid = 0.5*(bid+offer);
  double traded_delta = 0.0;

  FXOptionsCombination * mystraddle = FXOptionsCombination::DynamicCaster(this->straddle);
  mystraddle->UpdateSpotDT(mid,qtime);

  if(!ifdelta100pct){
    traded_delta = -(this->last_reb_delta_hedge - (-1)*mystraddle->GetDeltaC1Amount());
  }
  else{
    if(
       ((mystraddle->GetOptionCcyPair()==USDXXX)&&
	(fabs(mystraddle->GetDeltaC1Amount()-mystraddle->GetUSDPrice())>0.01*mystraddle->GetC1Notional()))
       ||
       ((mystraddle->GetOptionCcyPair()==XXXUSD)&&
	(fabs(mystraddle->GetDeltaC1Amount())>0.01*mystraddle->GetC1Notional()))
      ){
      traded_delta = -(this->last_reb_delta_hedge - (-1)*mystraddle->GetDeltaC1Amount()/fabs(mystraddle->GetDeltaC1Amount())*mystraddle->GetC1Notional());
    }
    else{
      traded_delta = -(this->last_reb_delta_hedge);
    }
  }

  delete mystraddle;  
  mystraddle=NULL;

  double fill_rate = 0.0;
  double slippage = 0.0;

  if(traded_delta>0){ // offer side
    slippage = GetVolumeDrivenSlippage(fabs(traded_delta),offersize);
    fill_rate = offer+slippage;
  }
  else{ // bid side
    slippage = GetVolumeDrivenSlippage(fabs(traded_delta),bidsize);
    fill_rate = bid-slippage;
  }

  this->last_num_orders_hit=1;
  this->last_slippage_pips = slippage * this->fx_mult;
  this->last_reb_delta_hedge+=traded_delta;
  this->last_reb_spot = fill_rate;
  delete this->last_reb_dt;
  this->last_reb_dt = new dt(qtime);

  this->straddle->UpdateSpotDT(this->last_reb_spot,*(this->last_reb_dt));
  this->CheckIfLogExtensionIsRequired();
  this->log_entries[this->num_log_entries] = new log_entry(*this);
  this->num_log_entries++;

  this->CalculateSteps();
  this->CalculateOrders();

  this->market_orders++;

  return fill_rate;
}

void Portfolio::CheckIfLogExtensionIsRequired(){

  if((this->num_log_entries+1)==this->max_num_log_entries){
    log_entry **tmp_copy = new log_entry*[this->num_log_entries];
    for (int i=0;i<this->num_log_entries;i++){
      tmp_copy[i]=this->log_entries[i];
    }
    delete[] this->log_entries;
    this->log_entries=NULL;

    this->max_num_log_entries = 2*this->max_num_log_entries;
    this->log_entries = new log_entry*[this->max_num_log_entries];

    for (int i=0;i<this->num_log_entries;i++){
      this->log_entries[i]=tmp_copy[i];
      tmp_copy[i]=NULL;
    }
    delete[] tmp_copy;
    tmp_copy=NULL;
  }
}

void Portfolio::PrintSteps() const {
  std::cout<<std::endl;
  std::cout<<"top steps: "<<std::endl;
  for(int i=0;i<this->num_top_steps;i++){
    std::cout<<this->top_steps[i]<<std::endl;
  }
  std::cout<<std::endl;
  std::cout<<"bottom steps: "<<std::endl;
  for(int i=0;i<this->num_bottom_steps;i++){
    std::cout<<this->bottom_steps[i]<<std::endl;
  }
}

void Portfolio::PrintOrders() const {
  std::cout<<std::endl;
  std::cout<<"top orders: "<<std::endl;
  for(int i=0;i<this->num_top;i++){
    std::printf("%9.5f %+10.0f\n",this->top_orders[i]->fxrate,this->top_orders[i]->deltac1amt);
  }

  std::cout<<std::endl;
  std::cout<<"bottom orders: "<<std::endl;
  for(int i=0;i<this->num_bottom;i++){
    std::printf("%9.5f %+10.0f\n",this->bottom_orders[i]->fxrate,this->bottom_orders[i]->deltac1amt);
  }
}

void Portfolio::CalculateOrders(){

  if(this->num_top>0)
    for(int i=0; i<num_top;i++){
      delete this->top_orders[i];
      this->top_orders[i]=NULL;
    }
  delete[] this->top_orders;
  this->top_orders = NULL;

  if(this->num_bottom>0)
    for(int i=0; i<num_bottom;i++){
      delete this->bottom_orders[i];
      this->bottom_orders[i]=NULL;
    }
  delete[] this->bottom_orders;
  this->bottom_orders = NULL;

  this->top_orders = new order*[this->num_top_steps];
  this->bottom_orders = new order*[this->num_bottom_steps];

  double accumulated_top_delta = this->last_reb_delta_hedge;
  double accumulated_bottom_delta = this->last_reb_delta_hedge;

  this->num_top = 0;
  this->num_bottom = 0;

  for(int i = 0;i<this->num_top_steps;i++){
    //FXStraddle * newstraddle_top = new FXStraddle(*(this->straddle));
    FXOptionsCombination * newstraddle_top = FXOptionsCombination::DynamicCaster(this->straddle);

    double toprate = newstraddle_top->GetSpot()+this->top_steps[i];
    newstraddle_top->UpdateSpot(toprate);
    double newtopdelta = -newstraddle_top->GetDeltaC1Amount();
    double top_adj = newtopdelta-accumulated_top_delta;

    //short straddle delta gets more negative
    //delta hedge gets more positive

    // if((top_adj>0)&&(top_adj>this->straddle->GetC1Notional()*0.005)){
    //   this->top_orders[this->num_top]=new order(toprate,top_adj);
    //   accumulated_top_delta=newtopdelta;
    //   this->num_top++;
    // }

    if(fabs(top_adj)>this->straddle->GetC1Notional()*0.005){
      this->top_orders[this->num_top]=new order(toprate,top_adj);
      accumulated_top_delta=newtopdelta;
      this->num_top++;
    }

    delete newstraddle_top;
    newstraddle_top = NULL;
  }

  for(int i = 0;i<this->num_bottom_steps;i++){
    //FXStraddle * newstraddle_bottom = new FXStraddle(*(this->straddle));
    FXOptionsCombination * newstraddle_bottom = FXOptionsCombination::DynamicCaster(this->straddle);

    double bottomrate = newstraddle_bottom->GetSpot()-this->bottom_steps[i];
    newstraddle_bottom->UpdateSpot(bottomrate);
    double newbottomdelta = -newstraddle_bottom->GetDeltaC1Amount();
    double bottom_adj = newbottomdelta-accumulated_bottom_delta;

    //short straddle delta gets more positive
    //delta hedge gets more negative

    // if((bottom_adj<0)&&(-bottom_adj>this->straddle->GetC1Notional()*0.005)){
    //   this->bottom_orders[this->num_bottom]=new order(bottomrate,bottom_adj);
    //   accumulated_bottom_delta=newbottomdelta;
    //   this->num_bottom++;
    // }

    if(fabs(bottom_adj)>this->straddle->GetC1Notional()*0.005){
      this->bottom_orders[this->num_bottom]=new order(bottomrate,bottom_adj);
      accumulated_bottom_delta=newbottomdelta;
      this->num_bottom++;
    }

    delete newstraddle_bottom;
    newstraddle_bottom = NULL;
  }
  this->total_orders+=this->num_bottom+this->num_top;
}

bool Portfolio::GetLastSpotChangeDirection(){
  return !std::signbit(this->log_entries[this->num_log_entries-1]->reb_spot_change_pips);
}

bool Portfolio::CheckIfTooManyRebalancings(){
  int lookback = 6;
  if(this->num_log_entries<lookback){
    return false;
  }
  else{
        // check time interval
    dt * dt_0 = new dt(*(this->log_entries[this->num_log_entries-1]->reb_dt));
    dt * dt_1 = new dt(*(this->log_entries[this->num_log_entries-1-lookback+1]->reb_dt));
    
    double minutes = (dt_0->CalculateNumberOfYears(*dt_1))*(365*24*60);

    delete dt_0;
    delete dt_1;
    dt_0 = NULL;
    dt_1 = NULL;

    if(minutes>double(lookback)){
      return false;
    }
    else{
      // check sign
      //0 1 2 3 4 5 [6]
      //0   2   4
      //  1   3   5
    
      bool iftoomany=true;
      bool zerosign = !std::signbit(this->log_entries[this->num_log_entries-1]->reb_spot_change_pips);

      for(int i=0; i<=lookback-1;i+=2){
	bool sign0 = !std::signbit(this->log_entries[this->num_log_entries-1-i]->reb_spot_change_pips);
	bool sign1 = !std::signbit(this->log_entries[this->num_log_entries-1-i-1]->reb_spot_change_pips);

	if((zerosign==sign0) and (zerosign!=sign1)){
	  continue;
	}
	else{
	  iftoomany=false;
	  break;
	}
      }
      return iftoomany;
    }
  }
}

void Portfolio::CalculateSteps(){

  if(this->top_steps!=NULL){
    delete this->top_steps;
    this->top_steps=NULL;
  }

  if(this->bottom_steps!=NULL){
    delete this->bottom_steps;
    this->bottom_steps=NULL;
  }

  this->top_steps = new double[this->num_top_steps];
  this->bottom_steps = new double[this->num_bottom_steps];

  if(this->straddle->GetYTM() > this->straddle->GetMinYTM()){   
    double max_step = this->max_step_pct*this->straddle->GetSpot();

    for (int i=0;i<std::max(this->num_top_steps,this->num_bottom_steps);i++){
      double calculated_step = GetStepValue(this->USDthreshold*(1+this->threshold_mult*(i)),this->straddle->GetUSDGammaForStepCalculation(),max_step);
      double adj_calculated_step = calculated_step+this->straddle->GetSpot()*0.001*i;

      if((i>0)&&(calculated_step == max_step)){
	if(i<this->num_top_steps)
	  this->top_steps[i]   =adj_calculated_step;
	if(i<this->num_bottom_steps)
	  this->bottom_steps[i]=adj_calculated_step;
      }
      else{
	if(i<this->num_top_steps)
	  this->top_steps[i]   =calculated_step;
	if(i<this->num_bottom_steps)
	  this->bottom_steps[i]=calculated_step;
      }
    }
    
    bool iftoomany = this->CheckIfTooManyRebalancings();
    if(iftoomany){
      if(this->GetLastSpotChangeDirection()){

	if(this->num_bottom_steps>0){
	  double ds = this->bottom_steps[1]-this->bottom_steps[0];
	  this->bottom_steps[0]=std::max(this->bottom_steps[0]*2.0,this->last_reb_step*1.5);

	  for(int i=1;i<this->num_bottom_steps;i++){
	    this->bottom_steps[i]=std::max(this->bottom_steps[i]*2.0,this->last_reb_step*1.5+ds*i);
	  }
	}
	else{
	  this->bottom_steps[0]=std::max(this->bottom_steps[0]*2.0,this->last_reb_step*1.5);
	}
      }

      else{
	if(this->num_top_steps>0){
	  double ds = this->top_steps[1]-this->top_steps[0];
	  this->top_steps[0]=std::max(this->top_steps[0]*2.0,this->last_reb_step*1.5);

	  for(int i=1;i<this->num_top_steps;i++){
	    this->top_steps[i]=std::max(this->top_steps[i]*2.0,this->last_reb_step*1.5+i*ds);
	  }
	}
	else{
	  this->top_steps[0]=std::max(this->top_steps[0]*2.0,this->last_reb_step*1.5);	  
	}
      }
    }

  }
  else{
    double small_step_top = (this->straddle->GetLinearDeltaWidth()-1.0)*this->straddle->GetStrike() / this->num_top_steps;
    double small_step_bottom = (this->straddle->GetLinearDeltaWidth()-1.0)*this->straddle->GetStrike() / this->num_bottom_steps;

    for (int i=0;i<std::max(this->num_top_steps,this->num_bottom_steps);i++){
      if(i<this->num_top_steps)
	this->top_steps[i]=small_step_top*(1+i);
      if(i<this->num_bottom_steps)      
	this->bottom_steps[i]=small_step_bottom*(1+i);
    }
  }

  if((this->num_top_steps>0)&&(this->num_bottom_steps>0))
    this->last_reb_step = std::max(this->top_steps[0],this->bottom_steps[0]);
  else if(this->num_top_steps>0)
    this->last_reb_step = this->top_steps[0];    
  else if(this->num_bottom_steps>0)
    this->last_reb_step = this->bottom_steps[0];

  this->last_reb_step_pips = this->last_reb_step*this->fx_mult;
}

double Portfolio::GetStepValue(double USDthreshold, double USDgamma, double max_step) {

  //    def GammaToStepFormula(self,curr2_threshold, abs_gamma):
  //        return min(round(sqrt(curr2_threshold*2/(abs(abs_gamma)))*10000,0),self.max_step)        

  USDgamma = (USDgamma>0)?USDgamma:-USDgamma;
  if(USDgamma!=0){
    double step = sqrt(USDthreshold*2/USDgamma);
    step = round(step*100000)/100000;
    return (step>max_step)?(max_step):step;
  }
  else
    return max_step;
}

#endif
