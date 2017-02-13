#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>

#include "WeightDataDay.h"
#include "HBCostFunctor.h"
#include "LoadWDD.h"

using ceres::AutoDiffCostFunction;
using ceres::DynamicAutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;

#undef DEBUG_OUTPUT
//#define DEBUG_OUTPUT

void printOutput(double initial_weightOffset,
		 const std::vector<double>& weightOffset,
		 double initial_actScalar,
		 const std::vector<double>& actScalar,
		 const std::vector<WeightDataDay>& daysVector)
{
#ifdef DEBUG_OUTPUT
  std::cout << summary.BriefReport() << "\n";
  std::cout << summary.FullReport() << "\n";
#endif

  std::cout << "actScalar : " << initial_actScalar;
  for (int i = 0; i < actScalar.size(); i++)
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
}  

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  std::vector<WeightDataDay> daysVector;
  if (loadWDD("data.txt", daysVector) != 0)
  {
    std::cerr << "Error - failed to load data.txt" << std::endl;
    exit(-1);
  }

// Change this value to do piecewise slope optimization...
  int numParameterDims = 1;
  for (int i = 1; i < argc; i++)
  {
    std::string s(argv[i]);
    if (s == "-n" && argc > i+1)
    {
      std::stringstream ss(std::string(argv[i+1]));
      ss >> numParameterDims;
    }
  }

  for(auto &wdd : daysVector)
  {
    double pct = (double)wdd.m_dayNum / (double)daysVector.size();   
    wdd.m_parameterDim = pct*numParameterDims;
  }
  std::cout << "Loaded " << daysVector.size() << " data points." << std::endl;

  double initial_actScalar = 1.0;
  std::vector<double> actScalar(numParameterDims);
  double initial_weightOffset = 0.0;
  std::vector<double> weightOffset(1);
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;

  Problem problem;
  HBCostFunctor* hbcf = new HBCostFunctor(daysVector);
  DynamicAutoDiffCostFunction<HBCostFunctor>* cost_function =
    new DynamicAutoDiffCostFunction<HBCostFunctor>(hbcf);
  cost_function->AddParameterBlock(numParameterDims);
  cost_function->AddParameterBlock(1);
  cost_function->SetNumResiduals(daysVector.size());
  problem.AddResidualBlock(cost_function, NULL, 
			   actScalar.data(), 
			   weightOffset.data());

  Solver::Options options;
//  options.linear_solver_type = ceres::DENSE_QR;
//  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;

  // First holding gain constant and optimizing offset
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockConstant(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "actScalar held constant:" << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      daysVector);

  // Next holding offset constant and optimizing gain
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockConstant(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "weightOffset held constant: " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      daysVector);

  // Last optimizing all variables
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "Both parameters optimized: " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      daysVector);
  return 0;
}
