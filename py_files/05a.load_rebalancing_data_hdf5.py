from datetime import datetime, timedelta
import pymysql
import os.path
import pass_word

import numpy as np
import pandas as pd

def LoadCycles(ccypair,tenor):
    
    myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
    mycursor = myconn.cursor()
    mysql_order = r"select cycle_id, cycle_start, cycle_end from %s_%s_cycles order by cycle_id" % (ccypair,tenor)

    mycursor.execute(mysql_order)
    myconn.commit()
    results = mycursor.fetchall()
    myconn.close()    
    return results

def LoadTxtFile(filename):

    #filename = "../reports/eurusd_1w_500/report_7_2009-05-13_2009-05-20.txt"
    #columns_str = "date time spot chg step slpg ht delta unhedged de_pnl de_total op_chg po_pnl po_total o_delta o_gamma o_theta o_xgm op_prc"
    columns_str = "reb_date,reb_time,reb_spot,reb_spot_chg_pips,reb_step_pips,reb_slippage_pips,reb_no_orders_hit,reb_delta_hedge,reb_unhedged_delta,reb_delta_pnl,reb_total_delta_pnl,reb_option_price_change,reb_portfolio_pnl,reb_total_portfolio_pnl,reb_delta,reb_gamma,reb_theta,reb_xgamma,option_price"
    columns = columns_str.split(",")

    report_df = pd.read_csv(filename,parse_dates=True,skiprows=1,infer_datetime_format=True,names=columns,delim_whitespace=True)
    return report_df

test_type = "spread" # straddle or spread
array_of_hashes = []


if(test_type=="straddle"):
    ccypairs = ["eurusd","usdjpy","gbpusd"]
    tenors = ["1w","1m"]
    thresholds = [100,250,500,750,1000,1250,2500,5000]

    array_of_hashes = [{'test_type':test_type,'thresholds':thresholds,'ccypair':ccypair,'tenor':tenor} 
                       for tenor in tenors
                       for ccypair in ccypairs]

else: #spread
    ccypairs = ["usdjpy","eurusd","gbpusd"]
    tenors = ["1w"]
    thresholds = [50,75,100,250,500,750,1000,1250,2500]
    opttypes = ["put","call"] # put or call
    delta = 25
    volspreads = ["m05","m025","00","025","05"]

    array_of_hashes = [{'test_type':test_type,'thresholds':thresholds,'delta':delta,'ccypair':ccypair,'tenor':tenor,'opttype':opttype,'volspread':volspread}
                       for volspread in volspreads
                       for opttype in opttypes
                       for tenor in tenors
                       for ccypair in ccypairs]

reportdir = "../hdf5s";
if not os.path.exists(reportdir):
    os.makedirs(reportdir)

for myhash in array_of_hashes:

    cycles_results = LoadCycles(myhash['ccypair'],myhash['tenor'])

    for threshold in myhash['thresholds']:

        database = "%s_%s_%s" % (myhash['ccypair'],myhash['tenor'],threshold)
        if(myhash['test_type']=='straddle'):
            h5_filename = reportdir+"/%s_%s.h5" % (database,myhash['test_type'])
        else:
            h5_filename = reportdir+"/%s_%s_%s%s_%s.h5" % (database,myhash['test_type'],myhash['delta'],myhash['opttype'],myhash['volspread'])

        dataframes = []

        for (cycle_id, cycle_start,cycle_end) in cycles_results:

            if(myhash['test_type']=="straddle"):
                txt_filename = "../reports/%s_%s_%s/report_%s_%s_%s.txt" % (myhash['ccypair'],myhash['tenor'],threshold,cycle_id,str(cycle_start),str(cycle_end))
            else:
                #eurusd_1w_25callspr_00_50
                txt_filename = "../reports/%s_%s_%s%sspr_%s_%s/report_%s_%s_%s.txt" % (myhash['ccypair'],myhash['tenor'],delta,myhash['opttype'],myhash['volspread'],threshold,cycle_id,str(cycle_start),str(cycle_end))

            if not os.path.exists(txt_filename):
                print("no ", txt_filename)
            else:
                txt_df = LoadTxtFile(txt_filename)
                txt_df["reb_cycle_id"]=cycle_id

                dataframes.append(txt_df)

        result_df = pd.concat(dataframes)
        result_df.to_hdf(h5_filename,database,mode='w')
