#ifndef _AUX_CLASSES_HPP
#define _AUX_CLASSES_HPP

#include <string>
#include "dt.h"
#include "fxoptionscombination.h"
#include "fxspread.h"
#include "fxstraddle.h"
#include "portfolio.h"
#include "log_entry.h"
#include "aux_classes.h"

#include "math.h" //fabs

#include "portfolio.hpp"
#include "log_entry.hpp"
#include "fxoptionscombination.hpp"
#include <algorithm>
#include "params.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

void c_quote::printquote(){
  std::printf("%d-%02d-%02d %02d:%02d:%02d.%03d ",
	    this->quote_dt->GetYear(),
	    this->quote_dt->GetMonth(),
	    this->quote_dt->GetDay(),
	    this->quote_dt->GetHour(),
	    this->quote_dt->GetMinute(),
	    this->quote_dt->GetSecond(),
	    this->quote_dt->GetMillisecond());

  std::printf("%7.5f %4.2f %7.5f %4.2f\n",this->bid, this->bidsize/1e6, this->offer, this->offersize/1e6);
}

c_quote::c_quote(const c_quote & mycopy){
  this->quote_dt = new dt(*(mycopy.quote_dt));
  this->bid = mycopy.bid;
  this->bidsize = mycopy.bidsize;
  this->offer = mycopy.offer;
  this->offersize = mycopy.offersize;
  this->rowid = mycopy.rowid;
}

c_quote::c_quote(std::string datetimems, std::string sbid, std::string sbidsize, std::string soffer, std::string soffersize, std::string srowid){
  this->quote_dt = new dt(datetimems);
  this->bid = stod(sbid);
  this->bidsize = stod(sbidsize)*1e6;
  this->offer = stod(soffer);
  this->offersize = stod(soffersize)*1e6;
  this->rowid = stoi(srowid);
}

c_quote::c_quote(std::string datetimems, double dbid, double dbidsize, double doffer, double doffersize, unsigned int uirowid){
  this->quote_dt = new dt(datetimems);
  this->bid = (dbid);
  this->bidsize = (dbidsize)*1e6;
  this->offer = (doffer);
  this->offersize = (doffersize)*1e6;
  this->rowid = uirowid;
}

c_quote::c_quote(std::string sqdate, std::string sqtime, std::string sqms, double dbid, double dbidsize, double doffer, double doffersize, unsigned int uirowid){

  //0.123
  while(sqms.size()<5){
    sqms+="0";
  }  
  std::string datetimems = sqdate+" "+sqtime+sqms.substr(1,sqms.size()-1);

  this->quote_dt = new dt(datetimems);
  this->bid = dbid;
  this->bidsize = dbidsize*1e6;
  this->offer = doffer;
  this->offersize = doffersize*1e6;
  this->rowid = uirowid;
}

c_quote::~c_quote(){
  delete this->quote_dt;
  this->quote_dt = NULL;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

bool c_cycle::IfAdjustOrders(unsigned int q){

  if(!(q%100==0))
    return false;

  dt * current_dt = new dt(*(this->cycle_quotes[q]->quote_dt));
  if((current_dt->GetDayOfWeek()==0)&&(current_dt->GetHour()>=17)&&(current_dt->GetHour()<=23)){
    delete current_dt;
    current_dt=NULL;
    return false;
  }

  double current_spot = 0.5*(this->cycle_quotes[q]->bid+this->cycle_quotes[q]->offer);
  //FXStraddle * mystraddle = new FXStraddle(*(this->my_portf->straddle));
  FXOptionsCombination * mystraddle = FXOptionsCombination::DynamicCaster(this->my_portf->straddle);
  bool ifadjustorders = false;

  mystraddle->UpdateSpotDT(current_spot,*current_dt);

  if(abs(-mystraddle->GetDeltaC1Amount()-this->my_portf->last_reb_delta_hedge)>this->manual_rebalancing_delta_fraction*mystraddle->GetC1Notional())
    ifadjustorders=true;
    
  // if(this->my_portf->num_top>0){
  //   mystraddle->UpdateSpotDT(this->my_portf->top_orders[0]->fxrate,*current_dt);

  //  if(fabs(-mystraddle->GetDeltaC1Amount()-(this->my_portf->top_orders[0]->deltac1amt+this->my_portf->last_reb_delta_hedge))>0.1*mystraddle->GetC1Notional()){
  //     ifadjustorders = true;
  //   }
  // }

  // if((!ifadjustorders)&&(this->my_portf->num_bottom>0)){
  //   mystraddle->UpdateSpotDT(this->my_portf->bottom_orders[0]->fxrate,*current_dt);
  //   if(fabs(-mystraddle->GetDeltaC1Amount()-(this->my_portf->bottom_orders[0]->deltac1amt+this->my_portf->last_reb_delta_hedge))>0.1*mystraddle->GetC1Notional()){
  //     ifadjustorders = true;
  //   }
  // }

  delete mystraddle;
  mystraddle=NULL;
  delete current_dt;
  current_dt = NULL;

  return ifadjustorders;
}

bool c_cycle::IfSkipQuote(unsigned int q) const {
  bool ifskipbid = false;
  bool ifskipoffer = false;

  if(this->my_portf->num_top>0){
    if(this->cycle_quotes[q]->bidsize<0.1*fabs(this->my_portf->top_orders[0]->deltac1amt)){
      ifskipbid=true;
    }
  }

  if((!ifskipbid)&&(this->my_portf->num_bottom>0)){
    if(this->cycle_quotes[q]->offersize<0.1*fabs(this->my_portf->bottom_orders[0]->deltac1amt)){
      ifskipoffer=true;
    }
  }

  return (ifskipbid||ifskipoffer);
}

void c_cycle::WriteToFile(std::string filename){
  FILE * pFile;
  pFile = fopen(filename.c_str(),"a+");

  if(pFile==NULL){
    perror("Error opening file");
    printf("Error code opening file: %d\n",errno);
    printf("Error opening file: %s\n",strerror(errno));
    exit(-1);
  }
  else{

    fprintf(pFile,"%-4d ",this->cycle_id);

    fprintf(pFile,"%d-%02d-%02d %02d:%02d ",
	    this->cycle_start_dt->GetYear(),
	    this->cycle_start_dt->GetMonth(),
	    this->cycle_start_dt->GetDay(),
	    this->cycle_start_dt->GetHour(),
	    this->cycle_start_dt->GetMinute());

    fprintf(pFile,"%d-%02d-%02d %02d:%02d ",
	    this->cycle_end_dt->GetYear(),
	    this->cycle_end_dt->GetMonth(),
	    this->cycle_end_dt->GetDay(),
	    this->cycle_end_dt->GetHour(),
	    this->cycle_end_dt->GetMinute());

    if(this->my_portf->straddle->GetStrike(0)==this->my_portf->straddle->GetStrike(1)){
      fprintf(pFile,"%9.5f %9.5f %9.5f %5.2f ",
	      this->strike,
	      this->first_quote, 
	      this->last_quote,
	      this->vol*100);
    }
    else{
      fprintf(pFile,"%9.5f %9.5f %9.5f %9.5f %5.2f %5.2f ",
	      this->strike,
	      this->my_portf->straddle->GetStrike(1),
	      this->first_quote, 
	      this->last_quote,
	      this->vol*100,
	      this->my_portf->straddle->GetVol(1)*100);    
    }

    fprintf(pFile,"%5d %5d %5d %4d %3d %6d ",
	    this->num_quotes/1000,
	    this->my_portf->num_log_entries,
	    this->my_portf->hit_orders,
	    this->my_portf->market_orders,
	    this->my_portf->hit_orders+this->my_portf->market_orders-this->my_portf->num_log_entries,
	    this->my_portf->total_orders);


    fprintf(pFile,"%7.0f %7.0f % 7.0f % 7.0f % 4.0f %4.0f\n",
	    this->my_portf->GetInitialPrice(),
	    this->my_portf->GetFinalPrice(),
	    this->my_portf->GetTotalDeltaPnl(),
	    this->my_portf->GetTotalPortfolioPnl(),
	    ((this->my_portf->GetInitialPrice()-this->my_portf->GetFinalPrice())+this->my_portf->GetTotalDeltaPnl())-this->my_portf->GetTotalPortfolioPnl(),
	    this->my_portf->GetTotalTradedC1Notional()/1.0e6);

    fclose(pFile);
  }
}

bool c_cycle::IfMinYTMRebalancing(const c_quote & myquote){
  if(this->closetoexpiry){
    return false;
  }
  else{
    dt * current_dt = new dt(*(myquote.quote_dt));
    dt * maturity_dt = new dt(this->my_portf->straddle->GetMaturityDate());
    double ytm = maturity_dt->CalculateNumberOfYears(*current_dt);
    delete current_dt;
    delete maturity_dt;
    current_dt = NULL;
    maturity_dt = NULL;

    if(ytm < this->my_portf->straddle->GetMinYTM()){
      this->closetoexpiry=true;
      return true;
    }
    else{
      return false;
    }
  }
}

bool c_cycle::IfSundayRebalancing(const c_quote & myquote){
  return this->IfSundayRebalancing(*(myquote.quote_dt));
}

bool c_cycle::IfSundayRebalancing(const dt & mydt){  
  if((mydt.GetDayOfWeek()==0)&&(this->last_sunday->lt(mydt,true))){
    if((mydt.GetHour()>=17)&&(mydt.GetMinute()>=0)){
      delete this->last_sunday;
      this->last_sunday=NULL;
      this->last_sunday = new dt(mydt);
      return true;
    }
  }
  return false;
}

void c_cycle::CalculateSundaySteps(const c_quote & myquote){
  dt * current_dt = new dt(*(myquote.quote_dt));
  double current_spot = (myquote.bid+myquote.offer)*0.5;

  double step_pips_value = this->my_portf->sunday_step_pct * this->my_portf->straddle->GetSpot();

  //FXStraddle * existing_straddle = new FXStraddle(*(this->my_portf->straddle));
  FXOptionsCombination * existing_straddle = FXOptionsCombination::DynamicCaster(this->my_portf->straddle);
  
  this->my_portf->straddle->UpdateSpotDT(current_spot,*current_dt);
  this->my_portf->CalculateSteps();

  if(this->my_portf->last_reb_spot+this->my_portf->top_steps[0]<current_spot){
    this->my_portf->top_steps[0]=step_pips_value;
    for (int i = 1; i<this->my_portf->num_top_steps;i++){
      this->my_portf->top_steps[i]=this->my_portf->top_steps[i-1]+step_pips_value;
    }
  }
  else if (this->my_portf->last_reb_spot-this->my_portf->bottom_steps[0]>current_spot){
    this->my_portf->bottom_steps[0]=step_pips_value;
    for (int i = 1; i<this->my_portf->num_bottom_steps;i++){
      this->my_portf->bottom_steps[i]=this->my_portf->bottom_steps[i-1]+step_pips_value;
    }
  }

  //this->my_portf->PrintSteps();
  this->my_portf->CalculateOrders();
  //this->my_portf->PrintOrders();

  delete this->my_portf->straddle;
  this->my_portf->straddle=NULL;
  //this->my_portf->straddle = new FXStraddle(*existing_straddle);
  this->my_portf->straddle = FXOptionsCombination::DynamicCaster(existing_straddle);

  delete current_dt;
  current_dt=NULL;
  delete existing_straddle;
  existing_straddle=NULL;
}

bool c_cycle::IfFridayRebalancing(const c_quote & myquote){  
  if((myquote.quote_dt->GetDayOfWeek()==5)&&(this->last_friday->lt(*(myquote.quote_dt),true))){
    if((myquote.quote_dt->GetHour()>=16)&&(myquote.quote_dt->GetMinute()>=45)){
      delete this->last_friday;
      this->last_friday=NULL;
      this->last_friday = new dt(*(myquote.quote_dt));
      return true;
    }
  }
  return false;
}

bool c_cycle::IfFridayRebalancing(const dt & mydt){
  if((mydt.GetDayOfWeek()==5)&&(this->last_friday->lt(mydt,true))){
    if((mydt.GetHour()>=16)&&(mydt.GetMinute()>=45)){
      delete this->last_friday;
      this->last_friday=NULL;
      this->last_friday = new dt(mydt);
      return true;
    }
  }
  return false;
}

std::string c_cycle::get_filename(std::string dirname){
  std::string filename = dirname+"/report_";
  filename+=std::to_string(this->cycle_id)+"_";
  filename+=this->cycle_start+"_"+this->cycle_end+".txt";
  return filename;
}

std::string c_cycle::sql_getcyclequotes(std::string tablename, std::string start_time){
  std::string stringA = "SELECT quotedate, quotetime, quotems, quotebid, bidvolume, quoteoffer, offervolume, rowid from ";

  std::string stringB = " where ( (rowid>="+std::to_string(this->minrowid)+" and rowid<="+std::to_string(this->maxrowid)+") AND ";
  std::string stringC = " ( (quotedate>'"+(this->cycle_start)+"' or (quotedate='"+(this->cycle_start)+"' and quotetime>='"+start_time+"')) AND ";
  std::string stringD = " (quotedate<'"+(this->cycle_end)+"' or (quotedate='"+(this->cycle_end)+"' and quotetime<'10:00:00')) ) );";

  return stringA+tablename+stringB+stringC+stringD;
}

std::string sql_getquotesfromrowidrange(std::string tablename, unsigned int minrowid, unsigned int maxrowid){
  std::string stringA = "SELECT quotedate, quotetime, quotems, quotebid, bidvolume, quoteoffer, offervolume, rowid from ";
  std::string stringB = " where (rowid>="+std::to_string(minrowid)+" and rowid<="+std::to_string(maxrowid)+");";
  return stringA+tablename+stringB;
}

void c_cycle::add_quote(c_quote * pquote){

  if(!
     ( (pquote->quote_dt->lt(*(this->cycle_start_dt)))
      ||
      (pquote->quote_dt->gt(*(this->cycle_end_dt))) ) 
    ){

      if(this->num_quotes+1==this->max_num_quotes){
	c_quote **temparray = new c_quote*[this->max_num_quotes];

	for(int i=0;i<this->num_quotes;i++){
	  temparray[i] = this->cycle_quotes[i];
	}
	delete[] this->cycle_quotes;
	this->cycle_quotes=NULL;

	this->max_num_quotes = 2*this->max_num_quotes;
	this->cycle_quotes = new c_quote*[this->max_num_quotes];

	for(int i=0;i<this->num_quotes;i++){
	  this->cycle_quotes[i] = temparray[i];
	}
	delete[] temparray;
	temparray = NULL;
      }
      this->cycle_quotes[this->num_quotes] = pquote; 
      this->num_quotes++;
  }
}

void c_cycle::add_quote(std::string sqdate, std::string sqtime, std::string sqms, double dbid, double dbidsize, double doffer, double doffersize, unsigned int uirowid){

  //0.123
  while(sqms.size()<5){
    sqms+="0";
  }  
  std::string datetimems = sqdate+" "+sqtime+sqms.substr(1,sqms.size()-1);
                       //c_quote(std::string datetimems, double dbid, double dbidsize, double doffer, double doffersize)
  c_quote * pquote = new c_quote(datetimems,dbid,dbidsize,doffer,doffersize, uirowid);
  this->add_quote(pquote);
  delete pquote; pquote=NULL;
}

void c_cycle::add_quote(std::string sqdate, std::string sqtime, std::string sqms, std::string sbid, std::string sbidsize, std::string soffer, std::string soffersize, std::string srowid){
  this->add_quote(sqdate,sqtime,sqms,stod(sbid),stod(sbidsize),stod(soffer),stod(soffersize),stoi(srowid));
}

c_cycle::c_cycle(const c_cycle & my_cycle){
  
  this->cycle_id = my_cycle.cycle_id;
  this->cycle_start = my_cycle.cycle_start;
  this->cycle_end = my_cycle.cycle_end;
  this->manual_rebalancing_delta_fraction = my_cycle.manual_rebalancing_delta_fraction;

  this->cycle_start_dt = new dt(*(my_cycle.cycle_start_dt));
  this->cycle_end_dt = new dt(*(my_cycle.cycle_end_dt));

  this->last_friday = new dt(*(my_cycle.last_friday));
  this->last_sunday = new dt(*(my_cycle.last_sunday));

  this->starting_spot = my_cycle.starting_spot;
  this->forward = my_cycle.forward;
  this->vol = my_cycle.vol;
  this->strike = my_cycle.strike;

  this->minrowid = my_cycle.minrowid;
  this->maxrowid = my_cycle.maxrowid;
  
  this->num_quotes=my_cycle.num_quotes;
  this->cycle_quotes=new c_quote*[my_cycle.max_num_quotes];
  for(int i=0;i<my_cycle.num_quotes;i++){
    this->cycle_quotes[i]=my_cycle.cycle_quotes[i];
  }

  this->odir = my_cycle.odir;
  this->ocp = my_cycle.ocp;

  this->my_portf = new Portfolio();
}

c_cycle::c_cycle(std::string scycle_id, std::string cycle_start, std::string cycle_start_time, std::string cycle_end, std::string cycle_end_time, std::string sstarting_spot, std::string sstrike, std::string sforward, std::string svol, std::string sminrowid, std::string smaxrowid, option_direction my_option_direction, option_ccypair my_option_ccypair, double manual_rebalancing_delta_fraction){
  this->cycle_id = stoi(scycle_id);
  this->cycle_start = cycle_start;
  this->cycle_end = cycle_end;
  this->manual_rebalancing_delta_fraction = manual_rebalancing_delta_fraction;

  this->cycle_start_dt = new dt(this->cycle_start+" "+cycle_start_time);
  this->cycle_end_dt = new dt(this->cycle_end+" "+cycle_end_time);

  this->last_friday = new dt("1900-01-01 16:45:00.000");
  this->last_sunday = new dt("1900-01-01 16:45:00.000");

  this->starting_spot = stod(sstarting_spot);
  this->forward = stod(sforward);
  this->vol = stod(svol)*0.01;
  this->strike = stod(sstrike);

  this->minrowid = stoi(sminrowid);
  this->maxrowid = stoi(smaxrowid);

  this->cycle_quotes=new c_quote*[this->max_num_quotes];

  this->odir = my_option_direction;
  this->ocp = my_option_ccypair;

  this->my_portf = new Portfolio();
}

// void c_cycle::load_fxoptionscombination(double linear_delta_width, double min_ytm){
//   //double K, double V, double S, double F, double notional, dt mat, dt cur, option_direction mydir, option_ccypair mycp
//   this->my_portf->LoadStraddle(this->strike,this->vol,this->starting_spot,this->forward,10.0e6,*(this->cycle_end_dt),*(this->cycle_start_dt),this->odir, this->ocp, linear_delta_width, min_ytm);
// }

void c_cycle::load_fxoptionscombination(c_params * my_params){

  if(my_params->my_fxoptionscombination_type==fxstraddle){
    //double K, double V, double S, double F, double notional, dt mat, dt cur, option_direction mydir, option_ccypair mycp
    this->my_portf->LoadFXStraddle(this->strike,this->vol,this->starting_spot,this->forward,10.0e6,*(this->cycle_end_dt),*(this->cycle_start_dt),this->odir, this->ocp, my_params->linear_delta_width, my_params->min_ytm);
    //this->cycle_start_dt->print();
  }
  else{ // fxspread
    this->my_portf->LoadFXSpread(this->strike,*(my_params->strike_delta),this->vol,*(my_params->vol_spread),this->starting_spot,this->forward,10.0e6,*(this->cycle_end_dt),*(this->cycle_start_dt),this->odir, this->ocp, *(my_params->my_option_type), my_params->linear_delta_width, my_params->min_ytm);
  }
}

void c_cycle::delete_all_quotes(){
    for (int i = 0; i<this->num_quotes; i++){
      delete this->cycle_quotes[i];
      this->cycle_quotes[i]=NULL;
    }
    delete[] this->cycle_quotes;
    this->cycle_quotes=NULL;
    this->num_quotes = 0;
}

void c_cycle::delete_quotes_array(){
  delete[] this->cycle_quotes;
  this->cycle_quotes=NULL;
  this->num_quotes = 0;
}

c_cycle::~c_cycle(){
  if(this->num_quotes>0){
    for (int i = 0; i<this->num_quotes; i++){
      delete this->cycle_quotes[i];
      this->cycle_quotes[i]=NULL;
    }
    delete[] this->cycle_quotes;
    this->cycle_quotes=NULL;
  }

  delete this->my_portf;
  this->my_portf=NULL;

  delete this->last_friday;
  delete this->last_sunday;
}

//////////////////////////////
//////////////////////////////
//////////////////////////////


c_arrayofquotes::c_arrayofquotes(std::string database, std::string user, std::string passw, std::string host, std::string cycles_table, std::string quotes_table, std::string mapping_table){
  this->database_name = database;
  this->user_name = user;
  this->password = passw;
  this->host = host;
  this->cycles_table_name = cycles_table;
  this->quotes_table_name = quotes_table;
  this->mapping_table_name = mapping_table;

  this->qarray = new c_quote*[max_num_quotes];
}

void c_arrayofquotes::CheckIfExtensionIsRequired(){
  if(this->num_quotes+1 == this->max_num_quotes){
    c_quote ** temparray = new c_quote*[this->num_quotes];

    for(int i=0;i<this->num_quotes;i++){
      temparray[i]=this->qarray[i];
    }
    delete[] this->qarray;
    this->qarray=NULL;

    this->max_num_quotes = 2*this->max_num_quotes;
    this->qarray = new c_quote*[max_num_quotes];

    for(int i=0;i<this->num_quotes;i++){
      this->qarray[i]=temparray[i];
    }    
    delete[] temparray;
    temparray = NULL;
  }
}

std::string c_arrayofquotes::GenerateSQLStatement(){
  std::string stringA = "SELECT quotedate, quotetime, quotems, quotebid, bidvolume, quoteoffer, offervolume, rowid from ";
  std::string stringB = " where (rowid>="+std::to_string(this->minrowid_to_load)+" and rowid<="+std::to_string(this->maxrowid_to_load)+");";
  return stringA+this->quotes_table_name+stringB;
}

void c_arrayofquotes::LoadQuotes(sql::Driver * driver){

  sql::Connection *con;
  sql::Statement *stmt;
  sql::ResultSet *res;

  std::cout<<"\tconnecting to database... ";
  //driver = get_driver_instance();
  con = driver->connect(this->host,this->user_name,this->password);
  con->setSchema(this->database_name);
  std::cout<<"done\n";

  std::cout<<"\tloading quotes... ";
  stmt = con->createStatement();
  res = stmt->executeQuery(this->GenerateSQLStatement());

  unsigned int loaded_quotes=0;

  while (res->next()){
    loaded_quotes++;
    std::string s1 = res->getString("quotedate");
    std::string s2 = res->getString("quotetime");
    std::string s3 = res->getString("quotems");
    double d4 = res->getDouble("quotebid");
    double d5 = res->getDouble("bidvolume");
    double d6 = res->getDouble("quoteoffer");
    double d7 = res->getDouble("offervolume");
    int i0 = res->getInt("rowid");

    this->CheckIfExtensionIsRequired();
    this->qarray[this->num_quotes]=new c_quote(s1,s2,s3,d4,d5,d6,d7,i0);
    this->num_quotes++;
  }
  
  std::cout<<"\tdone (loaded:"<<loaded_quotes<<" total:"<<this->num_quotes<<")\n";

  delete res; delete stmt; delete con;  
  res = NULL; stmt = NULL; con = NULL;

  this->minrowid = this->qarray[0]->rowid;
  this->maxrowid = this->qarray[this->num_quotes-1]->rowid;

}

void c_arrayofquotes::UpdateMinMaxRowID(unsigned int newminrowid, unsigned int newmaxrowid){

  if((this->minrowid==0)&&(this->maxrowid==0)){
    this->minrowid_to_load = newminrowid;
    this->maxrowid_to_load = newmaxrowid;

    this->minrowid = newminrowid;
    this->maxrowid = newmaxrowid;
  }
  else if(newminrowid<=this->maxrowid){ //there is some overlap
    c_quote ** temparray = new c_quote*[this->num_quotes];
    unsigned int num_temp = 0;

    for(int i=0; i<this->num_quotes;i++){
      if(this->qarray[i]->rowid<newminrowid){
	delete this->qarray[i];
	this->qarray[i]=NULL;
      }
      else if((this->qarray[i]->rowid>=newminrowid)&&(this->qarray[i]->rowid<=newmaxrowid)){
	temparray[num_temp]=this->qarray[i];
	num_temp++;
      }
      else if(this->qarray[i]->rowid>newmaxrowid){
	delete this->qarray[i];
	this->qarray[i]=NULL;
      }
    }

    delete [] this->qarray;
    this->qarray = new c_quote*[this->max_num_quotes];
    this->num_quotes=0;
    for(int i=0;i<num_temp;i++){
      this->qarray[i] = temparray[i];
      num_quotes++;
    }
    delete [] temparray;
    temparray=NULL;
        
    //............        previous set of quotes
    //     .............. new set of quotes
    //     .......        remaining set of quotes
    //            ....... set of quotes to be loaded
    
    this->minrowid = newminrowid;
    this->maxrowid = std::min(this->maxrowid,newmaxrowid);

    this->minrowid_to_load = this->maxrowid+1;
    this->maxrowid_to_load = newmaxrowid;
  }
  else{//(newminrowid>this->maxrowid) // no overlap
    for(int i=0; i<this->num_quotes;i++){
      delete this->qarray[i];
      this->qarray[i]=NULL;
    }
    delete [] this->qarray;
    this->qarray = NULL;

    this->qarray = new c_quote*[this->max_num_quotes];
    this->num_quotes = 0;

    this->minrowid = newminrowid;
    this->maxrowid = newmaxrowid;

    this->minrowid_to_load = newminrowid;
    this->maxrowid_to_load = newmaxrowid;
  }
}

c_arrayofquotes::~c_arrayofquotes(){
  for(int i=0;i<num_quotes;i++){
    delete this->qarray[i];
    this->qarray[i]=NULL;
  }
  delete[] this->qarray;
  this->qarray=NULL;
}

#endif
