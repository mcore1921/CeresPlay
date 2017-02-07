#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>

#include "WeightDataDay.h"
#include "HBCostFunctor.h"
#include "LoadWDD.h"

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
  if (loadWDD("data.txt", daysVector) != 0)
  {
    std::cerr << "Error - failed to load data.txt" << std::endl;
    exit(-1);
  }

// Change this value to do piecewise slope optimization...
// With 1, you'll have a single activity gain
// With n, you'll have N activity gains, each spread evenly aross the data
// In both cases you'll only have one offset adjustment
  const int numParameterDims = 1;
  for(auto &wdd : daysVector)
  {
    double pct = (double)wdd.m_dayNum / (double)daysVector.size();   
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
    new AutoDiffCostFunction<HBCostFunctor, ceres::DYNAMIC, numParameterDims, 1>(hbcf, daysVector.size());
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

  std::cout << "Loaded " << daysVector.size() << " data points." << std::endl;
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
  std::cout << std::endl;
  std::cout << "weightOffset : " << initial_weightOffset;
  std::cout << " -> " << weightOffset[0];
  std::cout << std::endl;
  return 0;
}
