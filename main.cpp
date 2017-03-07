#include "glog/logging.h"

//#undef DEBUG_OUTPUT
//#define DEBUG_OUTPUT

#include "WeightDataDay.h"
//#include "LoadWDD.h"
#include "WDDSet.h"
#include "Solution.h"
#include "LinearSolution.h"
#include "CalorieFitSolution.h"
#include "HBSolution.h"

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

#ifdef DEBUG_OUTPUT
  std::cout << "Debug output enabled" << std::endl;
#endif

// Change this value to do piecewise slope optimization...
  enum ParamDimType { Monthly, Linear };
  ParamDimType paramDimType = Monthly;
  int numParameterDims = 1;
  int monthCutoffDays = 14;
  std::string datafile("data.txt");
  for (int i = 1; i < argc; i++)
  {
    std::string s(argv[i]);
    if (s == "-n" && argc > i+1)
    {
      std::stringstream ss(std::string(argv[i+1]));
      ss >> numParameterDims;
      paramDimType = Linear;
    }
    else if (s == "-d" && argc > i+1)
    {
      datafile = std::string(argv[i+1]);
    }
    else if (s == "-m" && argc > i+1)
    {
      std::stringstream ss(std::string(argv[i+1]));
      ss >> monthCutoffDays;
      paramDimType = Monthly;
    }
      
  }

  WDDSet wddSet(datafile);
  if (wddSet.load() != 0)
  {
    std::cerr << "Error - failed to load " << datafile << std::endl;
    exit(-1);
  }
  if (paramDimType == Linear)
    wddSet.initParameterDimsLinear(numParameterDims);
  else
    wddSet.initParameterDimsMonthly(monthCutoffDays);

  std::cout << "Loaded " << wddSet.m_daysVector.size() << " data points." << std::endl;

  std::vector<std::unique_ptr<Solution> > sols;
  sols.push_back(std::unique_ptr<Solution>(new LinearSolution(wddSet)));
  sols.push_back(std::unique_ptr<Solution>(new CalorieFitSolution(wddSet)));
  sols.push_back(std::unique_ptr<Solution>(new HBSolution(wddSet)));
  for (auto& s : sols)
  {
    s->solve();
    std::cout << s->solutionDescription();
  }

  return 0;
}
