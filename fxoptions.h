#ifndef _FXOPTIONS_H
#define _FXOPTIONS_H

#include <iostream>
#include <math.h>
#include "dt.h"
#include "norm.h"
#include <algorithm>

enum option_type {put=0, call=1};
enum option_direction {buy=1, sell=-1};
enum option_ccypair {XXXUSD=0, USDXXX=1}; // EURUSD, GBPUSD and USDJPY

class fxopt{
 public:
  fxopt(double, double, double, double, double,         dt, dt, option_type, option_direction, option_ccypair, double, double);
  fxopt(double, double, double, double, double, double, dt, dt, option_type, option_direction, option_ccypair, double, double);
  fxopt(const fxopt &);
  fxopt & operator=(const fxopt &);
  fxopt copy();
  ~fxopt();
  
  void UpdateRisks();
  double GetUSDPrice() const;
  double GetUSDPayout(double) const;
  double GetUSDVega(double) const;
  double GetUSDTheta(double) const;
  double GetUSDDelta(double) const;
  double GetUSDGamma(double) const;
  double GetUSDGammaForStepCalculation() const;
  double GetDeltaC1Amount() const; 

  void UpdateSpot(double);
  void UpdateVol(double);
  void UpdateDT(const dt &);
  void UpdateSpotDT(double, const dt &);

  void PrintOptParams(bool) const;

  dt GetMaturityDate() const {return this->maturity_dt->copy();}
  dt GetCurrentDate() const {return this->current_dt->copy();}
  double GetVol() const {return this->vol;}
  double GetSpot() const {return this->spot;}
  double GetStrike() const {return this->strike;}
  double GetR_c2() const {return this->r_c2;}
  double GetR_c1() const {return this->r_c1;}
  double GetForward() const {return this->forward;}
  double GetUSDNotional() const {return (this->ccypair==XXXUSD)?this->notional_c2:this->notional_c1;}

  double GetC1Notional() const {return this->notional_c1;}
  double GetC2Notional() const {return this->notional_c2;}

  double GetYTM() const {return this->ytm;}
  option_type GetOptionType() const {return this->otype;}
  option_direction GetOptionDir() const {return this->odir;}
  option_ccypair GetOptionCcyPair() const {return this->ccypair;}
  double GetLinearDeltaWidth() const {return this->linear_delta_width;}
  double GetMinYTM() const {return this->min_ytm;}

 private:
  double spot;
  double strike;
  double forward;
  double vol;

  dt * maturity_dt = NULL;
  dt * current_dt = NULL;

  option_type otype;
  option_direction odir;
  option_ccypair ccypair;

  double notional_c1;
  double notional_c2;
  double r_c2; //FOR_DOM = C1_C2
  double r_c1;

  double ytm; //years to maturity

  double price, price_c1, price_c2;
  double delta; //currency1 delta
  double gamma;
  double speed;

  double theta, theta_c1, theta_c2;
  double vega, vega_c1, vega_c2;

  double d1;
  double d2;
  double ncdf_d1;
  double ncdf_d2;

  double linear_delta_width = 1.0014;
  double min_ytm = 1.0/(24*365);

  double LinearDeltaFunction() const;

  void CalculateYTM();
  void CalculateRFRatesFromForward(double rusd=0.0102);
  void CalculateForwardFromRFRates();

  void CalculateD1D2();
  void CalculatePrice();
  void CalculateDelta();
  void CalculateGamma();
  void CalculateSpeed();
  void CalculateTheta();
  void CalculateVega();
  void CalculatePayout();

};

double fxopt::GetDeltaC1Amount() const {
  if(this->ccypair==XXXUSD)
    return this->notional_c1*this->delta;
  else //USDXXX /C1C2
    return (this->notional_c1*this->delta + this->GetUSDPrice());
}

double fxopt::GetUSDPayout(double last_quote) const{

  double return_value=0;

  if(this->GetOptionType()==call){
    return_value=-1.0*((int)this->GetOptionDir())*this->GetC1Notional()*std::max(last_quote-this->GetStrike(),0.0);
  }
  else{
    return_value=-1.0*((int)this->GetOptionDir())*this->GetC1Notional()*std::max(this->GetStrike()-last_quote,0.0);
  }
  
  if(this->GetOptionCcyPair()==USDXXX){
    return_value = return_value/last_quote;
  }

  return return_value;
}

void fxopt::PrintOptParams(bool firstrow=false) const {
         //     __ __ __ ________ ________ ________ ______ _____ _____ _____ ______ ______ ______ ______ ___________
  if(firstrow)
    std::cout<<"tp dr cy   strike     spot  forward    ytm   vol  r_c2  r_c1    nd1    nd2     d1     d2  DeltaC1Amt  C1Notional"<<std::endl;
  else
    printf("%2i %2i %2i %8.4f %8.4f %8.4f %6.4f %5.2f %+4.2f %+4.2f %+3.3f %+3.3f %+3.3f %+3.3f %+11.0f %11.0f\n",this->otype, this->odir, this->ccypair, this->strike, this->spot,this->forward, this->ytm, this->vol*100, this->r_c2*100, this->r_c1*100, this->ncdf_d1, this->ncdf_d2,this->d1, this->d2,this->GetDeltaC1Amount(),this->GetC1Notional());
}

void fxopt::UpdateVol(double vol){
  this->vol=vol;
  this->UpdateRisks();
}

void fxopt::UpdateDT(const dt & newdt){
  delete this->current_dt;
  this->current_dt = NULL;
  this->current_dt = new dt(newdt);
  this->UpdateRisks();
}

void fxopt::UpdateSpot(double newspot){
  this->spot = newspot;
  this->UpdateRisks();
}

void fxopt::UpdateSpotDT(double newspot, const dt & newdt){
  delete this->current_dt;
  this->current_dt = NULL;
  this->current_dt = new dt(newdt);
  this->spot = newspot;
  this->UpdateRisks();  
}

double fxopt::GetUSDPrice() const{
  if(this->ccypair==XXXUSD)
    return this->price_c2;
  else
    return this->price_c1;
}

double fxopt::GetUSDVega(double change) const{
  if(this->ccypair==XXXUSD)
    return this->vega_c2*change;
  else
    return this->vega_c2*change/this->spot;
}

double fxopt::GetUSDTheta(double change) const{
  if(this->ccypair==XXXUSD)
    return this->theta_c2*change;
  else
    return this->theta_c2*change/this->spot;    
}

double fxopt::GetUSDDelta(double change) const{
  if(this->ccypair==XXXUSD)
    return this->GetDeltaC1Amount()*change;
  else
    return this->GetDeltaC1Amount()*change/this->spot;
    //return c2_pnl_amount/this->spot;
}

double fxopt::GetUSDGammaForStepCalculation() const{
  if(this->ccypair==XXXUSD)
    return (this->gamma*this->notional_c1);
  else
    return (this->gamma*this->notional_c1)/this->spot;    
}

double fxopt::GetUSDGamma(double change) const{
  double c2_pnl_amount = this->gamma*this->notional_c1*0.5*change*change;
  if(this->ccypair==XXXUSD)
    return c2_pnl_amount;
  else
    return c2_pnl_amount/this->spot;    
}

void fxopt::UpdateRisks(){
  this->CalculateYTM();
  this->CalculateForwardFromRFRates();
  this->CalculateD1D2();
  this->CalculatePrice();
  this->CalculateDelta();
  this->CalculateGamma();
  this->CalculateTheta();
  this->CalculateVega();
}

void fxopt::CalculateYTM(){
  this->ytm = this->maturity_dt->CalculateNumberOfYears(*(this->current_dt));
  this->ytm = (this->ytm<0)?0:this->ytm;
}

void fxopt::CalculateRFRatesFromForward(double rusd){
  //self.forw = self.spot*exp((self.rdom-self.rfor)*self.time)
  // ln (F/S) = (rdom-rfor)*ytm
  // 1/ytm * ln(F/S) = rdom - rfor

  if(this->ccypair==XXXUSD){
    this->r_c2 = rusd;
    if(this->ytm>this->min_ytm)
      this->r_c1 = this->r_c2 - (1/this->ytm)*log(this->forward/this->spot);
    else
      this->r_c1 = 0.0;
  }
  else{ //USDXXX
    this->r_c1 = rusd;
    if(this->ytm>this->min_ytm)
      this->r_c2 = this->r_c1 + (1/this->ytm)*log(this->forward/this->spot);
    else
      this->r_c2 = 0.0;
  }
}

void fxopt::CalculateForwardFromRFRates(){
  //self.forw = self.spot*exp((self.rdom-self.rfor)*self.time)
  this->forward = this->spot*exp((this->r_c2-this->r_c1)*this->ytm);
}

fxopt fxopt::copy(){
  //double K, double V, double S, double rdom, double rfor, double notional, dt mat, dt cur, option_type mytype, option_direction mydir
  double USDnotional = (this->ccypair==XXXUSD)?this->notional_c2:this->notional_c1;
  return fxopt(this->strike,this->vol,this->spot,this->r_c2,this->r_c1,USDnotional,*(this->maturity_dt), *(this->current_dt), this->otype,this->odir,this->ccypair, this->linear_delta_width, this->min_ytm);
}

void fxopt::CalculateD1D2(){

  if(this->ytm>this->min_ytm){ //up until 1 hour before expiry
    //self.d1 = (log(self.forw/self.strike) + 0.5*self.time*self.vol**2)/(self.vol*sqrt(self.time))
    //self.d2 = self.d1 - self.vol*sqrt(self.time)
    this->d1 = (log(this->forward/this->strike) + 0.5*this->ytm*this->vol*this->vol)/(this->vol*sqrt(this->ytm));
    this->d2 = this->d1 - this->vol*sqrt(this->ytm);
    this->ncdf_d1 = ncdf(d1);
    this->ncdf_d2 = ncdf(d2);
  }
  else if ((this->ytm<this->min_ytm)&&(this->ytm>0))
  {
    this->d1 = (this->spot>this->strike)?5.0:-5.0;
    this->d2 = this->d1;
    
    this->ncdf_d1 = this->LinearDeltaFunction();//ncdf(d1);
    this->ncdf_d2 = this->ncdf_d1;
  }
  else{ //this->ytm==0
    this->d1 = (this->spot>this->strike)?5.0:-5.0;
    this->d2 = this->d1;

    if(this->spot>this->strike)
      this->ncdf_d1 = 1.0;
    else
      this->ncdf_d1 = 0.0;

    this->ncdf_d2 = this->ncdf_d1;
  }
}

double fxopt::LinearDeltaFunction() const {
  double width = this->linear_delta_width;
  double ret_value = 0.0;
  if(this->otype==call){
    if(this->spot<this->strike)
      ret_value = 0.0;
    else//spot>strike
      ret_value = (this->spot-this->strike)/(this->strike*width-this->strike);
  }
  else{ //put
    if(this->spot<this->strike/width)
      ret_value = 0.0;
    else//spot<strike
      ret_value = (this->spot-this->strike/width)/(this->strike-this->strike/width); 
  }
  
  return (ret_value>1.0)?(1.0):ret_value;
}

void fxopt::CalculatePrice(){
  //if self.optiontype == "call":
  //self.price = (-1)*self.direction*exp(-self.rfor*self.time)*(self.forw*self.NormalCDF_d1 - self.strike*self.NormalCDF_d2)
  //else:
  //self.price = (-1)*self.direction*exp(-self.rfor*self.time)*(-self.forw*(1 - self.NormalCDF_d1) + self.strike*(1-self.NormalCDF_d2))

  if(this->otype == call)
    this->price = (-1)*this->odir*exp(-this->r_c1*this->ytm)*(this->forward*this->ncdf_d1 - this->strike*this->ncdf_d2);
  else
    this->price = (-1)*this->odir*exp(-this->r_c1*this->ytm)*(-this->forward*(1-this->ncdf_d1) + this->strike*(1-this->ncdf_d2));

  this->price_c2 = this->price * this->notional_c1;
  this->price_c1 = this->price_c2/this->spot;
}

void fxopt::CalculateDelta(){
    //if self.optiontype == "call":
    //self.delta = self.direction*exp(-self.rfor*self.time)*self.NormalCDF_d1
    //else:
    //self.delta = self.direction*exp(-self.rfor*self.time)*(-1)*(1-self.NormalCDF_d1)
    if(this->otype == call)
      this->delta = this->odir*exp(-this->r_c1*this->ytm)*this->ncdf_d1;
    else
      this->delta = this->odir*exp(-this->r_c1*this->ytm)*(-1)*(1-this->ncdf_d1);
}

void fxopt::CalculateGamma(){

  if(this->ytm>this->min_ytm) //up until 1 hour before expiry
    //self.gamma = self.direction*exp(-self.rfor*self.time)*NormalDensityFunction(self.d1)/(self.spot*self.vol*sqrt(self.time))        
    this->gamma = this->odir * exp(-this->r_c1*this->ytm)*npdf(this->d1)/(this->spot*this->vol*sqrt(this->ytm));
  else
    this->gamma = 0;
}

void fxopt::CalculateSpeed(){
  if(this->ytm>this->min_ytm) //up until 1 hour before expiry
    //self.speed = -(self.gamma/self.spot)*(self.d1/(self.vol*sqrt(self.time))+1)
    this->speed = -(this->gamma/this->spot)*(1+this->d1/(this->vol*sqrt(this->ytm)));
  else
    this->speed = 0;
}

void fxopt::CalculateVega(){

  if(this->ytm>this->min_ytm) //up until 1 hour before expiry
    //self.vega = -self.direction*self.spot*exp(-self.rfor*self.time)*sqrt(self.time)*NormalDensityFunction(self.d1)
    this->vega = this->odir*this->spot*exp(-this->r_c1*this->ytm)*sqrt(this->ytm)*npdf(this->d1);
  else
    this->vega = 0;

  this->vega_c2 = this->vega * this->notional_c1;
  this->vega_c1 = this->vega_c2 / this->spot;
}

void fxopt::CalculateTheta(){

  if(this->ytm>this->min_ytm){ //up until 1 hour before expiry
  /*    a = -exp(-self.rfor*self.time)*NormalDensityFunction(self.d1)*self.spot*self.vol/(2*sqrt(self.time))
        Nd1 = self.NormalCDF_d1
        Nd2 = self.NormalCDF_d2
        if(self.optiontype == "put"):
            Nd1 = 1 - Nd1
            Nd2 = 1 - Nd2

        b = self.rfor*self.spot*exp(-self.rfor*self.time)*Nd1 #self.NormalCDF_d1
        c = self.rdom*self.strike*exp(-self.rdom*self.time)*Nd2 #self.NormalCDF_d2

        if self.optiontype == "call":
            self.theta = self.direction*(a + (b-c))
        else:
            self.theta = self.direction*(a - (b-c))
  */

    double a = -exp(-this->r_c1*this->ytm)*npdf(this->d1)*this->spot*this->vol/(2*sqrt(this->ytm));

    double Nd1 = this->ncdf_d1;
    double Nd2 = this->ncdf_d2;
    if(this->otype==put){
      Nd1 = 1-Nd1; Nd2 = 1-Nd2;
    }

    double b = this->r_c1 * this->spot   * exp(-this->r_c1 * this->ytm) * Nd1;
    double c = this->r_c2 * this->strike * exp(-this->r_c2 * this->ytm) * Nd2;

    if (this->otype==call)
      this->theta = this->odir * (a+(b-c));
    else
      this->theta = this->odir * (a-(b-c));
  }
  else
    this->theta = 0;

  this->theta_c2 = this->theta*this->notional_c1;
  this->theta_c1 = this->theta_c2 / this->spot;
}

///////////////
// constructors
///////////////

fxopt & fxopt::operator=(const fxopt & copy){
  if(this==&copy)
    return *this;
  else{

      this->strike = copy.strike;
      this->vol = copy.vol;
      this->spot = copy.spot;
      this->forward = copy.forward;
      this->ccypair = copy.ccypair;

      if(this->ccypair==XXXUSD){
	this->notional_c2 = copy.GetUSDNotional();
	this->notional_c1 = this->notional_c2/this->strike;
      }
      else{
	this->notional_c1 = copy.GetUSDNotional();
	this->notional_c2 = this->notional_c1 * this->strike;
      }

      this->maturity_dt = new dt(*(copy.maturity_dt));
      this->current_dt = new dt(*(copy.current_dt));
  
      this->otype = copy.otype;
      this->odir = copy.odir;

      this->linear_delta_width = copy.linear_delta_width;
      this->min_ytm = copy.min_ytm;

      this->CalculateYTM();
      this->CalculateRFRatesFromForward();
      this->UpdateRisks();

      return *this;
  }
}

fxopt::fxopt(const fxopt& copy){
  this->strike = copy.strike;
  this->vol = copy.vol;
  this->spot = copy.spot;
  this->forward = copy.forward;
  this->ccypair = copy.ccypair;

  if(this->ccypair==XXXUSD){
    this->notional_c2 = copy.GetUSDNotional();
    this->notional_c1 = this->notional_c2/this->strike;
  }
  else{
    this->notional_c1 = copy.GetUSDNotional();
    this->notional_c2 = this->notional_c1 * this->strike;
  }

  this->maturity_dt = new dt(*(copy.maturity_dt));
  this->current_dt = new dt(*(copy.current_dt));
  
  this->otype = copy.otype;
  this->odir = copy.odir;

  this->linear_delta_width = copy.linear_delta_width;
  this->min_ytm = copy.min_ytm;

  this->CalculateYTM();
  this->CalculateRFRatesFromForward();
  this->UpdateRisks();  
}

//create option using forward
fxopt::fxopt(double K, double V, double S, double F, double notional, dt mat, dt cur, option_type mytype, option_direction mydir, option_ccypair mycp, double linear_delta_width, double min_ytm){

  this->strike = K;
  this->vol = V;
  this->spot = S;
  this->forward = F;
  this->ccypair = mycp;

  if(this->ccypair==XXXUSD){
    this->notional_c2 = notional;
    this->notional_c1 = this->notional_c2/this->strike;
  }
  else{
    this->notional_c1 = notional;
    this->notional_c2 = this->notional_c1 * this->strike;
  }

  this->maturity_dt = new dt(mat);
  this->current_dt = new dt(cur);
  
  this->linear_delta_width = linear_delta_width;
  this->min_ytm = min_ytm;

  this->otype = mytype;
  this->odir = mydir;

  this->CalculateYTM();
  this->CalculateRFRatesFromForward();
  this->UpdateRisks();
}

//create option using rdom, rfor
fxopt::fxopt(double K, double V, double S, double r_c2, double r_c1, double notional, dt mat, dt cur, option_type mytype, option_direction mydir, option_ccypair mycp, double linear_delta_width, double min_ytm){

  this->strike=K;
  this->vol = V;
  this->spot=S;

  this->r_c2 = r_c2;
  this->r_c1 = r_c1;
  this->ccypair = mycp;

  if(this->ccypair==XXXUSD){
    this->notional_c2 = notional;
    this->notional_c1 = this->notional_c2/this->strike;
  }
  else{
    this->notional_c1 = notional;
    this->notional_c2 = this->notional_c1 * this->strike;
  }

  this->maturity_dt = new dt(mat);
  this->current_dt = new dt(cur);

  this->linear_delta_width = linear_delta_width;
  this->min_ytm = min_ytm;

  this->otype = mytype;
  this->odir = mydir;

  this->CalculateYTM();
  this->CalculateForwardFromRFRates();
  this->UpdateRisks();
}

fxopt::~fxopt(){
  delete this->maturity_dt;
  delete this->current_dt;
  this->maturity_dt = NULL;
  this->current_dt = NULL;
}

#endif


