#include <stdlib.h>
#include <iostream>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "dt.h"
#include "portfolio.h"
#include "log_entry.h"
#include "aux_classes.h"

#include "portfolio.hpp"
#include "log_entry.hpp"
#include "aux_classes.hpp"

#include <algorithm>
#include <climits>

#include "params.h"

#include <sys/stat.h>

using namespace std;

int main(int argc, char **argv){

  if(argc<2){
    cout<<"usage: program_name conf_file1 conf_file2 ...\n";
    return 0;
  }

  struct stat st1;
  if(stat("reports",&st1)!=0){
    system("mkdir reports");
  }

  c_params **my_params = new c_params*[argc-1];

  sql::Driver *driver;
  driver = get_driver_instance();
  sql::Connection *con;
  sql::Statement *stmt;
  sql::ResultSet *res;

  for(int k=0;k<argc-1;k++){
    cout<<"reading conf file "+to_string(k+1)+" of "+to_string(argc-1)+"... ";
    my_params[k] = new c_params(argv[k+1]);
    cout<<"done\n";

    for(int j2=0;j2<my_params[k]->num_thresholds;j2++){
      string dirname = my_params[k]->folder_name+"_"+to_string(my_params[k]->thresholds[j2]);
      cout<<"preparing directory "+dirname+" ...";

      struct stat st2;
      if(stat(dirname.c_str(),&st2)!=0){
	system(("mkdir "+dirname).c_str());
      }
      cout<<"done\n";
    }
  }

  for(int k=0;k<argc-1;k++){
    cout<<"connecting to database... ";
    con = driver->connect(my_params[k]->host, my_params[k]->user_name, my_params[k]->password);
    con->setSchema(my_params[k]->database_name);
    cout<<"done\n";

    string sql_query_cycles = my_params[k]->generate_sql_string();

    cout<<"loading cycles... ";
    stmt = con->createStatement();
    res = stmt->executeQuery(sql_query_cycles);

    int num_cycles = 0;
    int max_num_cycles = 2500;
    c_cycle **my_cycles = new c_cycle*[max_num_cycles];

    while (res->next()) {

      string cycle_id = res->getString(1);
      string cycle_start = res->getString(2);
      string cycle_end = res->getString(3);
      string start_quote = res->getString(4);
      string strike = res->getString(5);
      string forward = res->getString(5);
      string vol = res->getString(6);
      string minrowid = res->getString(7);
      string maxrowid = res->getString(8);

      my_cycles[num_cycles] = new c_cycle(cycle_id,cycle_start,my_params[k]->start_time,cycle_end,"10:00:00.000",start_quote,strike,forward,vol,minrowid,maxrowid,sell,my_params[k]->ccypair,my_params[k]->manual_rebalancing_delta_fraction);
      num_cycles++;
    }

    cout<<"done\n"; //cycles data has been loaded

    int super_cycle_num = my_params[k]->num_cycles;

    c_arrayofquotes * my_quotesarray = new c_arrayofquotes(my_params[k]->database_name,my_params[k]->user_name,my_params[k]->password,my_params[k]->host,my_params[k]->cycles_table_name,my_params[k]->quotes_table_name,my_params[k]->mapping_table_name);

    for(int j=0; j<(int)(ceil(num_cycles/((double)super_cycle_num))); j++){
      cout<<"\tsuper cycle "+to_string(j+1)+" of "+to_string(int(ceil(num_cycles/((double)super_cycle_num))))+"\n";
      
      unsigned int minrowid = UINT_MAX;
      unsigned int maxrowid = 0;

      for (int j1=super_cycle_num*j;j1<min(super_cycle_num*(j+1),num_cycles);j1++){
         
	if(minrowid>my_cycles[j1]->get_minrowid()){
	  minrowid = my_cycles[j1]->get_minrowid();
	}
	if(maxrowid<my_cycles[j1]->get_maxrowid()){
	  maxrowid = my_cycles[j1]->get_maxrowid();
	}
      }

      my_quotesarray->UpdateMinMaxRowID(minrowid,maxrowid);
      my_quotesarray->LoadQuotes(driver);

      for (int i=0; i<my_quotesarray->num_quotes;i++){
	for (int j1=super_cycle_num*j;j1<min(super_cycle_num*(j+1),num_cycles);j1++){
	  my_cycles[j1]->add_quote(my_quotesarray->qarray[i]);
	}
      }

      for (int j1=super_cycle_num*j;j1<min(super_cycle_num*(j+1),num_cycles);j1++){
	cout<<"\t\tcycle_id: "<<my_cycles[j1]->cycle_id<<"\n";
	cout<<"\t\tnum quotes: "<<my_cycles[j1]->num_quotes<<"\n";

	for (int j2=0; j2<my_params[k]->num_thresholds;j2++){
	  cout<<"\t\t\tthreshold: "<<my_params[k]->thresholds[j2]<<"\n";

	  string dirname = my_params[k]->folder_name+"_"+to_string(my_params[k]->thresholds[j2]);

	  c_cycle * t_cycle = new c_cycle (*(my_cycles[j1]));
	
	  //load straddle
	  t_cycle->load_fxoptionscombination(my_params[k]);
	  t_cycle->my_portf->SetThreshold(my_params[k]->thresholds[j2]);

	  //first rebalancing
	  // cout<<"first rebalancing\n";
	  double fill_rate = t_cycle->my_portf->RebalanceDeltaAtMarket(*(t_cycle->cycle_quotes[0]));
	  t_cycle->set_first_quote(fill_rate);

	  for(int q = 1; q<t_cycle->num_quotes;q++){

	    if(t_cycle->IfSkipQuote(q)){
	      continue;
	    }

	    bool condA = t_cycle->IfFridayRebalancing(*(t_cycle->cycle_quotes[q]));
	    bool condB = t_cycle->IfSundayRebalancing(*(t_cycle->cycle_quotes[q]));
	    bool condC = t_cycle->IfMinYTMRebalancing(*(t_cycle->cycle_quotes[q]));

	    //last rebalancing:
	    if(q==t_cycle->num_quotes-1){
	      //cout<<"last rebalancing\n";
	      
	      delete t_cycle->cycle_quotes[q]->quote_dt;
	      t_cycle->cycle_quotes[q]->quote_dt = new dt(*(t_cycle->cycle_end_dt));

	      fill_rate = t_cycle->my_portf->RebalanceDeltaAtMarket(*(t_cycle->cycle_quotes[q]),true);
	      t_cycle->set_last_quote(fill_rate);
	      t_cycle->my_portf->SetFinalPrice(fill_rate);
	    }
	    else if(condB){
	      //cout<<"Sunday rebalancing\n";
	      t_cycle->CalculateSundaySteps(*(t_cycle->cycle_quotes[q]));
	    }
	    else if(condA||condC){
	      //cout<<"condA or condC rebalancing\n";
	      t_cycle->my_portf->RebalanceDeltaAtMarket(*(t_cycle->cycle_quotes[q]));
	    }
	    else{
	      bool rebalancing_result = t_cycle->my_portf->RebalanceDeltaAtOrder(*(t_cycle->cycle_quotes[q]));

	      //manual rebalancing at market
	      if((!rebalancing_result)&&(t_cycle->IfAdjustOrders(q))){
		// cout<<"manual rebalancing\n";
		t_cycle->my_portf->RebalanceDeltaAtMarket(*(t_cycle->cycle_quotes[q]));
	      }
	    }
	  }

	  cout<<"\t\t\tnum rebalancings : "<<t_cycle->my_portf->GetNumLogEntries()<<"\n";

	  cout<<"\t\t\tsaving results to "<<t_cycle->get_filename(dirname)<<" ";
	  t_cycle->my_portf->WriteToFile(t_cycle->get_filename(dirname));
	  cout<<"done\n";

	  cout<<"\t\t\tappending summary file... ";
	  t_cycle->WriteToFile(dirname+"/00.summary.txt");
	  cout<<" done\n";

	  t_cycle->delete_quotes_array();
	  delete t_cycle;
	  t_cycle=NULL;
	}
	my_cycles[j1]->delete_quotes_array();
	delete my_cycles[j1];
	my_cycles[j1]=NULL;
      }
    }

    delete my_quotesarray;
    my_quotesarray=NULL;

    delete[] my_cycles;
    my_cycles = NULL;
  }

  delete res;    delete stmt;    delete con;  
  res = NULL;    stmt = NULL;    con = NULL;

  for(int k=0;k<argc-1;k++){
    delete my_params[k];
    my_params[k]=NULL;
  }
  delete[] my_params;
  my_params=NULL;
}

