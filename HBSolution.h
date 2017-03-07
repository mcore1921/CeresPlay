#pragma once

#include "Solution.h"
#include "HBModelParams.h"

class HBSolution : public Solution
{
public:
  HBSolution(WDDSet wddSet)
    : Solution(wddSet)
    {}

  virtual ~HBSolution() {}

  // Returns 0 on success
  virtual int solve();

};
