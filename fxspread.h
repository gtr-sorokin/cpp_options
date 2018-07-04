#ifndef _FXSPREAD_H
#define _FXSPREAD_H

#include<iostream>
#include<math.h>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include "fxoptionscombination.h"
#include <cstdlib>

class FXSpread: public FXOptionsCombination {
 public:
  FXSpread(double, double, double, double, double, double, double,         dt, dt, option_direction, option_ccypair, option_type, double, double);
  FXSpread(double, double, double, double, double, double, double, double, dt, dt, option_direction, option_ccypair, option_type, double, double);
  FXSpread(const FXSpread &);
  FXSpread(FXSpread *);
  ~FXSpread();

  void PrintParams() const;
};

void FXSpread::PrintParams() const{
  //  std::printf("K=%9.5f; F=%9.5f; V=%5.2f; YTM=%6.6f\n",this->GetStrike(), this->GetForward(), this->GetVol()*100, this->GetYTM());
  //  std::printf("s=%9.5f; p=%8.0f; d=%9.0f; C1not=%10.0f\n",this->GetSpot(),this->GetUSDPrice(), this->GetDeltaC1Amount(),this->GetC1Notional());
}

FXSpread::FXSpread(const FXSpread & straddletocopy){
  this->num_portf = 2;
  this->portf = new fxopt*[2];

  for(int i=0;i<this->num_portf;i++){
    this->portf[i] = new fxopt(straddletocopy.GetStrike(i),straddletocopy.GetVol(i),straddletocopy.GetSpot(),straddletocopy.GetR_c2(),straddletocopy.GetR_c1(),straddletocopy.GetUSDNotional(i),straddletocopy.GetMaturityDate(),straddletocopy.GetCurrentDate(),straddletocopy.GetOptionType(i),straddletocopy.GetOptionDir(i),straddletocopy.GetOptionCcyPair(i),straddletocopy.GetLinearDeltaWidth(),straddletocopy.GetMinYTM());
  }
}

FXSpread::FXSpread(FXSpread * straddletocopy){
  this->num_portf = 2;
  this->portf = new fxopt*[2];

  for(int i=0;i<this->num_portf;i++){
    this->portf[i] = new fxopt(straddletocopy->GetStrike(i),straddletocopy->GetVol(i),straddletocopy->GetSpot(),straddletocopy->GetR_c2(),straddletocopy->GetR_c1(),straddletocopy->GetUSDNotional(i),straddletocopy->GetMaturityDate(),straddletocopy->GetCurrentDate(),straddletocopy->GetOptionType(i),straddletocopy->GetOptionDir(i),straddletocopy->GetOptionCcyPair(i),straddletocopy->GetLinearDeltaWidth(),straddletocopy->GetMinYTM());
  }

}

FXSpread::FXSpread(double Kbuy, double Ksell, double Vbuy, double dV, double S, double F, double notionalbuy, dt mat, dt cur, option_direction mydir, option_ccypair mycp, option_type mytype, double linear_delta_width, double min_ytm){
  this->num_portf = 2;
  this->portf = new fxopt*[2];

  double USDnotionalsell;
  if(mycp==USDXXX){
    USDnotionalsell = notionalbuy;
  }
  else{ // mycp==XXXUSD
    double XXXnotionalbuy = notionalbuy / Kbuy;
    USDnotionalsell = XXXnotionalbuy * Ksell;
  }

  //(double K, double V, double S, double F, double notional, dt mat, dt cur, option_type mytype, option_direction mydir, option_ccypair mycp,double,double)  
  this->portf[0] = new fxopt(Kbuy ,Vbuy,S,F,notionalbuy ,mat,cur,mytype, mydir,mycp,linear_delta_width,min_ytm);
  this->portf[1] = new fxopt(Ksell,Vbuy+dV,S,F,USDnotionalsell,mat,cur,mytype,(option_direction)(-mydir),mycp,linear_delta_width,min_ytm);
}

FXSpread::FXSpread(double Kbuy, double Ksell, double Vbuy, double dV, double S, double r_c2, double r_c1, double notionalbuy, dt mat, dt cur, option_direction mydir, option_ccypair mycp, option_type mytype, double linear_delta_width, double min_ytm){

  this->num_portf = 2;
  this->portf = new fxopt*[2];

  double USDnotionalsell;
  if(mycp==USDXXX){
    USDnotionalsell = notionalbuy;
  }
  else{ // mycp==XXXUSD
    double XXXnotionalbuy = notionalbuy / Kbuy;
    USDnotionalsell = XXXnotionalbuy * Ksell;
  }

  //(double K, double V, double S, double r_c2, double r_c1, double notional, dt mat, dt cur, option_type mytype, option_direction mydir, option_ccypair mycp,double,double)
  this->portf[0] = new fxopt(Kbuy ,Vbuy   , S, r_c2, r_c1, notionalbuy , mat, cur, mytype, mydir, mycp, linear_delta_width, min_ytm);
  this->portf[1] = new fxopt(Ksell,Vbuy+dV, S, r_c2, r_c1, USDnotionalsell, mat, cur, mytype,(option_direction)(-mydir), mycp, linear_delta_width, min_ytm);
}

FXSpread::~FXSpread(){
  for(int i=0;i<this->num_portf;i++){
    delete this->portf[i];
    this->portf[i]=NULL;
  }
  delete[] this->portf;
  this->portf=NULL;
}

#endif
