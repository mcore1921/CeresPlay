#pragma once
// Cost function for linear fit to data
// Relies on a full set of data

#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>
#include "WeightDataDay.h"

class LinearCostFunctor {
public:
  
  LinearCostFunctor(const std::vector<WeightDataDay>& wdv)
    : m_wdv(wdv) {}

  template <typename T>
  bool operator()(T const* const* parameters,
		  T* residual) const
    {
      const T* const actScalar = parameters[0];
      const T* const weightOffset = parameters[1];
      residual[0] = T(0);
      std::vector<WeightDataDay> wdv(m_wdv);
      T rollingWeightEstimate = T(wdv[0].m_weight) + weightOffset[0];
      for (int i = 0; i < wdv.size(); i++)
      {
	if (i > 0)
	  rollingWeightEstimate -= actScalar[0];
	T dailyError = T(wdv[i].m_weight) - rollingWeightEstimate;
	residual[i] = dailyError;
#ifdef DEBUG_OUTPUT
	std::cout << "inside LinearCostFunctor:" << i 
		  << " " << actScalar[0]
		  << " " << weightOffset[0]
		  << " " << wdv[i].m_weight
		  << " " << rollingWeightEstimate 
		  << " " << dailyError 
		  << " " << residual[i] 
		  << std::endl;
#endif
      }
  
  return true;
    }
  
  std::vector<WeightDataDay> m_wdv;

private:

};
