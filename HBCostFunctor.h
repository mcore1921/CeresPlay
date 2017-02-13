#pragma once
// Cost function for Harris-Benedict equation
// Relies on a full set of data

#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>
#include "WeightDataDay.h"

class HBCostFunctor {
public:
  
  HBCostFunctor(const std::vector<WeightDataDay>& wdv)
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
      std::vector<T> lossVector = estDailyLosses(wdv, 
						 actScalar, weightOffset);
      for (int i = 0; i < wdv.size(); i++)
      {
	rollingWeightEstimate -= lossVector[i];
	T dailyError = T(wdv[i].m_weight) - rollingWeightEstimate;
	residual[i] = dailyError;
#ifdef DEBUG_OUTPUT
	std::cout << "inside CostFunctor:" << i 
		  << " " << actScalar[wdv[i].m_parameterDim]
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

  template <typename T>
  static T estDailyLoss(T baseWeight, T actEstimate, T calIntake) 
    {
      const T height = T(74);
      const T age = T(39);
      T bmr=(66.0+(6.23*baseWeight)+(12.7*height)-(6.8*age))*actEstimate;
      return (bmr-calIntake)/T(3500);
    }


  template <typename T>
  static std::vector<T> estDailyLosses(std::vector<WeightDataDay>& days, 
				const T* const actScalar,
				const T* const weightOffset)
    {
      std::vector<T> r;
      if (days.size() < 1)
	return r;
      r.push_back(T(0));
#ifdef DEBUG_OUTPUT
      std::cout << "Daily Losses computed with offset " << weightOffset[0] << std::endl;
      std::cout << 0 << " " << r[0] << std::endl;
#endif
      for (int i = 1; i < days.size(); i++)
      {
	r.push_back(estDailyLoss(T(days[i-1].m_weight),
				 T(days[i-1].m_actEstimate) * actScalar[days[i-1].m_parameterDim],
				 T(days[i-1].m_calIntake)));
#ifdef DEBUG_OUTPUT
	std::cout << i 
		  << " " << r[i] 
		  << " " << days[i-1].m_parameterDim
		  << " " << actScalar[days[i-1].m_parameterDim] 
		  << std::endl;
#endif
      }
    }


};
