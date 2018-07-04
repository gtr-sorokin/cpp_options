#ifndef _DT_H
#define _DT_H

#include <iostream>
#include <string>

class dt
{
 public:
  dt(int,int,int,int,int,int,int);
  dt(int,int,int);
  dt(const std::string &);
  dt(const dt &);
  dt & operator=(const dt &);
  dt copy();

  void print() const;
  void print_date(bool) const;
  int CalculateNumberOfDays(const dt&);
  double CalculateNumberOfYears(const dt&, int);
  void AddDay();
  void SubtractDay();
  void AddNDays(int);
  void AddNMonths(int);
  void AddNYears(int);
  bool IsLeapYear() const;
  int GetYear() const {return this->year;}
  int GetMonth() const {return this->month;}
  int GetDay() const {return this->day;}
  int GetHour() const {return this->hour;}
  int GetMinute() const {return this->minute;}
  int GetSecond() const {return this->second;}
  int GetMillisecond() const {return this->millisecond;}
  int GetDayOfWeek() const {return this->dayofweek;}

  void SetHHMMSSMS(int,int,int,int);

  bool gt(const dt&, bool);
  bool lt(const dt&, bool);
  bool eq(const dt&, bool);

 private:
  unsigned short gnum;   //used to calculate the difference between two dates in days
  unsigned short dayofweek; // Sun = 0; Mon = 1; ...
  std::string dayofweek_s; // 3-letter day of week representation
  unsigned short year;
  unsigned short month;  // 1 - 12
  unsigned short day;    // 1 - xx
  unsigned short hour;
  unsigned short minute;
  unsigned short second;
  unsigned short millisecond;
  void CalculateDayOfWeek();
  void CalculateGNum();
};

void dt::SetHHMMSSMS(int h, int m, int s, int ms){
  this->hour = h;
  this->minute = m;
  this->second = s;
  this->millisecond = ms;
}

bool dt::IsLeapYear() const {
  return (this->year % 4 == 0) && ((this->year % 100 != 0) || (this->year % 400 == 0));
}

void dt::AddNDays(int n){
  for (int i=1; i<=n; i++) {
    this->AddDay();
  }
}

void dt::SubtractDay(){
  unsigned short month_length[]      =     {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if((this->month==1)&&(this->day==1)){
    this->year-=1;
    this->month=12;
    this->day=31;
  }
  else if (this->day==1) {
    this->month-=1;
    int Feb_adj = ((this->IsLeapYear())&&(this->month==2))?1:0;
    this->day=month_length[this->month-1]+Feb_adj;
  }
  else{
    this->day-=1;
  }

  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

void dt::AddDay(){
  
  unsigned short month_length[]      =     {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int Feb_adj = ((this->IsLeapYear())&&(this->month==2))?1:0;

  if((this->month==12)&&(this->day==31)){
    this->year+=1;
    this->month=1;
    this->day=1;
  }
  else if (this->day==(month_length[this->month-1]+Feb_adj)) {
    this->month+=1;
    this->day=1;
  }
  else{
    this->day+=1;
  }

  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

void dt::AddNYears(int n){
  this->year+=n;
  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

void dt::AddNMonths(int n){
  int res = n % 12;
  int num_years = (n-res)/12;

  this->AddNYears(num_years);

  if(this->month+res>12){
      this->AddNYears(1);
      this->month=(this->month+res-12);
  }
  else{
      this->month=(this->month+res);
  }
    
  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

void dt::CalculateDayOfWeek(){

  unsigned int d = this->day;
  unsigned int m = this->month;
  unsigned int y = this->year;

  this->dayofweek = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;
  std::string dayofweek_strings[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  this->dayofweek_s = dayofweek_strings[this->dayofweek];

}

void dt::CalculateGNum(){

  unsigned int d = this->day;
  unsigned int m = this->month;
  unsigned int y = this->year;

  m = (m + 9) % 12;
  y = y - m/10;
  this->gnum = (365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( d - 1 ));
}


int dt::CalculateNumberOfDays(const dt & dt2){
  /*
    function g(y,m,d)
    m = (m + 9) % 12
    y = y - m/10
    return 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( d - 1 )

    Difference between two dates = g(y2,m2,d2) - g(y1,m1,d1)
  */

  return (this->gnum - dt2.gnum);
}

double dt::CalculateNumberOfYears(const dt & dt2, int base = 365){
  return ((double)(this->CalculateNumberOfDays(dt2))+((double)(this->hour-dt2.hour))/24+((double)(this->minute-dt2.minute))/(24*60)+((double)(this->second-dt2.second))/(24*60*60)+((double)(this->millisecond-dt2.millisecond))/(24*60*60*1000))/base;
}

bool dt::gt(const dt & dt2, bool dateonly = false){
  if(this->year>dt2.year)
    return true;
  else if ((this->year==dt2.year)&&(this->month>dt2.month))
    return true;
  else if((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day>dt2.day))
    return true;
  else if((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day==dt2.day)&&(this->hour>dt2.hour)&&(not dateonly))
    return true;
  else if((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day==dt2.day)&&(this->hour==dt2.hour)&&(this->minute>dt2.minute)&&(not dateonly))
    return true;
  else if((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day==dt2.day)&&(this->hour==dt2.hour)&&(this->minute==dt2.minute)&&(this->second>dt2.second)&&(not dateonly))
    return true;
  else if ((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day==dt2.day)&&(this->hour==dt2.hour)&&(this->minute==dt2.minute)&&(this->second==dt2.second)&&(this->millisecond>dt2.millisecond)&&(not dateonly))
    return true;
  else
    return false;
}

bool dt::eq(const dt & dt2,bool dateonly = false){

  if (not dateonly){
    if((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day==dt2.day)&&(this->hour==dt2.hour)&&(this->minute==dt2.minute)&&(this->second==dt2.second)&&(this->millisecond==dt2.millisecond))
      return true;
    else 
      return false;
  }
  else{
    if((this->year==dt2.year)&&(this->month==dt2.month)&&(this->day==dt2.day))
      return true;
    else 
      return false;
  }
}

bool dt::lt(const dt & dt2, bool dateonly = false){

  bool cond1 = this->gt(dt2,dateonly);
  if (cond1)
    return false;

  bool cond2 = this->eq(dt2,dateonly);
  if (cond2)
    return false;

  if ((not cond1) && (not cond2))
    return true;
}

void dt::print() const {
  std::printf("%4i-%02i-%02i %02i:%02i:%02i.%03i\n",this->year,this->month,this->day, this->hour, this->minute, this->second,this->millisecond);
}

void dt::print_date(bool printweekday = false) const {
  if(not printweekday)
    std::printf("%4i-%02i-%02i\n",this->year,this->month,this->day);
  else
    std::printf("%4i-%02i-%02i %s\n",this->year,this->month,this->day,this->dayofweek_s.c_str());
}

dt dt::copy(){
  return dt(this->year,this->month,this->day,this->hour,this->minute,this->second,this->millisecond);
}

dt & dt::operator=(const dt & mydt){
  if(this==&mydt){
    return *this;
  }
  else
  {
    this->year = mydt.year;
    this->month = mydt.month;
    this->day = mydt.day;
    this->hour = mydt.hour;
    this->minute = mydt.minute;
    this->second = mydt.second;
    this->millisecond = mydt.millisecond;

    this->CalculateDayOfWeek();
    this->CalculateGNum();

    return *this;
  }
}

dt::dt(const dt & mydt){
  this->year = mydt.year;
  this->month = mydt.month;
  this->day = mydt.day;
  this->hour = mydt.hour;
  this->minute = mydt.minute;
  this->second = mydt.second;
  this->millisecond = mydt.millisecond;

  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

dt::dt(const std::string & dtstr){

  //0123456789
  //2017-03-10
  if (dtstr.size()==10){
    this->year=std::stoi(dtstr.substr(0,4));
    this->month = std::stoi(dtstr.substr(5,2));
    this->day = std::stoi(dtstr.substr(8,2));
    this->hour = 1;
    this->minute = 1;
    this->second = 1;
    this->millisecond = 1;
  }
  
  //0123456789112345678
  //2017-03-10 16:54:12
  else if (dtstr.size()==19){
    this->year=std::stoi(dtstr.substr(0,4));
    this->month = std::stoi(dtstr.substr(5,2));
    this->day = std::stoi(dtstr.substr(8,2));
    this->hour = std::stoi(dtstr.substr(11,2));
    this->minute = std::stoi(dtstr.substr(14,2));
    this->second = std::stoi(dtstr.substr(17,2));
    this->millisecond = 1;
  }

  //01234567891123456789212
  //2017-03-10 16:54:12.001
  else{ 
    this->year=std::stoi(dtstr.substr(0,4));
    this->month = std::stoi(dtstr.substr(5,2));
    this->day = std::stoi(dtstr.substr(8,2));
    this->hour = std::stoi(dtstr.substr(11,2));
    this->minute = std::stoi(dtstr.substr(14,2));
    this->second = std::stoi(dtstr.substr(17,2));
    this->millisecond = std::stoi(dtstr.substr(20,3));
  }

  this->CalculateDayOfWeek();
  this->CalculateGNum();
}


dt::dt(int y, int m, int d){
  this->year = y;
  this->month = m;
  this->day = d;
  this->hour = 1;
  this->minute = 1;
  this->second = 1;
  this->millisecond = 1;  

  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

dt::dt(int y, int m, int d, int h, int mi, int s, int ms){
  this->year = y;
  this->month = m;
  this->day = d;
  this->hour = h;
  this->minute = mi;
  this->second = s;
  this->millisecond = ms;

  this->CalculateDayOfWeek();
  this->CalculateGNum();
}

#endif
