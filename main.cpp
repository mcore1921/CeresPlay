#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>

using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;

#undef DEBUG_OUTPUT
//#define DEBUG_OUTPUT

class WeightDataDay
{
public:
  WeightDataDay(int dn, double w, double ci, double ae)
    : m_dayNum(dn), m_weight(w), m_calIntake(ci), m_actEstimate(ae) 
    {}
  int m_dayNum;
  double m_weight;
  double m_calIntake;
  double m_actEstimate;
  int m_parameterDim;
};

template <typename T>
T estDailyLoss(T baseWeight, T actEstimate, T calIntake)
{
  const T height = T(74);
  const T age = T(39);
  T bmr=(66.0+(6.23*baseWeight)+(12.7*height)-(6.8*age))*actEstimate;
  return (bmr-calIntake)/T(3500);
}

template <typename T>
std::vector<T> estDailyLosses(std::vector<WeightDataDay>& days, 
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

std::vector<WeightDataDay> s_days;

struct CostFunctor {
   template <typename T>
   bool operator()(const T* const actScalar, 
		   const T* const weightOffset,
		   T* residual) const {
     residual[0] = T(0);
     T rollingWeightEstimate = T(s_days[0].m_weight) + weightOffset[0];
     std::vector<T> lossVector = estDailyLosses(s_days, 
						actScalar, weightOffset);
     for (int i = 0; i < s_days.size(); i++)
     {
       rollingWeightEstimate -= lossVector[i];
       T dailyError = T(s_days[i].m_weight) - rollingWeightEstimate;
//       residual[0] += (T(dailyError));
//       residual[0] += (dailyError < T(0)) ? dailyError*T(-1.0) : dailyError;
       residual[i] = dailyError;
#ifdef DEBUG_OUTPUT
       std::cout << "inside CostFunctor:" << i 
		 << " " << actScalar[s_days[i].m_parameterDim]
		 << " " << weightOffset[0]
		 << " " << s_days[i].m_weight
		 << " " << rollingWeightEstimate 
		 << " " << dailyError 
		 << " " << residual[i] 
		 << std::endl;
#endif
     }

     return true;
   }
};

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  int c = 1;
  s_days.push_back(WeightDataDay(c++, 242,   1800, 1.2));
  s_days.push_back(WeightDataDay(c++, 241.6, 1925, 1.2));
  s_days.push_back(WeightDataDay(c++, 242.6, 1900, 1.2));
  s_days.push_back(WeightDataDay(c++, 241.4, 1840, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.6, 1880, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.6, 1890, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.4, 1895, 1.2));
  s_days.push_back(WeightDataDay(c++, 240,   1831, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.8, 1933, 1.2));
  s_days.push_back(WeightDataDay(c++, 240,   1916, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.4, 2110, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.6, 1750, 1.2));
  s_days.push_back(WeightDataDay(c++, 238.6, 2150, 1.2));
  s_days.push_back(WeightDataDay(c++, 239.2, 1740, 1.2));
  s_days.push_back(WeightDataDay(c++, 238.8, 1725, 1.2));
  s_days.push_back(WeightDataDay(c++, 238.8, 1535, 1.2));
  s_days.push_back(WeightDataDay(c++, 238.0, 1730, 1.2));
  s_days.push_back(WeightDataDay(c++, 237.2, 1740, 1.2));
  s_days.push_back(WeightDataDay(c++, 236.8, 1770, 1.2));
  s_days.push_back(WeightDataDay(c++, 236.2, 2380, 1.2));
  s_days.push_back(WeightDataDay(c++, 235.2, 1800,  1.2));
//  s_days.push_back(WeightDataDay(c++, 235.2, 470,  1.2));

// UPDATE THIS to do piecewise slope optimization...
// With 1, you'll have a single activity gain
// With n, you'll have N activity gains, each spread evenly aross the data
// In both cases you'll only have one offset adjustment
  const int numParameterDims = 1;
  for(auto &wdd : s_days)
  {
    double pct = (double)wdd.m_dayNum / (double)c;   
    wdd.m_parameterDim = pct*numParameterDims;
  }

  // The variable to solve for with its initial value.
  double initial_actScalar = 1.0;
  double *actScalar = new double[numParameterDims];
  for (int i = 0; i < numParameterDims; i++)
  {
    actScalar[i] = initial_actScalar;
  }
  double initial_weightOffset = 0.0;
  double weightOffset[1];
  weightOffset[0] = initial_weightOffset;

  // Build the problem.
  Problem problem;

  // Set up the only cost function (also known as residual). This uses
  // auto-differentiation to obtain the derivative (jacobian).
  CostFunction* cost_function =
    new AutoDiffCostFunction<CostFunctor, ceres::DYNAMIC, numParameterDims, 1>(new CostFunctor, c-1);
  problem.AddResidualBlock(cost_function, NULL, actScalar, weightOffset);

//  problem.SetParameterBlockConstant(&weightOffset);

  // Run the solver!
  Solver::Options options;
//  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;
  Solve(options, &problem, &summary);

  std::cout << summary.BriefReport() << "\n";
//  std::cout << summary.FullReport() << "\n";

  std::cout << "actScalar : " << initial_actScalar;
  for (int i = 0; i < numParameterDims; i++)
  {
    std::cout << " -> " << actScalar[i];
    for (auto &wdd : s_days)
      if (wdd.m_parameterDim == i)
      {
	std::cout << " (day " << wdd.m_dayNum << ")";
	break;
      }
  }
  std::cout << "\n";
  std::cout << "weightOffset : " << initial_weightOffset;
  std::cout << " -> " << weightOffset[0];
  std::cout << "\n";
  return 0;
}
