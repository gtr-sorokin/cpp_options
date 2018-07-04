#ifndef _C_PARAMS_H
#define _C_PARAMS_H

#include <iostream>
#include <string>

enum fxoptioncombination_type {fxstraddle, fxspread};


class c_params{
 public:
  c_params(std::string);
  ~c_params();
  std::string generate_sql_string();

  static std::string parsestring(std::string);
  static std::string removehash(std::string);
  static std::string removespacetab(std::string);
  static std::string trimfrontsymbols(std::string,std::string);
  static std::string trimbacksymbols(std::string,std::string);
  static std::string trimsymbols(std::string);
  
  std::string database_name;
  std::string user_name;
  std::string password;
  std::string host;

  std::string cycles_table_name;
  std::string quotes_table_name;
  std::string mapping_table_name;

  fxoptioncombination_type my_fxoptionscombination_type;

  option_type * my_option_type = NULL; // for fxspread only
  double * strike_delta = NULL; // for fxspread only
  double * vol_spread = NULL; // for fxspread only

  option_ccypair ccypair;

  int starting_cycle;
  int ending_cycle;

  int * thresholds;
  int num_thresholds=0;
  
  unsigned int num_cycles;
  double linear_delta_width;
  double min_ytm;
  double manual_rebalancing_delta_fraction;
  std::string conf_file_name;
  std::string folder_name;
  std::string start_time;
};

std::string c_params::generate_sql_string(){
  std::string sql_query_cycles = "SELECT a.cycle_id, a.cycle_start, a.cycle_end, a.starting_spot, a.strike, a.vol_bid, b1.m_minrowid, b2.m_maxrowid from "+this->cycles_table_name+" as a, "+this->mapping_table_name+" as b1, "+this->mapping_table_name+" as b2 where a.cycle_start=b1.m_date and a.cycle_end=b2.m_date and ((not a.vol_bid is null) or (not a.fwd_pts is null)) ";

  if(this->starting_cycle>=1){
    sql_query_cycles+=" and a.cycle_id>="+std::to_string(this->starting_cycle)+" ";
  }
  if(this->ending_cycle>=1){
    sql_query_cycles+=" and a.cycle_id<="+std::to_string(this->ending_cycle)+" ";
  }

  sql_query_cycles+=" order by a.cycle_id;";

  return sql_query_cycles;
}

std::string c_params::parsestring(std::string sline){
  sline = c_params::removehash(sline);
  sline = c_params::trimsymbols(sline);
  return sline;
}

std::string c_params::removehash(std::string sline){
  int comment_pos = sline.find("#"); 
  if(comment_pos!=std::string::npos){
    sline = sline.substr(0,comment_pos);
  }
  return sline;
}

std::string c_params::removespacetab(std::string sline){
  int trailing_space_pos = sline.find(" "); 
  if(trailing_space_pos!=std::string::npos){
    sline = sline.substr(0,trailing_space_pos);
  }
  int tab_pos = sline.find("\t"); 
  if(tab_pos!=std::string::npos){
    sline = sline.substr(0,tab_pos);
  }
  return sline;
}

std::string c_params::trimsymbols(std::string sline){
  sline = c_params::trimfrontsymbols(sline," ");
  sline = c_params::trimbacksymbols(sline," ");
  sline = c_params::trimfrontsymbols(sline,"\t");
  sline = c_params::trimbacksymbols(sline,"\t");
  sline = c_params::trimfrontsymbols(sline," ");
  sline = c_params::trimbacksymbols(sline," ");
  return sline;
}

std::string c_params::trimfrontsymbols(std::string sline, std::string elem=" "){
  int space_pos = sline.find(elem);
  while((space_pos==0)&&(sline.size()>0)){
    sline=sline.substr(1,sline.size()-1);
    space_pos = sline.find(elem);
  }
  return sline;
}

std::string c_params::trimbacksymbols(std::string sline, std::string elem=" "){
  int space_pos = sline.rfind(elem);
  while((space_pos==sline.size()-1)&&(sline.size()>0)){
    sline=sline.substr(0,sline.size()-1);
    space_pos = sline.rfind(elem);
  }
  return sline;
}

c_params::c_params(std::string conf_file){
  FILE *pFile = fopen(conf_file.c_str(), "r");

  if(pFile!=NULL){
    char line[256];

    fgets(line,sizeof(line),pFile);
    this->database_name = c_params::parsestring(std::string(line));

    fgets(line,sizeof(line),pFile);
    this->user_name = c_params::parsestring(std::string(line));

    fgets(line,sizeof(line),pFile);
    this->password = c_params::parsestring(std::string(line));

    fgets(line,sizeof(line),pFile);
    this->host = c_params::parsestring(std::string(line));

    fgets(line,sizeof(line),pFile);
    this->cycles_table_name = c_params::parsestring(std::string(line));
    fgets(line,sizeof(line),pFile);
    this->quotes_table_name = c_params::parsestring(std::string(line));
    fgets(line,sizeof(line),pFile);
    this->mapping_table_name = c_params::parsestring(std::string(line));

    //parse strategy type: fxstraddle or fxspread
    fgets(line,sizeof(line),pFile);
    std::string string1 = c_params::removehash(std::string(line));
    string1 = c_params::trimsymbols(string1);
    
    if(string1 == "fxstraddle"){
      this->my_fxoptionscombination_type=fxstraddle;
    }
    else{
      this->my_fxoptionscombination_type=fxspread;
      char s1[256], s2[256], s3[256], s4[256]; 
      sscanf(string1.c_str(),"%s %s %s %s",s1,s2,s3,s4);

      this->strike_delta = new double (stod(std::string(s3)));
      this->vol_spread   = new double (stod(std::string(s4)));

      if(std::string(s2)=="call")
	this->my_option_type = new option_type(call);
      else
	this->my_option_type = new option_type(put);
    }

    fgets(line,sizeof(line),pFile);
    this->starting_cycle = std::stoi(c_params::parsestring(std::string(line)));

    fgets(line,sizeof(line),pFile);
    this->ending_cycle = std::stoi(c_params::parsestring(std::string(line)));

    //parse thresholds
    thresholds = new int[100];
    fgets(line,sizeof(line),pFile);
    std::string str_thresholds = c_params::removehash(std::string(line));

    int prev_first_pos = 0;
    int first_pos = str_thresholds.find(" ",prev_first_pos); 

    while(first_pos!=std::string::npos){
      if(first_pos!=prev_first_pos){
	thresholds[num_thresholds]=stoi(str_thresholds.substr(prev_first_pos, first_pos-prev_first_pos));
	num_thresholds++;
      }
      prev_first_pos = first_pos+1;
      first_pos = str_thresholds.find(" ",prev_first_pos);
    }

    if(prev_first_pos!=str_thresholds.size()){
      thresholds[num_thresholds]=stoi(str_thresholds.substr(prev_first_pos));
      num_thresholds++;      
    }

    fgets(line,sizeof(line),pFile);
    this->num_cycles = std::stoi(c_params::parsestring(std::string(line)));

    fgets(line,sizeof(line),pFile);
    this->linear_delta_width = std::stod(c_params::parsestring(std::string(line)));

    fgets(line,sizeof(line),pFile);
    this->min_ytm = std::stod(c_params::parsestring(std::string(line)));

    fgets(line,sizeof(line),pFile);
    this->manual_rebalancing_delta_fraction = std::stod(c_params::parsestring(std::string(line)));

    fgets(line,sizeof(line),pFile);
    this->start_time = c_params::parsestring(std::string(line));

    fclose(pFile);

    this->conf_file_name = conf_file;
    this->folder_name = conf_file;

    unsigned int pos1 = this->folder_name.rfind("/");    
    if(pos1!=std::string::npos){
      this->folder_name = this->folder_name.substr(pos1+1,this->folder_name.size()-pos1+1);
    }

    unsigned int pos2 = this->folder_name.rfind(".");
    if(pos2!=std::string::npos){
      this->folder_name = this->folder_name.substr(0,pos2);
    }

    this->folder_name = "reports/"+this->folder_name;
    
    if(this->quotes_table_name.substr(0,3)=="usd"){
      this->ccypair = USDXXX;
    }
    else if (this->quotes_table_name.substr(3,3)=="usd"){
      this->ccypair = XXXUSD;
    }
  }
  else{
    std::cout<<("Cannot open file "+conf_file)<<std::endl;
  }
}


c_params::~c_params(){
  delete[] thresholds;
  thresholds=NULL;

  if(this->strike_delta!=NULL){
    delete this->strike_delta;
    this->strike_delta=NULL;
  }
  if(this->vol_spread!=NULL){
    delete this->vol_spread;
    this->vol_spread=NULL;
  }
  if(this->my_option_type!=NULL){
    delete this->my_option_type;
    this->my_option_type=NULL;
  }
}

#endif
