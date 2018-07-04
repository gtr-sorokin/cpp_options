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

insert_into_cols = "(reb_cycle_id,reb_date,reb_time,reb_spot,reb_spot_chg_pips,reb_step_pips,reb_slippage_pips,reb_no_orders_hit,reb_delta_hedge,reb_unhedged_delta,reb_delta_pnl,reb_total_delta_pnl,reb_option_price_change,reb_portfolio_pnl,reb_total_portfolio_pnl,reb_delta,reb_gamma,reb_theta,reb_xgamma,option_price,threshold,tenor,ccypair)"

ccypairs = ["eurusd","usdjpy","gbpusd"]
tenors = ["1w","1m"]
thresholds = [100,250,500,750,1000,1250,2500,5000]

for ccypair in ccypairs:
    for tenor in tenors:
        cycles_results = LoadCycles(ccypair,tenor)
        for threshold in thresholds:
            for (cycle_id, cycle_start,cycle_end) in cycles_results:
                txt_filename = "../reports/%s_%s_%s/report_%s_%s_%s.txt" % (ccypair,tenor,threshold,cycle_id,str(cycle_start),str(cycle_end))
                if not os.path.exists(txt_filename):
                    print("no ", txt_filename)
                else:
                    txt_df = LoadTxtFile(txt_filename)
                    
                    str_array = []
                    for i in txt_df.index:
                        sql_query = "INSERT INTO rebalancing_data_straddles %s VALUES (%s,\"%s\",\"%s\",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,\"%s\",\"%s\");" % (insert_into_cols, cycle_id, 
                         txt_df.ix[i,"reb_date"], txt_df.ix[i,"reb_time"], txt_df.ix[i,"reb_spot"], txt_df.ix[i,"reb_spot_chg_pips"], 
                         txt_df.ix[i,"reb_step_pips"],txt_df.ix[i,"reb_slippage_pips"],txt_df.ix[i,"reb_no_orders_hit"], 
                         txt_df.ix[i,"reb_delta_hedge"],txt_df.ix[i,"reb_unhedged_delta"],txt_df.ix[i,"reb_delta_pnl"], 
                         txt_df.ix[i,"reb_total_delta_pnl"],txt_df.ix[i,"reb_option_price_change"], 
                         txt_df.ix[i,"reb_portfolio_pnl"],txt_df.ix[i,"reb_total_portfolio_pnl"], 
                         txt_df.ix[i,"reb_delta"],txt_df.ix[i,"reb_gamma"],txt_df.ix[i,"reb_theta"],txt_df.ix[i,"reb_xgamma"],
                        txt_df.ix[i,"option_price"],threshold,tenor,ccypair)

                        str_array.append(sql_query)
                    
                    myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
                    mycursor = myconn.cursor()

                    for str1 in str_array:
                        mycursor.execute(str1)
                    
                    myconn.commit()
                    myconn.close()    


