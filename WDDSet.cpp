#include "WDDSet.h"

#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <map>
#include <algorithm>

namespace {

void init_tm(struct tm& t)
{
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_mday = 0;
  t.tm_mon = 0;
  t.tm_year = 0;
  t.tm_wday = 0;
  t.tm_yday = 0;
  t.tm_isdst = 0;
}

tm init_tm()
{
  tm t;
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_mday = 0;
  t.tm_mon = 0;
  t.tm_year = 0;
  t.tm_wday = 0;
  t.tm_yday = 0;
  t.tm_isdst = 0;
  return t;
}

time_t timeFromString(std::string timestring)
{
  tm inputDate;
  init_tm(inputDate);
  while (timestring.find("-") != std::string::npos)
	timestring.replace(timestring.find("-"), 1, " ");
  std::stringstream timestringss(timestring);
  timestringss >> inputDate.tm_year >> inputDate.tm_mon >> inputDate.tm_mday;
  if (!timestringss)
    return -1;
  inputDate.tm_year -= 1900; // Correct for year offset (year since 1900)
  inputDate.tm_mon -= 1; // Correct for mon offset (month of year from 0 to 11)
  return mktime(&inputDate);
}

}

int WDDSet::load()
{

  std::vector<WeightDataDay> tvec;
  HBModelParams hbp;
  std::ifstream infile(m_filename.c_str());
  if (!infile)
    return -1;
  std::string line;
  int count = 1;
  while (std::getline(infile, line))
  {
    if (line.find("#") == 0 || line.find("//") == 0)
      continue;

    // Stack layer for variable protection
    {
      std::stringstream iss(line);
      std::string token;
      iss >> token;
      if (token == "GENDER")
      {
	std::string gender;
	iss >> gender;
	if (gender == "MALE")
	  hbp.m_gender = HBModelParams::MALE;
	else if (gender == "FEMALE")
	  hbp.m_gender = HBModelParams::FEMALE;
	else
	  return -1;
	if (!iss)
	  return -1;
	continue;
      }
      else if (token == "HEIGHT")
      {
	iss >> hbp.m_height;
	if (!iss)
	  return -1;
	continue;
      }
      else if (token == "DOB")
      {
	std::string date;
	iss >> date;
	time_t datenum = timeFromString(date);
	if (datenum == -1)
	  return -1;
	hbp.m_dob = datenum;
	if (!iss)
	  return -1;
	continue;
      }
    }

    {
      std::stringstream iss(line);
      std::string date;
      double w, cal, act;
      iss >> date >> w >> cal >> act;
      if (!iss)
	return -1;
      time_t datenum = timeFromString(date);
      if (datenum == -1)
	return -1;
      tvec.push_back(WeightDataDay(count, datenum, w, cal, act));
      count++;
    }
  }

  m_daysVector = tvec;
  m_hbParams = hbp;
  return 0;
}

void WDDSet::initParameterDimsLinear(int numParameterDims)
{
  m_numParameterDims = numParameterDims;
  for(auto &wdd : m_daysVector)
  {
    double pct = (double)wdd.m_dayNum / (double)m_daysVector.size();   
    wdd.m_parameterDim = pct*m_numParameterDims;
  }
  
}

// Any month with fewer days than this will be lumped in with prior / post months.
void WDDSet::initParameterDimsMonthly(int cutoffDays)
{
  // Map count of months since 1900 to # entries in that month
  std::map<int, int> paramMap, monthCount;
  for (auto &wdd : m_daysVector)
  {
    tm t = *gmtime(&wdd.m_date);
    // Cheat a bit and rely on auto-initialization of val to 0 if key isn't present
    monthCount[t.tm_year*12+t.tm_mon]++;
  }

  std::vector<int> keys;
  for (auto &mcEntry : monthCount)
    keys.push_back(mcEntry.first);
  // Sort the keys to be sure we attack in the appropriate order
  std::sort(keys.begin(), keys.end());
  int paramDim = 0;

  for (int i = 0; i < keys.size(); i++)
  {
    // If there aren't enough days, use the running counter and
    // don't increment.  Note this covers the starting condition where there's
    // only one month, or there aren't enough days in the first month to warrant
    // its own dimension.
    // This does not cover the final month not having enough days; that's something
    // we'll check after the loop is over
    if (monthCount[keys[i]] < cutoffDays)
    {
      paramMap[keys[i]] = paramDim;
    }
    else
    {
      paramMap[keys[i]] = paramDim;
      paramDim++;
    }
  }

  // Now check for the condition that the last month was short; 
  // In this case we'll set its dim to the prior month
  if (keys.size() > 1 &&
      monthCount[keys[keys.size()-1]] < cutoffDays)
  {
    paramMap[keys[keys.size()-1]] = paramMap[keys[keys.size()-2]];
  }
      
  // Now we have an appropriate mapping; let's go mark each WDD correctly
  for (auto &wdd : m_daysVector)
  {
    tm t = *gmtime(&wdd.m_date);
    wdd.m_parameterDim = paramMap[t.tm_year*12+t.tm_mon];
  }

  // Lastly, set the number of parameters
  m_numParameterDims = paramMap[keys[keys.size()-1]] + 1;
}
