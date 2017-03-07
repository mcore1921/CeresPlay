#pragma once

#include <vector>
#include <string>
#include "ceres/ceres.h"
//#include "WeightDataDay.h"
#include "WDDSet.h"

class Solution
{
public:
  Solution(WDDSet wddSet)
    : m_wddSet(wddSet)
    {}
  virtual ~Solution() {}

  // Returns 0 on success
  virtual int solve() = 0;

  virtual std::string solutionDescription()
  {
    return m_solutionDescription;
  }

protected:
  std::ostream& printOutput(std::ostream& oss,
			    double initial_weightOffset,
			    const std::vector<double>& weightOffset,
			    double initial_actScalar,
			    const std::vector<double>& actScalar,
			    const std::vector<WeightDataDay>& daysVector,
			    const ceres::Solver::Summary& summary);

  WDDSet m_wddSet;
  std::string m_solutionDescription;

};
