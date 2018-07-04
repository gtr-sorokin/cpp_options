from datetime import datetime, timedelta
import pymysql
import os.path
import pass_word
import pandas as pd
import numpy as np
from math import sqrt

import matplotlib.pyplot as plt 

#################
#################

test_type = "spread"
ccypairs = ["eurusd","usdjpy","gbpusd"]
tenors = ["1w"]
thresholds = [50,75,100,250,500,750,1000,1250,2500]
opttypes = ["put","call"] # put or call
delta = 25
volspreads = ["m05","m025","00","025","05"]

array_of_hashes = [{'test_type':test_type,'delta':delta,'thresholds':thresholds,'ccypair':ccypair,'tenor':tenor,'opttype':opttype}
                       for opttype in opttypes
                       for tenor in tenors
                       for ccypair in ccypairs]

for myhash in array_of_hashes:

    dfs_sharpe = []
    dfs_std = []
    for volspread in volspreads:
        #daily_25call_eurusd_1w_00.txt
        new_df = pd.read_csv("../daily_returns/sharpe_%s%s_%s_%s_%s.txt" % (myhash['delta'], myhash['opttype'], myhash['ccypair'],myhash['tenor'],volspread),delim_whitespace=True,nrows=6)
        dfs_sharpe.append(new_df)

    for volspread in volspreads:
        new_df = pd.read_csv("../daily_returns/sharpe_%s%s_%s_%s_%s.txt" % (myhash['delta'], myhash['opttype'], myhash['ccypair'],myhash['tenor'],volspread),delim_whitespace=True,nrows=6,skiprows=8)
        dfs_std.append(new_df)

    pngfile = "../daily_returns/spreads_%s_%s%s_%s.png" % (myhash['ccypair'], myhash['delta'], myhash['opttype'], myhash['tenor'])

    colours = ['b','g','c','y','r']
    volspreads_l = ["-0.5","-0.25","0.0","+0.25","+0.5"]
    
    s_fig = plt.figure()
    for i in range(0,len(dfs_sharpe)):
        plt.scatter(dfs_std[i].ix[["Thu","Fri"],["1000","1250"]],dfs_sharpe[i].ix[["Thu","Fri"],["1000","1250"]],label=volspreads_l[i],color=colours[i])

    plt.legend()
    plt.title("volspread for %s %s %s %s" % (myhash['delta'], myhash['opttype'], myhash['ccypair'],myhash['tenor']))
    plt.axis([0,2.5,-2.5,3.5])
    plt.savefig(pngfile)
    plt.close(s_fig)


