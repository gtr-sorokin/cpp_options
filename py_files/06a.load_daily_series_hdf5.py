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

def ReturnSharpeOfSharpes(array,ifstdev=False):
    average = 0
    for i in array:
        average+=i
    average = average/len(array)

    std = 0
    for i in array:
        std+=(i-average)*(i-average)
    std = sqrt(std/(len(array)-1))

    if(not ifstdev):
        return average/std
    else:
        return std

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

def GenerateFileNames(reportdir, myhash):
    if(myhash["test_type"]=="straddle"):
        reportfile = reportdir+r"/daily_%s_%s.txt" % (myhash["ccypair"],myhash["tenor"])
        sharpefile = reportdir+r"/sharpe_%s_%s.txt" % (myhash["ccypair"],myhash["tenor"])

    elif(myhash["test_type"]=="spread"):
        reportfile = reportdir+r"/daily_%s%s_%s_%s_%s.txt" % (myhash["delta"],myhash["opttype"],myhash["ccypair"],myhash["tenor"],myhash["volspread"])
        sharpefile = reportdir+r"/sharpe_%s%s_%s_%s_%s.txt" % (myhash["delta"],myhash["opttype"],myhash["ccypair"],myhash["tenor"],myhash["volspread"])

    return (reportfile,sharpefile)

def LoadHDF(myhash,threshold):
    database = "%s_%s_%s" % (myhash['ccypair'],myhash['tenor'],threshold)

    if(myhash['test_type']=='straddle'):
        h5_file = "../hdf5s/%s_%s.h5" % (database,myhash['test_type'])
    else:
        h5_file = "../hdf5s/%s_%s_%s%s_%s.h5" % (database,myhash['test_type'],myhash['delta'],myhash['opttype'],myhash['volspread'])

    results_df = pd.read_hdf(h5_file,database)
    return results_df

def LoadIndexValues(myhash):
    results_df = LoadHDF(myhash,myhash['thresholds'][0])
    index_values = results_df["reb_date"].unique()
    del results_df
    return index_values

def LoadCycleIDs(myhash,weekday):

    if(myhash["tenor"]=="1w"):
        mysql_order = "select cycle_id from %s_%s_cycles where dayofweek(cycle_start)=%s;" % (myhash["ccypair"],myhash["tenor"],weekday)
    else: #1m or 3m
        mysql_order = "select cycle_id from %s_%s_cycles where cycle_id %s;" % (myhash["ccypair"],myhash["tenor"],"%"+str(myhash["divider"])+"="+str(weekday))
        #"%"+str(myhash["divider"])+"="+str(weekday)

    myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
    mycursor = myconn.cursor()
    mycursor.execute(mysql_order)
    myconn.commit()
    results = mycursor.fetchall()
    myconn.close()

    return [res[0] for res in results]

def SaveResults(sharpefile, sharpe_df, sharpe_of_sharpes, yearly_sharpe_df, years, mystring):

    sharpe_df = sharpe_df.round(4)
    sharpe_of_sharpes = sharpe_of_sharpes.round(4)

    for j in range(0,len(yearly_sharpe_df)):
        yearly_sharpe_df[j] = yearly_sharpe_df[j].round(4)

    reportf = open(sharpefile,'w')
    reportf.write(sharpe_df.to_string())

    reportf.write("\n\n"+mystring+"\n")
    reportf.write(sharpe_of_sharpes.to_string())

    for j in range(0,len(yearly_sharpe_df)):
        reportf.write("\n\n"+str(years[j])+" to "+str(years[j+1])+"\n")
        reportf.write(yearly_sharpe_df[j].to_string())

    reportf.close()

################
################

test_type = "spread" # straddle or spread
array_of_hashes = []

if(test_type=="straddle"):
    ccypairs = ["eurusd","usdjpy","gbpusd"]
    tenors = ["1w","1m"]

    #ccypairs = ["eurusd"]
    #tenors = ["1w"]
    thresholds = [100,250,500,750,1000,1250,2500,5000]

    array_of_hashes = [{'test_type':test_type,'thresholds':thresholds,'ccypair':ccypair,'tenor':tenor} 
                       for tenor in tenors
                       for ccypair in ccypairs]

else: #spread
    #ccypairs = ["eurusd","gbpusd"]
    ccypairs = ["eurusd","usdjpy","gbpusd"]
    tenors = ["1w"]
    thresholds = [50,75,100,250,500,750,1000,1250,2500]
    opttypes = ["put","call"] # put or call
    delta = 25
    volspreads = ["m05","m025","00","025","05"]

    array_of_hashes = [{'test_type':test_type,'delta':delta,'thresholds':thresholds,'ccypair':ccypair,'tenor':tenor,'opttype':opttype,'volspread':volspread}
                       for volspread in volspreads
                       for opttype in opttypes
                       for tenor in tenors
                       for ccypair in ccypairs]

years = range(2010,2018)
dates = [datetime(i,1,1).date() for i in years]

reportdir = "../daily_returns";
if not os.path.exists(reportdir):
    os.makedirs(reportdir)

for myhash in array_of_hashes:
    (myhash["divider"], weekdays, weekdaysL, column_names, column_tnames, sharpe_df, sharpe_of_sharpes_df) = PresetVariablesAndArrays(myhash)
    yearly_sharpe_df = [pd.DataFrame(0.0,columns=thresholds,index=weekdaysL) for i in range(0,len(dates)-1)]

    (reportfile,sharpefile) = GenerateFileNames(reportdir,myhash)

    print(myhash['ccypair'],myhash['tenor'])

    if(not os.path.exists(reportfile)):
        index_values = LoadIndexValues(myhash)
        results_df = pd.DataFrame(0.0,columns=column_names+column_tnames,index=index_values)

        for threshold in thresholds:
            h5_df = LoadHDF(myhash,threshold)
            print("\tmain dataframe length: ",h5_df["reb_date"].count()," threshold = ",threshold)

            for i in range(0,len(weekdays)):
                weekday = weekdays[i]
                weekdayL = weekdaysL[i]
                cycle_ids = LoadCycleIDs(myhash,weekday)
                subh5_df = h5_df[h5_df["reb_cycle_id"].isin(cycle_ids)]
                print("\t\tsub dataframe length: ",subh5_df["reb_date"].count(), " weekday = ", weekdayL)

                reb_dates = subh5_df["reb_date"].unique()

                for reb_date in reb_dates:
                    my_pnl = subh5_df.ix[subh5_df["reb_date"]==reb_date,"reb_portfolio_pnl"].sum()

                    if(reb_date not in results_df.index):
                        results_df.loc[reb_date]=0.0
                        results_df = results_df.sort_index()

                    results_df.ix[reb_date,weekdayL+str(threshold)]+=my_pnl
                        
                del reb_dates
                del subh5_df
                
            del h5_df

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
                yearly_sharpe_df[j].ix[weekdayL,threshold]=sqrt(250)*results_df.ix[(results_df.index>=dates[j].strftime("%Y-%m-%d"))&(results_df.index<dates[j+1].strftime("%Y-%m-%d")),weekdayL+str(threshold)].mean()/results_df.ix[(results_df.index>=dates[j].strftime("%Y-%m-%d"))&(results_df.index<dates[j+1].strftime("%Y-%m-%d")),weekdayL+str(threshold)].std()
                array_of_sharpes.append(yearly_sharpe_df[j].ix[weekdayL,threshold])

            sharpe_of_sharpes_df.ix[weekdayL,threshold] = ReturnSharpeOfSharpes(array_of_sharpes,True)

    SaveResults(sharpefile, sharpe_df, sharpe_of_sharpes_df, yearly_sharpe_df, years,"stdev")
    
    ####

    pngfile = reportfile + ".png"
    s_fig = plt.figure()
    color_count = 0

    if(myhash['tenor']=="1w"):
        colors = ['b','b','b','r','y']
        for i in sharpe_df.index:
            plt.scatter(sharpe_of_sharpes_df.ix[i,:],sharpe_df.ix[i,:],label="%s" % (i),color=colors[color_count])
            color_count+=1
    else:
        #thresholds = [100,250,500,750,1000,1250,2500,5000]
        colors = ['b','b','b','g','y','r','c','m']
        for i in sharpe_df.columns:
            plt.scatter(sharpe_of_sharpes_df.ix[:,i],sharpe_df.ix[:,i],label="%s" % (i),color=colors[color_count])
            color_count+=1

    plt.title("%s_%s" % (myhash['ccypair'],myhash['tenor']))
    plt.legend(prop={'size':8})
    #plt.show()
    plt.savefig(pngfile)
    plt.close(s_fig)

        # for i in range(0,len(_weekdays)):
        # s_fig = plt.figure()
        # _weekdayL = _weekdaysL[i]
        # counter = 0

        # for _threshold in _thresholds:
        #     counter+=1
        #     arr = []

        #     for k in range(0,len(years)):
        #         arr.append(years_summary_tabs[k].get_value(_threshold,_weekdayL))

        #     plt.subplot(3,3,counter)
        #     plt.plot(years,arr,label="%s" % (_weekdayL+r":"+str(_threshold)))
        #     plt.xticks(years,years, fontsize = 9)
        #     plt.yticks([0,1.0,2.0,3.0,4.0,5.0], fontsize = 9)
        #     plt.legend(prop={'size':8})

        # #plt.show()
        # plt.savefig(pathtographs+_weekdayL+".png")
        # plt.close(s_fig)



