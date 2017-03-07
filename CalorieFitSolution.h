#pragma once

#include "Solution.h"

class CalorieFitSolution : public Solution
{
public:
  CalorieFitSolution(WDDSet wddSet)
    : Solution(wddSet)
    {}

  virtual ~CalorieFitSolution() {}

  // Returns 0 on success
  virtual int solve();

};
