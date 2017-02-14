#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>

#undef DEBUG_OUTPUT
//#define DEBUG_OUTPUT

#include "WeightDataDay.h"
#include "HBCostFunctor.h"
#include "LinearCostFunctor.h"
#include "CalFitCostFunctor.h"
#include "LoadWDD.h"

using ceres::AutoDiffCostFunction;
using ceres::DynamicAutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;


void printOutput(double initial_weightOffset,
		 const std::vector<double>& weightOffset,
		 double initial_actScalar,
		 const std::vector<double>& actScalar,
		 const std::vector<WeightDataDay>& daysVector,
		 const Solver::Summary& summary)
{
#ifdef DEBUG_OUTPUT
  std::cout << summary.BriefReport() << "\n";
  std::cout << summary.FullReport() << "\n";
#endif

  std::cout << "    actScalar : " << initial_actScalar;
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
  std::cout << "    weightOffset : " << initial_weightOffset;
  std::cout << " -> " << weightOffset[0];
  std::cout << std::endl;
  std::cout << "    cost : " << summary.initial_cost
	    << " -> " << summary.final_cost << std::endl;
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

  // Stack layer for Linear solution
  {
  std::cout << "*** LINEAR SOLUTION ***" << std::endl;
  Problem problem;
  LinearCostFunctor* lcf = new LinearCostFunctor(daysVector);
  DynamicAutoDiffCostFunction<LinearCostFunctor>* cost_function =
    new DynamicAutoDiffCostFunction<LinearCostFunctor>(lcf);
  cost_function->AddParameterBlock(1);
  cost_function->AddParameterBlock(1);
  cost_function->SetNumResiduals(daysVector.size());
  problem.AddResidualBlock(cost_function, NULL, 
			   actScalar.data(), 
			   weightOffset.data());

  Solver::Options options;
//  options.linear_solver_type = ceres::DENSE_QR;
//  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;
  double linInit_actScalar = (daysVector[0].m_weight-daysVector[daysVector.size()-1].m_weight)/(double)daysVector.size();

  // Optimizing Offset
  actScalar[0] = linInit_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockConstant(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "  actScalar held constant " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      linInit_actScalar, actScalar,
	      daysVector, summary);

  // Optimizing Offset
  actScalar[0] = linInit_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockConstant(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "  weightOffset held constant " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      linInit_actScalar, actScalar,
	      daysVector, summary);


  // Optimizing all variables
  actScalar[0] = linInit_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "  Both parameters optimized: " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      linInit_actScalar, actScalar,
	      daysVector, summary);


  }

  // Stack layer for Calorie Estimation
  {
  std::cout << "*** CALORIE ESTIMATE ***" << std::endl;

  double aveCal = 0;
  for (auto &wdd : daysVector)
    aveCal += wdd.m_calIntake;
  aveCal = aveCal / daysVector.size();

  Problem problem;
  double initial_cal = 2000.0;
  std::vector<double> cal(1);
  cal[0] = initial_cal;
  weightOffset[0] = initial_weightOffset;
  CalFitCostFunctor* lcf = new CalFitCostFunctor(daysVector);
  DynamicAutoDiffCostFunction<CalFitCostFunctor>* cost_function =
    new DynamicAutoDiffCostFunction<CalFitCostFunctor>(lcf);
  cost_function->AddParameterBlock(1);
  cost_function->AddParameterBlock(1);
  cost_function->SetNumResiduals(daysVector.size());
  problem.AddResidualBlock(cost_function, NULL, 
			   cal.data(),
			   weightOffset.data());

  Solver::Options options;
//  options.linear_solver_type = ceres::DENSE_QR;
//  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;

  Solve(options, &problem, &summary);

  std::cout << "  Calories estimated (as check) : " << std::endl;
  std::cout << "    average cal from input data : " << aveCal << std::endl;
  std::cout << "    cal : " << initial_cal 
	    << " -> " << cal[0] << std::endl;
  std::cout << "    weightOffset : " << initial_weightOffset
	    << " -> " << weightOffset[0] << std::endl;
  std::cout << "    cost : " << summary.initial_cost
	    << " -> " << summary.final_cost << std::endl;
  }

  // Stack layer for HB solution
  {
  std::cout << "*** HB SOLUTION ***" << std::endl;
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

  std::cout << "  actScalar held constant:" << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      daysVector, summary);

  // Next holding offset constant and optimizing gain
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockConstant(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "  weightOffset held constant: " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      daysVector, summary);

  // Last optimizing all variables
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  std::cout << "  Both parameters optimized: " << std::endl;
  printOutput(initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      daysVector, summary);
  }
  return 0;
}
