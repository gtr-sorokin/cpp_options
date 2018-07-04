import zipfile
import os
import pymysql
from datetime import datetime
import time
import io
import pass_word

from pytz import timezone
import pytz

def ConvertTwoStringsIntoDateTime(str1,str2):
   return datetime.strptime(str1+" "+str2,"%Y-%m-%d %H:%M:%S")

def LoadLastQuoteFromDB(ccypair):
   myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
   mycursor = myconn.cursor()
   mycursor.execute("SELECT utc_quotedate, utc_quotetime from %s_new1 order by rowid desc limit 1" % (ccypair))
   result_array = mycursor.fetchone();
   myconn.commit()
   myconn.close()

   #utc_datetime = datetime.strptime((str(result_array[0])+" "+str(result_array[1])),"%Y-%m-%d %H:%M:%S")
   #utc_tz = timezone('UTC')
   #utc_datetime = utc_tz.localize(mydatetime)

   if((result_array!=None) and (result_array[0]!=None) and (result_array[1]!=None)):
      return ConvertTwoStringsIntoDateTime(str(result_array[0]),str(result_array[1]))
   else:
      return ConvertTwoStringsIntoDateTime(str("1900-01-01"),str("10:00:00"))

def ParseString(mystr):
   mystr=mystr.replace('\n','')
   mystr=mystr.replace('\r','')
   myarray = mystr.split(',')

   #2009-12-31 00:00:34.125,1.4338,1.4337,1.5,1.9

   mydatetimems = myarray[0]
   myoffer = myarray[1]
   mybid = myarray[2]
   myoffervol = myarray[3]
   mybidvol = myarray[4]

   (mydate,myms) = mydatetimems.split(".")
   myms = float("0."+myms)

   utc_tz = timezone('UTC')
   utc_datetime = utc_tz.localize(datetime.strptime(mydate,"%Y-%m-%d %H:%M:%S"))
   eastern_tz = timezone('US/Eastern')
   eastern_datetime = utc_datetime.astimezone(eastern_tz)

   return (utc_datetime.strftime("%Y-%m-%d"),utc_datetime.strftime("%H:%M:%S"),eastern_datetime.strftime("%Y-%m-%d"),eastern_datetime.strftime("%H:%M:%S"),myms,mybid,myoffer,mybidvol,myoffervol)


tofolder = r"../dukascopy_data/"

#EURUSD
#files = ["EURUSD_UTC_Ticks_Bid_2016.01.01_2017.04.12.csv","EURUSD_UTC_Ticks_Bid_2017.04.01_2017.04.27.csv"]

#GBPUSD
#files = ["GBPUSD_2009_2012aa","GBPUSD_2009_2012ab","GBPUSD_UTC_Ticks_Bid_2012.04.01_2015.01.31.csv","GBPUSD_2015_2017_aa","GBPUSD_2015_2017_ab","GBPUSD_2015_2017_ac","GBPUSD_2015_2017_ad"]

#USDJPY
files = ["USDJPY_UTC_Ticks_Bid_2008.12.31_2011.01.10.csv","USDJPY_UTC_Ticks_Bid_2011.01.01_2013.01.07.csv","USDJPY_UTC_Ticks_Bid_2013.01.01_2015.01.05.csv","USDJPY_UTC_Ticks_Bid_2015_2017_aa","USDJPY_UTC_Ticks_Bid_2015_2017_ab","USDJPY_UTC_Ticks_Bid_2017.04.01_2017.04.27.csv"]

##files = ["USDJPY_UTC_Ticks_Bid_2015.01.01_2017.04.13.csv"]
##files = ["USDJPY_UTC_Ticks_Bid_2017.04.01_2017.04.27.csv"]

for filename in files:

   ccypair = filename[0:6].lower();
   print("ccypair ",ccypair)

   utc_datetime = LoadLastQuoteFromDB(ccypair.lower())

   print("loading ", filename)
   with open(tofolder+filename,'r') as volf:
      mystringsarray = volf.readlines()
      volf.closed
   print("finished loading ",filename)

   counter = 0
   if_skip_quotes = True

   mysql_orders = []
   range_start = 0

   if(mystringsarray[0].find("Time")!=-1):
      range_start=1

   print("starting parsing the file from ",range_start," line")

   for i in range(range_start,len(mystringsarray)):
      counter+=1

      myline = mystringsarray[i]
      #myline = myline.replace("\n","")

      (utc_date,utc_time,eastern_date,eastern_time,myms,mybid,myoffer,mybidvol,myoffervol) = ParseString(myline)

      if(if_skip_quotes):
         current_utc_datetime = ConvertTwoStringsIntoDateTime(str(utc_date),str(utc_time))
         if(utc_datetime<current_utc_datetime):
            if_skip_quotes=False

      if(not if_skip_quotes):

         mysql_orders.append(r"insert into %s_new1 (quotedate, quotetime, quotems, quotebid, quoteoffer, utc_quotedate, utc_quotetime,bidvolume,offervolume) VALUES ('%s','%s',%s,%s,%s,'%s','%s',%s,%s)" % (ccypair.lower(),eastern_date,eastern_time,myms,mybid,myoffer,utc_date,utc_time,mybidvol,myoffervol))

   del mystringsarray

   print("total rows in file ",counter)
   print("total rows to add  ",len(mysql_orders))

   print("starting mysql procedures ")

   myconn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
   mycursor = myconn.cursor()
   print("connected to mysql")

   for mysql_order in mysql_orders:
      mycursor.execute(mysql_order)

   print("loaded execute instructions successfully")
   myconn.commit()
   myconn.close()

   del mysql_orders

