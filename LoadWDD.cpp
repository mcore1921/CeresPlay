#include "LoadWDD.h"
#include <fstream>
#include <string>
#include <sstream>

int loadWDD(const char* filename, std::vector<WeightDataDay>& retval)
{
  std::vector<WeightDataDay> tvec;
  std::ifstream infile(filename);
  if (!infile)
    return -1;
  std::string line;
  int count = 1;
  while (std::getline(infile, line))
  {
    if (line.find("#") == 0 || line.find("//") == 0)
      continue;

    std::stringstream iss(line);
    double w, cal, act;
    iss >> w >> cal >> act;
    if (!iss)
      return -1;
    tvec.push_back(WeightDataDay(count, w, cal, act));
    count++;
  }
  retval = tvec;
  return 0;
}
