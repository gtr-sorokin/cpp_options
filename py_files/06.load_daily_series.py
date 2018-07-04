from datetime import datetime, timedelta
import pymysql
import os.path
import pass_word
import pandas as pd
import numpy as np
from math import sqrt

def GenerateFileNames(reportdir, myhash):
    if(myhash["test_type"]=="straddle"):
        reportfile = reportdir+r"/daily_%s_%s.txt" % (myhash["ccypair"],myhash["tenor"])
        sharpefile = reportdir+r"/sharpe_%s_%s.txt" % (myhash["ccypair"],myhash["tenor"])

    elif(myhash["test_type"]=="spread"):
        reportfile = reportdir+r"/daily_%s%s_%s_%s_%s.txt" % (myhash["delta"],myhash["opttype"],myhash["ccypair"],myhash["tenor"],myhash["volspread"])
        sharpefile = reportdir+r"/sharpe_%s%s_%s_%s_%s.txt" % (myhash["delta"],myhash["opttype"],myhash["ccypair"],myhash["tenor"],myhash["volspread"])

    return (reportfile,sharpefile)
        
def LoadIndexValues(myhash):

    if(myhash["test_type"]=="straddle"):
        mysql_order01 = r"select distinct(reb_date) from %s where ccypair='%s' and tenor='%s' order by reb_date" % (myhash["table_name"],myhash["ccypair"],myhash["tenor"])
    else:
        mysql_order01 = r"select distinct(reb_date) from %s where ccypair='%s' and tenor='%s' and opttype='%s' and volspread='%s' order by reb_date" % (myhash["table_name"],myhash["ccypair"],myhash["tenor"],myhash["opttype"],myhash["volspread"])

    myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
    mycursor = myconn.cursor()
    mycursor.execute(mysql_order01)
    myconn.commit()
    results = mycursor.fetchall()
    myconn.close()

    index_values = [res[0] for res in results]
    return index_values


def SaveResults(sharpefile, sharpe_df, sharpe_of_sharpes, yearly_sharpe_df, years):

    sharpe_df = sharpe_df.round(4)
    sharpe_of_sharpes = sharpe_of_sharpes.round(4)

    for j in range(0,len(yearly_sharpe_df)):
        yearly_sharpe_df[j] = yearly_sharpe_df[j].round(4)

    reportf = open(sharpefile,'w')
    reportf.write(sharpe_df.to_string())

    reportf.write("\n\nsharpe of sharpes\n")
    reportf.write(sharpe_of_sharpes.to_string())

    for j in range(0,len(yearly_sharpe_df)):
        reportf.write("\n\n"+str(years[j])+" to "+str(years[j+1])+"\n")
        reportf.write(yearly_sharpe_df[j].to_string())

    reportf.close()


def PresetVariablesAndArrays(myhash):

    divider = 20
    if(myhash["tenor"]=="1w"):
        weekdaysL = ["Mon", "Tue", "Wed", "Thu", "Fri"]
        weekdays = range(2,2+len(weekdaysL))
    else:
        if(myhash["tenor"]=="1m"): divider = 20
        elif(myhash["tenor"]=="3m"): divider = 60
        weekdays = range(0,divider)
        weekdaysL = [str(w)+"s" for w in weekdays]

    column_names = [weekdayL+str(threshold) for threshold in thresholds for weekdayL in weekdaysL]
    column_tnames = ["t"+weekdayL+str(threshold) for threshold in thresholds for weekdayL in weekdaysL]

    sharpe_df = pd.DataFrame(0.0,columns=thresholds,index=weekdaysL)
    sharpe_of_sharpes = pd.DataFrame(0.0,columns=thresholds,index=weekdaysL)

    return (divider, weekdays, weekdaysL, column_names, column_tnames, sharpe_df, sharpe_of_sharpes)
    

def ReturnSharpeOfSharpes(array):
    average = 0
    for i in array:
        average+=i
    average = average/len(array)

    std = 0
    for i in array:
        std+=(i-average)*(i-average)
    std = sqrt(std/(len(array)-1))

    return average/std

def GetMysqlOrder02(myhash,threshold,weekday):

    if(myhash["test_type"]=="straddle"):    
        if(myhash["tenor"]=="1w"):
            mysql_order02 = r"select a.reb_date, a.reb_portfolio_pnl from %s a, %s_%s_cycles b where a.ccypair='%s' and a.tenor='%s' and a.threshold=%s and a.reb_cycle_id = b.cycle_id and dayofweek(b.cycle_start)=%s order by a.reb_date" % (myhash["table_name"],myhash["ccypair"],myhash["tenor"],myhash["ccypair"],myhash["tenor"],threshold,weekday)
        else: #1m or 3m
            mysql_order02 = r"select reb_date, reb_portfolio_pnl from %s where ccypair='%s' and tenor='%s' and threshold=%s and reb_cycle_id %s order by reb_date" % (myhash["table_name"],myhash["ccypair"],myhash["tenor"],threshold,"%"+str(myhash["divider"])+"="+str(weekday))
    else: #spread
        if(myhash["tenor"]=="1w"):
            mysql_order02 = r"select a.reb_date, a.reb_portfolio_pnl from %s a, %s_%s_cycles b where a.ccypair='%s' and a.tenor='%s' and a.opttype='%s' and a.volspread='%s' and a.threshold=%s and a.reb_cycle_id = b.cycle_id and dayofweek(b.cycle_start)=%s order by a.reb_date" % (myhash["table_name"],myhash["ccypair"],myhash["tenor"],myhash["ccypair"],myhash["tenor"],myhash["opttype"],myhash["volspread"],threshold,weekday)
        else: #1m or 3m
            mysql_order02 = r"select reb_date, reb_portfolio_pnl from %s where ccypair='%s' and tenor='%s' and opttype='%s' and volspread='%s' and threshold=%s and reb_cycle_id %s order by reb_date" % (myhash["table_name"],myhash["ccypair"],myhash["tenor"],myhash["opttype"],myhash["volspread"],threshold,"%"+str(myhash["divider"])+"="+str(weekday))

    return mysql_order02

####################################
####################################
####################################
####################################

test_type = "spread" # "spread" or "straddle"

table_name = "rebalancing_data_spreads" if (test_type=="spread") else "rebalancing_data_straddles"

if(test_type == "straddle"):
    ccypairs = ["eurusd","usdjpy","gbpusd"]
    tenors =   ["1w","1m"]
    #tenors = ["3m"]	    
    thresholds = [100,250,500,750,1000,1250,2500,5000]

    # ccypairs = ["eurusd","usdjpy"]
    # tenors =   ["1w","1m","3m"]
    # thresholds = [100,250,500,750,1000,1250,2500,5000]


else:
    ccypairs = ["eurusd"] 
    tenors = ["1w"] 
    thresholds = [50,75,100,250,500,750,1000,1250,2500]

delta = 25
opttypes = ["put","call"]
volspreads = ["m05","m025","00","025","05"]
#volspreads = ["00"]

years = range(2010,2018)
dates = [datetime(i,1,1).date() for i in years]

reportdir = "../daily_returns";
if not os.path.exists(reportdir):
    os.makedirs(reportdir)

array_of_hashes = []

if(test_type=="spread"):
    array_of_hashes = [{"test_type":test_type,"table_name":table_name,"opttype":opttype,"volspread":volspread,"tenor":tenor,"ccypair":ccypair,"delta": delta}
                       for tenor in tenors 
                       for ccypair in ccypairs 
                       for opttype in opttypes 
                       for volspread in volspreads]
else:
    array_of_hashes = [{"test_type":test_type,"table_name":table_name,"tenor":tenor,"ccypair":ccypair}
                       for tenor in tenors 
                       for ccypair in ccypairs]

for myhash in array_of_hashes:
    (myhash["divider"], weekdays, weekdaysL, column_names, column_tnames, sharpe_df, sharpe_of_sharpes_df) = PresetVariablesAndArrays(myhash)
    yearly_sharpe_df = [pd.DataFrame(0.0,columns=thresholds,index=weekdaysL) for i in range(0,len(dates)-1)]

    (reportfile,sharpefile) = GenerateFileNames(reportdir,myhash)

    if(not os.path.exists(reportfile)):
        index_values = LoadIndexValues(myhash)
        results_df = pd.DataFrame(0.0,columns=column_names+column_tnames,index=index_values)

        for threshold in thresholds:
            for i in range(0,len(weekdays)):
                weekday = weekdays[i]
                weekdayL = weekdaysL[i]

                print("analyzing: ",myhash["ccypair"],myhash["tenor"],threshold,weekday)
                mysql_order02 = GetMysqlOrder02(myhash,threshold,weekday)

                myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
                mycursor = myconn.cursor()
                mycursor.execute(mysql_order02)
                myconn.commit()
                results = mycursor.fetchall()
                myconn.close()
                print("total volume: ",len(results))

                for (my_date,my_pnl) in results:
                    results_df.ix[my_date,weekdayL+str(threshold)]+=my_pnl

        for col in column_names:
            results_df["t"+col]=results_df[col].cumsum()

        reportf = open(reportfile,'w')
        reportf.write(results_df.to_string())
        reportf.close()

    else:
        print("reading ",reportfile)
        results_df = pd.read_csv(reportfile,parse_dates=True,infer_datetime_format=True,delim_whitespace=True)

    for threshold in thresholds:
        for weekdayL in weekdaysL:
            sharpe_df.ix[weekdayL,threshold]=sqrt(250)*results_df[weekdayL+str(threshold)].mean()/results_df[weekdayL+str(threshold)].std()
            array_of_sharpes = []
            for j in range(0,len(yearly_sharpe_df)):
                yearly_sharpe_df[j].ix[weekdayL,threshold]=sqrt(250)*results_df.ix[dates[j]:dates[j+1],weekdayL+str(threshold)].mean()/results_df.ix[dates[j]:dates[j+1],weekdayL+str(threshold)].std()
                array_of_sharpes.append(yearly_sharpe_df[j].ix[weekdayL,threshold])

            sharpe_of_sharpes_df.ix[weekdayL,threshold] = ReturnSharpeOfSharpes(array_of_sharpes)

    SaveResults(sharpefile, sharpe_df, sharpe_of_sharpes_df, yearly_sharpe_df, years)

