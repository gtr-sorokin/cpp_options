#ifndef _FXOPTIONSCOMBINATION_H
#define _FXOPTIONSCOMBINATION_H

#include<iostream>
#include<math.h>
#include "dt.h"
#include "norm.h"
#include "fxoptions.h"
#include <cstdlib>

class FXOptionsCombination{
 public:
  FXOptionsCombination(){};
  virtual ~FXOptionsCombination(){};
  virtual void PrintParams() const = 0;

  //each of these functions returns a parameter that is the same for all options in a combination:
  double GetLinearDeltaWidth() const {return this->portf[0]->GetLinearDeltaWidth();}
  double GetC1Notional() const {return this->portf[0]->GetC1Notional();}
  double GetSpot() const {return this->portf[0]->GetSpot();}
  dt GetMaturityDate() const {return this->portf[0]->GetMaturityDate();}
  dt GetCurrentDate() const {return this->portf[0]->GetCurrentDate();}
  double GetForward() const {return this->portf[0]->GetForward();}
  double GetR_c2() const {return this->portf[0]->GetR_c2();}
  double GetR_c1() const {return this->portf[0]->GetR_c1();}
  double GetYTM() const {return this->portf[0]->GetYTM();}
  double GetMinYTM() const {return this->portf[0]->GetMinYTM();}
  option_ccypair GetOptionCcyPair(int i=0) const {return this->portf[i]->GetOptionCcyPair();}
  option_direction GetOptionDir(int i=0) const {return this->portf[i]->GetOptionDir();}
  option_type GetOptionType(int i=0) const {return this->portf[i]->GetOptionType();}

  static FXOptionsCombination* DynamicCaster(FXOptionsCombination *);

  //each of these functions returns a parameter that may vary for options in a combination:
  double GetUSDNotional(int i=0) const {return this->portf[i]->GetUSDNotional();} 
  double GetC2Notional(int i=0) const {return this->portf[i]->GetC2Notional();}
  double GetStrike(int i=0) const {return this->portf[i]->GetStrike();} 
  double GetVol(int i=0) const {return this->portf[i]->GetVol();}

  ////combination of risks/parameters
  double GetUSDPayout(double) const;
  double GetUSDPrice() const;
  double GetUSDVega(double ch) const;
  double GetUSDTheta(double ch) const; 
  double GetUSDDelta(double step) const;
  double GetUSDGamma(double step) const;
  double GetUSDGammaForStepCalculation() const;
  double GetDeltaC1Amount() const;

  void UpdateRisks();   
  void UpdateDT(const dt&);
  void UpdateSpot(double);
  void UpdateSpotDT(double, const dt&);
  //void UpdateVol(double); 

 protected:
  fxopt ** portf = NULL;
  unsigned int num_portf;
};


#endif
