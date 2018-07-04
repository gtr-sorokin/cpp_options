#ifndef _FXOPTIONSCOMBINATION_HPP
#define _FXOPTIONSCOMBINATION_HPP

#include<iostream>
#include<math.h>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include <cstdlib>
#include "fxoptionscombination.h"
#include "fxstraddle.h"
#include "fxspread.h"

FXOptionsCombination * FXOptionsCombination::DynamicCaster(FXOptionsCombination * p){
  FXStraddle* p1 = dynamic_cast<FXStraddle *>(p);
  if(p1!=NULL){
    FXOptionsCombination *preturn = new FXStraddle(p1);
    p1 = NULL;
    return preturn;
  }

  FXSpread* p2 = dynamic_cast<FXSpread *>(p);
  if(p2!=NULL){
    FXOptionsCombination *preturn = new FXSpread(p2);
    p2 = NULL;
    return preturn;
  }
}

double FXOptionsCombination::GetUSDPayout(double last_quote) const {
  double sum=0;
  for(int i=0;i<this->num_portf;i++){

    /*
    std::cout<<"num i:     "<<i<<std::endl;
    std::cout<<"payout:     "<<this->portf[i]->GetUSDPayout(last_quote)<<std::endl;
    std::cout<<"last_quote: "<<last_quote<<std::endl;
    std::cout<<"strike:     "<<this->portf[i]->GetStrike()<<std::endl<<std::endl;
    std::cout<<"C1 not:     "<<this->portf[i]->GetC1Notional()<<std::endl<<std::endl;
    */

    sum+=this->portf[i]->GetUSDPayout(last_quote);
  }
  return sum;
}

double FXOptionsCombination::GetUSDPrice() const{
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetUSDPrice();
  }
  return sum;
}

double FXOptionsCombination::GetUSDVega(double ch) const{
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetUSDVega(ch);
  }
  return sum;
}

double FXOptionsCombination::GetUSDTheta(double ch) const{ // {retur
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetUSDTheta(ch);
  }
  return sum;
}

double FXOptionsCombination::GetUSDDelta(double step) const{
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetUSDDelta(step);
  }
  return sum;
}

double FXOptionsCombination::GetUSDGamma(double step) const{
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetUSDGamma(step);
  }
  return sum;
} 

double FXOptionsCombination::GetUSDGammaForStepCalculation() const{
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetUSDGammaForStepCalculation();
  }
  return sum;
}

double FXOptionsCombination::GetDeltaC1Amount() const{
  double sum=0;
  for(int i=0;i<this->num_portf;i++){
    sum+=this->portf[i]->GetDeltaC1Amount();
  }
  return sum;
}

void FXOptionsCombination::UpdateRisks() {
  for(int i=0;i<this->num_portf;i++){
    this->portf[i]->UpdateRisks();
  }
}

void FXOptionsCombination::UpdateSpotDT(double x, const dt& newdt){
  for(int i=0;i<this->num_portf;i++){
    this->portf[i]->UpdateSpotDT(x,newdt);
  }
}

void FXOptionsCombination::UpdateDT(const dt& newdt){
  for(int i=0;i<this->num_portf;i++){
    this->portf[i]->UpdateDT(newdt);
  }
}

void FXOptionsCombination::UpdateSpot(double x){
  for(int i=0;i<this->num_portf;i++){
    this->portf[i]->UpdateSpot(x);
  }
}

#endif
