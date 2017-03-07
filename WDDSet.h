#pragma once

#include "WeightDataDay.h"
#include "HBModelParams.h"
#include <vector>
#include <string>

class WDDSet
{
public:
  WDDSet(const char* filename)
    : m_filename(filename)
    , m_numParameterDims(0)
    {}

  WDDSet(std::string filename)
    : m_filename(filename)
    , m_numParameterDims(0)
    {}

  int load();

  void initParameterDimsLinear(int numParameterDims);

  std::string m_filename;
  HBModelParams m_hbParams;
  std::vector<WeightDataDay> m_daysVector;
  int m_numParameterDims;
};

