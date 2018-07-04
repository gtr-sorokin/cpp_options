from datetime import datetime, timedelta
import pymysql
import os.path
import pass_word
import pandas as pd
import numpy as np
from math import sqrt

import matplotlib.pyplot as plt

ccypairs = ["eurusd","gbpusd","usdjpy"]
tenors = ["1w","1m"]
threshold = 1250

columns_str = "cycle_id cycle_start_date cycle_start_time cycle_end_date cycle_end_time strike spot_start spot_end vol num_quotes_mm num_rebalancings stop_orders market_orders secondary_orders total_orders initial_price final_price delta_pnl pnl residual C1_traded_notional vega_pnl true_vol"
my_columns = columns_str.split(" ")

for tenor in tenors:

    dataframes = []
    for ccypair in ccypairs:
        new_df = pd.read_csv("../reports/%s_%s_%s/01.summary.txt" % (ccypair, tenor, threshold),infer_datetime_format=True,delim_whitespace=True,names=my_columns,skiprows=2)
        new_df = new_df[new_df['cycle_start_date']>="2010-01-01"] #datetime(2010,1,1)
        dataframes.append(new_df)

    print("breakeven vol (average,stdev,25-75q range): ")
    for i in range(0,len(dataframes)):
        print(ccypairs[i],tenor,round(dataframes[i]["true_vol"].mean(),2),round(dataframes[i]["true_vol"].std(),2),round(dataframes[i]["true_vol"].quantile(0.25),2),round(dataframes[i]["true_vol"].quantile(0.75),2))

    print("market vol (average,stdev, 25-75q range): ")
    for i in range(0,len(dataframes)):
        print(ccypairs[i],tenor,round(dataframes[i]["vol"].mean(),2),round(dataframes[i]["true_vol"].std(),2),round(dataframes[i]["vol"].quantile(0.25),2),round(dataframes[i]["vol"].quantile(0.75),2))


    pngfile = "breakeven_vols_%s_%s.png" % (tenor,threshold)
    s_fig = plt.figure()
    for i in range(0,len(dataframes)):
        dataframes[i]["true_vol"].plot(kind='density',label=ccypairs[i])

    plt.legend()
    plt.title("breakeven vols for (%s,%s)" % (tenor,threshold))
    plt.axis([0,25,0,0.15])
    plt.savefig(pngfile)
    plt.close(s_fig)


    pngfile = "market_vols_%s_%s.png" % (tenor,threshold)
    s_fig = plt.figure()
    for i in range(0,len(dataframes)):
        dataframes[i]["vol"].plot(kind='density',label=ccypairs[i])

    plt.legend()
    plt.title("market vols for (%s,%s)" % (tenor,threshold))
    plt.axis([0,25,0,0.20])
    plt.savefig(pngfile)
    plt.close(s_fig)

