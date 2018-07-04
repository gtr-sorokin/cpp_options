import pymysql
import pass_word

for j in range(0,30):

   conn = pymysql.connect(host='127.0.0.1', user='valery', passwd=pass_word.var1, db=pass_word.var2)
   cur = conn.cursor()

   _ccypair = "usdjpy"

   cur.execute("SELECT max(m_date),max(m_minrowid) from %s_date_mapping" % (_ccypair))
   results=cur.fetchall()

   if(results[0][0]!=None):
      prev_max_date = results[0][0]
      prevrowid = results[0][1]
      print("previous date ",prev_max_date)
   else:
      prev_max_date = "1900-01-01"
      prevrowid = 0
      print("previous date ",prev_max_date)
      
   ##prev_max_date = "2010-01-01"

   mysql_query = r"SELECT distinct(quotedate) from %s_new1 where quotedate>'%s' order by quotedate limit 100" % (_ccypair,str(prev_max_date))
   #print(mysql_query)
   cur.execute(mysql_query)
   results=cur.fetchall()

   for m_date in results:
      print(m_date[0])
      query = r"SELECT MIN(rowid), MAX(rowid) from %s_new1 where quotedate='%s' and rowid>%s and rowid<%s" % (_ccypair,str(m_date[0]),str(prevrowid),str(prevrowid+2e6))
      print(query)
      cur.execute(query)
      min_rowid = cur.fetchone() 

      query = r"INSERT INTO %s_date_mapping (m_date,m_minrowid,m_maxrowid) VALUES ('%s',%s,%s) " % (_ccypair,str(m_date[0]),str(min_rowid[0]),str(min_rowid[1]))
      cur.execute(query)
      print(j, min_rowid[0],min_rowid[1],(min_rowid[1]-min_rowid[0]))
      prevrowid = min_rowid[1]

   conn.commit()
   conn.close()
