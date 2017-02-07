#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>

#include "WeightDataDay.h"
#include "HBCostFunctor.h"

using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;

#undef DEBUG_OUTPUT
//#define DEBUG_OUTPUT

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  std::vector<WeightDataDay> daysVector;
  int c = 1;
  daysVector.push_back(WeightDataDay(c++, 242,   1800, 1.2));
  daysVector.push_back(WeightDataDay(c++, 241.6, 1925, 1.2));
  daysVector.push_back(WeightDataDay(c++, 242.6, 1900, 1.2));
  daysVector.push_back(WeightDataDay(c++, 241.4, 1840, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.6, 1880, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.6, 1890, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.4, 1895, 1.2));
  daysVector.push_back(WeightDataDay(c++, 240,   1831, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.8, 1933, 1.2));
  daysVector.push_back(WeightDataDay(c++, 240,   1916, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.4, 2110, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.6, 1750, 1.2));
  daysVector.push_back(WeightDataDay(c++, 238.6, 2150, 1.2));
  daysVector.push_back(WeightDataDay(c++, 239.2, 1740, 1.2));
  daysVector.push_back(WeightDataDay(c++, 238.8, 1725, 1.2));
  daysVector.push_back(WeightDataDay(c++, 238.8, 1535, 1.2));
  daysVector.push_back(WeightDataDay(c++, 238.0, 1730, 1.2));
  daysVector.push_back(WeightDataDay(c++, 237.2, 1740, 1.2));
  daysVector.push_back(WeightDataDay(c++, 236.8, 1770, 1.2));
  daysVector.push_back(WeightDataDay(c++, 236.2, 2380, 1.2));
  daysVector.push_back(WeightDataDay(c++, 235.2, 1800,  1.2));
//  daysVector.push_back(WeightDataDay(c++, 235.2, 470,  1.2));

// UPDATE THIS to do piecewise slope optimization...
// With 1, you'll have a single activity gain
// With n, you'll have N activity gains, each spread evenly aross the data
// In both cases you'll only have one offset adjustment
  const int numParameterDims = 1;
  for(auto &wdd : daysVector)
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
  HBCostFunctor* hbcf = new HBCostFunctor(daysVector);
  CostFunction* cost_function =
    new AutoDiffCostFunction<HBCostFunctor, ceres::DYNAMIC, numParameterDims, 1>(hbcf, c-1);
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
    for (auto &wdd : daysVector)
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
