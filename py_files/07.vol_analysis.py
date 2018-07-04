from datetime import datetime, timedelta
import pymysql
import os.path
import pass_word
import pandas as pd
import numpy as np
from math import sqrt

def GetNumberOfDays(start,end):
    return (int(end.strftime("%j")) - int(start.strftime("%j")) + (int(end.year)-int(start.year))*365 + (int(end.strftime("%H"))-int(start.strftime("%H")))/24 + (int(end.strftime("%M"))-int(start.strftime("%M")))/(24*60) + (int(end.strftime("%S"))-int(start.strftime("%S")))/(24*60*60))


def GetUSDVega(ccypair,spot,strike,start_d,start_t,end_d,end_t):
    #self.vega = -self.direction*self.spot*exp(-self.rfor*self.time)*sqrt(self.time)*NormalDensityFunction(self.d1)
    #this->vega_c2 = this->vega * this->notional_c1;

    #if(this->ccypair==XXXUSD)
    #return this->vega_c2*change;
    #else
    #return this->vega_c2*change/this->spot;
    
    startdate = datetime.strptime(start_d+" "+start_t,"%Y-%m-%d %H:%M")
    enddate = datetime.strptime(end_d+" "+end_t,"%Y-%m-%d %H:%M")
    
    sq_time_to_maturity = sqrt(GetNumberOfDays(startdate,enddate)/365)
    const = 0.3989422804

    vega = spot * const*sq_time_to_maturity

    if((ccypair=="eurusd")or(ccypair=="gbpusd")):
        notional_c1 = 10.0e6/strike
        return vega*notional_c1*0.01
    else: #usdjpy
        notional_c1 = 10.0e6
        return vega*notional_c1*0.01/spot
    
ccypairs = ["eurusd","usdjpy","gbpusd"]
tenors =   ["1w","1m"]
thresholds = [100,250,500,750,1000,1250,2500,5000]

#ccypairs = ["eurusd"]
#tenors =   ["1w"]
#thresholds = [100]


for ccypair in ccypairs:
    for tenor in tenors:        
        for threshold in thresholds:

            report_file = "../reports/%s_%s_%s/00.summary.txt" % (ccypair, tenor, threshold)
            output_file = "../reports/%s_%s_%s/01.summary.txt" % (ccypair, tenor, threshold)

            col_names = ["cycle_id","cycle_start_date","cycle_start_time","cycle_end_date","cycle_end_time","strike","spot_start","spot_end","vol","num_quotes_mm","num_rebalancings","stop_orders","market_orders","secondary_orders","total_orders","initial_price","final_price","delta_pnl","pnl","residual","C1_traded_notional"]

            results_df = pd.read_csv(report_file,parse_dates=True,skiprows=0,infer_datetime_format=True,names=col_names,delim_whitespace=True,index_col=False)
            results_df = results_df.set_index("cycle_id")

            results_df["vega_pnl"] = 0.0
            results_df["true_vol"] = 0.0


            print("%s %s %s" % (ccypair, tenor, threshold))

            for j in results_df.index:
                #(ccypair,spot,strike,start_d,start_t,end_d,end_t)
                results_df.ix[j,"vega_pnl"]=results_df.ix[j,"pnl"]/GetUSDVega(ccypair,results_df.ix[j,"spot_start"],results_df.ix[j,"strike"],results_df.ix[j,"cycle_start_date"],results_df.ix[j,"cycle_start_time"],results_df.ix[j,"cycle_end_date"],results_df.ix[j,"cycle_end_time"])
                results_df.ix[j,"true_vol"]=results_df.ix[j,"vol"]-results_df.ix[j,"vega_pnl"]

            results_df[["vega_pnl","true_vol"]]=results_df[["vega_pnl","true_vol"]].round(2)

            with open(output_file,'w') as f:
                f.write(results_df.to_string())
                f.close()




