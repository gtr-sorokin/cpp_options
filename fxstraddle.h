#ifndef _FXSTRADDLE_H
#define _FXSTRADDLE_H

#include<iostream>
#include<math.h>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include "fxoptionscombination.h"
#include <cstdlib>

class FXStraddle: public FXOptionsCombination {
 public:
  FXStraddle(double, double, double, double, double,         dt, dt, option_direction, option_ccypair, double, double);
  FXStraddle(double, double, double, double, double, double, dt, dt, option_direction, option_ccypair, double, double);
  FXStraddle(const FXStraddle &);
  FXStraddle(FXStraddle *);
  ~FXStraddle();

  void PrintParams() const;
};

void FXStraddle::PrintParams() const{
  std::printf("K=%9.5f; F=%9.5f; V=%5.2f; YTM=%6.6f\n",this->GetStrike(), this->GetForward(), this->GetVol()*100, this->GetYTM());
  std::printf("s=%9.5f; p=%8.0f; d=%9.0f; C1not=%10.0f\n",this->GetSpot(),this->GetUSDPrice(), this->GetDeltaC1Amount(),this->GetC1Notional());
}

FXStraddle::FXStraddle(const FXStraddle & straddletocopy){
  this->num_portf = 2;
  this->portf = new fxopt*[2];

  for(int i=0; i<this->num_portf;i++){
    this->portf[i] = new fxopt(straddletocopy.GetStrike(),straddletocopy.GetVol(),straddletocopy.GetSpot(),straddletocopy.GetR_c2(),straddletocopy.GetR_c1(),straddletocopy.GetUSDNotional(),straddletocopy.GetMaturityDate(),straddletocopy.GetCurrentDate(),straddletocopy.GetOptionType(i),straddletocopy.GetOptionDir(i),straddletocopy.GetOptionCcyPair(),straddletocopy.GetLinearDeltaWidth(),straddletocopy.GetMinYTM());
  }
}

FXStraddle::FXStraddle(FXStraddle * straddletocopy){
  this->num_portf = 2;
  this->portf = new fxopt*[2];

  for(int i=0; i<this->num_portf;i++){
    this->portf[i] = new fxopt(straddletocopy->GetStrike(),straddletocopy->GetVol(),straddletocopy->GetSpot(),straddletocopy->GetR_c2(),straddletocopy->GetR_c1(),straddletocopy->GetUSDNotional(),straddletocopy->GetMaturityDate(),straddletocopy->GetCurrentDate(),straddletocopy->GetOptionType(i),straddletocopy->GetOptionDir(i),straddletocopy->GetOptionCcyPair(),straddletocopy->GetLinearDeltaWidth(),straddletocopy->GetMinYTM());
  }
}

FXStraddle::FXStraddle(double K, double V, double S, double F, double notional, dt mat, dt cur, option_direction mydir, option_ccypair mycp, double linear_delta_width, double min_ytm){
  this->num_portf = 2;
  this->portf = new fxopt*[2];
  //(double K, double V, double S, double F, double notional, dt mat, dt cur, option_type mytype, option_direction mydir, option_ccypair mycp,double,double)
  this->portf[0] = new fxopt(K,V,S,F,notional,mat,cur,call,mydir,mycp, linear_delta_width, min_ytm);
  this->portf[1] = new fxopt(K,V,S,F,notional,mat,cur, put,mydir,mycp, linear_delta_width, min_ytm);
}

//(double K, double V, double S, double r_c2, double r_c1, double notional, dt mat, dt cur, option_type mytype, option_direction mydir, option_ccypair mycp,double,double)
FXStraddle::FXStraddle(double K, double V, double S, double r_c2, double r_c1, double notional, dt mat, dt cur, option_direction mydir, option_ccypair mycp, double linear_delta_width, double min_ytm){

  this->num_portf = 2;
  this->portf = new fxopt*[2];

  this->portf[0] = new fxopt(K,V,S,r_c2,r_c1,notional,mat,cur,call,mydir,mycp, linear_delta_width, min_ytm);
  this->portf[1] = new fxopt(K,V,S,r_c2,r_c1,notional,mat,cur, put,mydir,mycp, linear_delta_width, min_ytm);
}

FXStraddle::~FXStraddle(){
  for(int i=0;i<this->num_portf;i++){
    delete this->portf[i];
    this->portf[i]=NULL;
  }
  delete[] this->portf;
  this->portf=NULL;
}

#endif
