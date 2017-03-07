#pragma once

#include "Solution.h"

class LinearSolution : public Solution
{
public:
  LinearSolution(WDDSet wddSet)
    : Solution(wddSet)
    {}

  virtual ~LinearSolution() {}

  // Returns 0 on success
  virtual int solve();

};
