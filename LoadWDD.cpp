#include "LoadWDD.h"
#include <fstream>
#include <string>
#include <sstream>

int loadWDD(const char* filename, 
	    HBModelParams& hbParams,
	    std::vector<WeightDataDay>& retval)
{
  std::vector<WeightDataDay> tvec;
  HBModelParams hbp;
  std::ifstream infile(filename);
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
      else if (token == "AGE")
      {
	iss >> hbp.m_age;
	if (!iss)
	  return -1;
	continue;
      }
    }

    {
      std::stringstream iss(line);
      double w, cal, act;
      iss >> w >> cal >> act;
      if (!iss)
	return -1;
      tvec.push_back(WeightDataDay(count, w, cal, act));
      count++;
    }
  }

  retval = tvec;
  hbParams = hbp;
  return 0;
}
