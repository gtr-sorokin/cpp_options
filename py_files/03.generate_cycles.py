from datetime import datetime, timedelta
import calendar
import pymysql
import pass_word

def IsHoliday(date,holidays_arr):
   bIsHoliday=False
   for _day in holidays_arr:
      if date==_day:
         bIsHoliday=True
         break
   return bIsHoliday

def CondOperator(condition,a,b):
   if(condition==True):
      return a
   else:
      return b

def GetEndDate(startdate,tenor):
   num_of_months = 1
   if(tenor=="3m"):
      num_of_months=3

   my_year = CondOperator(startdate.month<=12-num_of_months,startdate.year,startdate.year+1)
   my_month = CondOperator(startdate.month<=12-num_of_months,startdate.month+num_of_months,num_of_months-(12-startdate.month))
   lastday = calendar.monthrange(my_year, my_month)[1]

   if(startdate.day<=lastday):
      enddate = datetime(CondOperator(startdate.month<=12-num_of_months,startdate.year,startdate.year+1), CondOperator(startdate.month<=12-num_of_months,startdate.month+num_of_months,num_of_months-(12-startdate.month)),startdate.day)
   else:
      enddate = datetime(CondOperator(startdate.month<=12-num_of_months,startdate.year,startdate.year+1), CondOperator(startdate.month<=12-num_of_months,startdate.month+num_of_months,num_of_months-(12-startdate.month)),lastday)

   return enddate

def GetEndDate1W(startdate):

   lastday = calendar.monthrange(startdate.year, startdate.month)[1]

   if(startdate.day+7<=lastday):
      enddate = datetime(startdate.year,startdate.month,startdate.day+7)
   elif(startdate.month<12):
      enddate = datetime(startdate.year,startdate.month+1,7-(lastday-startdate.day))
   else:
      enddate = datetime(startdate.year+1,1,7-(lastday-startdate.day))

   return enddate

def GetNumberOfDays(start,end):
    return (int(end.strftime("%j")) - int(start.strftime("%j")) + (int(end.year)-int(start.year))*365)

JPY_holidays_str = r"2010-01-01,2010-01-11,2010-02-11,2010-03-22,2010-04-29,2010-05-03,2010-05-04,2010-05-05,2010-07-19,2010-09-20,2010-09-23,2010-10-11,2010-11-03,2010-11-23,2010-12-23,2010-12-31,2011-01-01,2011-01-02,2011-01-03,2011-01-10,2011-02-11,2011-03-21,2011-04-29,2011-05-03,2011-05-04,2011-05-05,2011-07-18,2011-09-19,2011-09-23,2011-10-10,2011-11-03,2011-11-23,2011-12-23,2011-12-31,2012-01-02,2012-01-03,2012-01-09,2012-02-11,2012-03-20,2012-04-30,2012-05-03,2012-05-04,2012-05-05,2012-07-16,2012-09-17,2012-09-22,2012-10-08,2012-11-03,2012-11-23,2012-12-24,2012-12-31,2013-01-01,2013-01-02,2013-01-03,2013-01-14,2013-02-11,2013-03-20,2013-04-29,2013-05-03,2013-05-04,2013-05-05,2013-05-06,2013-07-15,2013-09-16,2013-09-23,2013-10-14,2013-11-03,2013-11-04,2013-11-23,2013-12-23,2013-12-31,2014-01-01,2014-01-02,2014-01-03,2014-01-13,2014-02-11,2014-03-21,2014-04-29,2014-05-03,2014-05-04,2014-05-05,2014-05-06,2014-07-21,2014-09-15,2014-09-23,2014-10-13,2014-11-03,2014-11-23,2014-11-24,2014-12-23,2014-12-31,2015-01-01,2015-01-02,2015-01-03,2015-01-12,2015-02-11,2015-03-21,2015-04-29,2015-05-03,2015-05-04,2015-05-05,2015-05-06,2015-07-20,2015-09-21,2015-09-22,2015-09-23,2015-10-12,2015-11-03,2015-11-23,2015-12-23,2015-12-31,2016-01-01,2016-01-02,2016-01-03,2016-01-11,2016-02-11,2016-03-20,2016-03-21,2016-04-29,2016-05-03,2016-05-04,2016-05-05,2016-07-18,2016-08-11,2016-09-19,2016-09-22,2016-10-10,2016-11-03,2016-11-23,2016-12-23,2016-12-31,2017-01-01,2017-01-02,2017-01-03,2017-01-09,2017-02-11,2017-03-20,2017-04-29,2017-05-03,2017-05-04,2017-05-05,2017-07-17,2017-08-11,2017-09-18,2017-09-23,2017-10-09,2017-11-03,2017-11-23,2017-12-23,2017-12-31"

USD_holidays_str = r"2010-01-01,2010-01-18,2010-02-15,2010-03-14,2010-04-02,2010-05-31,2010-07-04,2010-07-05,2010-09-06,2010-10-11,2010-11-07,2010-11-11,2010-11-25,2010-12-24,2010-12-25,2011-01-01,2011-01-17,2011-02-21,2011-03-13,2011-04-22,2011-05-30,2011-07-04,2011-09-05,2011-10-10,2011-11-06,2011-11-11,2011-11-24,2011-12-25,2011-12-26,2012-01-01,2012-01-02,2012-01-16,2012-02-20,2012-03-11,2012-04-06,2012-05-28,2012-07-04,2012-09-03,2012-10-08,2012-11-04,2012-11-12,2012-11-22,2012-12-25,2013-01-01,2013-01-21,2013-02-18,2013-03-10,2013-03-29,2013-05-27,2013-07-04,2013-09-02,2013-10-14,2013-11-03,2013-11-11,2013-11-28,2013-12-25,2014-01-01,2014-01-20,2014-02-17,2014-03-09,2014-04-18,2014-05-26,2014-07-04,2014-09-01,2014-10-13,2014-11-02,2014-11-11,2014-11-27,2014-12-25,2015-01-01,2015-01-19,2015-02-16,2015-03-08,2015-04-03,2015-05-25,2015-07-03,2015-07-04,2015-09-07,2015-10-12,2015-11-01,2015-11-11,2015-11-26,2015-12-25,2016-01-01,2016-01-18,2016-02-15,2016-03-13,2016-03-25,2016-05-30,2016-07-04,2016-09-05,2016-10-10,2016-11-06,2016-11-11,2016-11-24,2016-12-25,2016-12-26,2017-01-01,2017-01-02,2017-01-16,2017-02-20,2017-03-12,2017-04-14,2017-05-29,2017-07-04,2017-09-04,2017-10-09,2017-11-05,2017-11-11,2017-11-23,2017-12-25"

GBP_holidays_str = r"2010-01-01,2010-03-28,2010-04-02,2010-04-05,2010-05-03,2010-05-31,2010-08-30,2010-10-31,2010-12-27,2010-12-28,2011-01-03,2011-03-27,2011-04-22,2011-04-25,2011-04-29,2011-05-02,2011-05-30,2011-08-29,2011-10-30,2011-12-26,2011-12-27,2012-01-02,2012-03-25,2012-04-06,2012-04-09,2012-05-07,2012-06-04,2012-06-05,2012-08-27,2012-10-28,2012-12-25,2012-12-26,2013-01-01,2013-03-29,2013-03-31,2013-04-01,2013-05-06,2013-05-27,2013-08-26,2013-10-27,2013-12-25,2013-12-26,2014-01-01,2014-03-30,2014-04-18,2014-04-21,2014-05-05,2014-05-26,2014-08-25,2014-10-26,2014-12-25,2014-12-26,2015-01-01,2015-03-29,2015-04-03,2015-04-06,2015-05-04,2015-05-25,2015-08-31,2015-10-25,2015-12-25,2015-12-28,2016-01-01,2016-03-25,2016-03-27,2016-03-28,2016-05-02,2016-05-30,2016-08-29,2016-10-30,2016-12-26,2016-12-27,2017-01-02,2017-03-26,2017-04-14,2017-04-17,2017-05-01,2017-05-29,2017-08-28,2017-10-29,2017-12-25,2017-12-26"

EUR_holidays_str = r"2010-01-01,2010-03-28,2010-04-02,2010-04-05,2010-05-01,2010-10-31,2010-12-25,2010-12-26,2011-01-01,2011-03-27,2011-04-22,2011-04-25,2011-05-01,2011-10-30,2011-12-25,2011-12-26,2012-01-01,2012-03-25,2012-04-06,2012-04-09,2012-05-01,2012-10-28,2012-12-25,2012-12-26,2013-01-01,2013-03-29,2013-03-31,2013-04-01,2013-05-01,2013-10-27,2013-12-25,2013-12-26,2014-01-01,2014-03-30,2014-04-18,2014-04-21,2014-05-01,2014-10-26,2014-12-25,2014-12-26,2015-01-01,2015-03-29,2015-04-03,2015-04-06,2015-05-01,2015-10-25,2015-12-25,2015-12-26,2016-01-01,2016-03-25,2016-03-27,2016-03-28,2016-05-01,2016-10-30,2016-12-25,2016-12-26,2017-01-01,2017-03-26,2017-04-14,2017-04-17,2017-05-01,2017-10-29,2017-12-25,2017-12-26"

_tenors = ["1w","1m","3m"]
_ccypair = "eurusd"

if _ccypair=="usdjpy":
   holidays_1 = USD_holidays_str.split(",")+JPY_holidays_str.split(",")
elif _ccypair=="eurusd":
   holidays_1 = USD_holidays_str.split(",")+EUR_holidays_str.split(",")
elif _ccypair=="gbpusd":
   holidays_1 = USD_holidays_str.split(",")+GBP_holidays_str.split(",")

holidays = tuple([datetime.strptime(_text,"%Y-%m-%d") for _text in holidays_1])

conn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
cur = conn.cursor()

for _tenor in _tenors:

   cur.execute("SELECT max(cycle_start) from %s_%s_cycles" % (_ccypair,_tenor))
   results=cur.fetchone()

   if (results[0]!=None):
      startdate = datetime.strptime(str(results[0]),"%Y-%m-%d")
      startdate = startdate+timedelta(1)
   else:
      if(_ccypair=="eurusd"):
         startdate = datetime(2009,5,5)
      else:
         startdate=datetime(2010,1,1)
      

   print("start date ",startdate)

   cur.execute("SELECT max(quotedate) from %s_new1" % (_ccypair))
   results=cur.fetchone()
   enddate = datetime.strptime(str(results[0]),"%Y-%m-%d")

   cur.execute("SELECT max(cycle_id) from %s_%s_cycles" % (_ccypair,_tenor))
   results=cur.fetchone()
   
   if (results[0]!=None):
      cycle_id = int(str(results[0]))+1
   else:
      cycle_id = 1

   tempstart = startdate
   tempdate = startdate

   if(_tenor == "1w"):
      tempend = GetEndDate1W(tempstart)
   else:
      tempend = GetEndDate(tempstart,_tenor)
      
   while((tempend.weekday()>4) or (IsHoliday(tempend,holidays))):
      tempend = tempend+timedelta(1)

   while tempend<=enddate:
      if((tempstart.weekday()<=4) and (not IsHoliday(tempstart,holidays))):
         print (cycle_id," ",tempstart," ",tempend," ",GetNumberOfDays(tempstart,tempend))
         
         query = "INSERT INTO %s_%s_cycles (cycle_id,cycle_start,cycle_end,cycle_days) VALUES (" % (_ccypair, _tenor)
         query = query+str(cycle_id)+",\'"+str(tempstart.strftime("%Y-%m-%d"))+"\',\'"+str(tempend.strftime("%Y-%m-%d"))+"\',"+str(GetNumberOfDays(tempstart,tempend))+")"
         
         print(query)
         cycle_id=cycle_id+1
         cur.execute(query)

      tempstart = tempstart+timedelta(1)
      
      if(_tenor == "1w"):
         tempend = GetEndDate1W(tempstart)
      else:
         tempend = GetEndDate(tempstart,_tenor)
      
      while((tempend.weekday()>4) or (IsHoliday(tempend,holidays))):
         tempend = tempend+timedelta(1)
      
conn.commit()
conn.close()


