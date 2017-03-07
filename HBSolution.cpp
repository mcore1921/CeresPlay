#include "HBSolution.h"
#include "HBCostFunctor.h"

using ceres::DynamicAutoDiffCostFunction;
using ceres::Problem;
using ceres::Solver;

int HBSolution::solve()
{

  double initial_actScalar = 1.0;
  std::vector<double> actScalar(m_wddSet.m_numParameterDims);
  double initial_weightOffset = 0.0;
  std::vector<double> weightOffset(1);
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;

  std::stringstream oss;

  oss << "*** HB SOLUTION ***" << std::endl;
  Problem problem;
  HBCostFunctor* hbcf = new HBCostFunctor(m_wddSet.m_hbParams, m_wddSet.m_daysVector);
  DynamicAutoDiffCostFunction<HBCostFunctor>* cost_function =
    new DynamicAutoDiffCostFunction<HBCostFunctor>(hbcf);
  cost_function->AddParameterBlock(m_wddSet.m_numParameterDims);
  cost_function->AddParameterBlock(1);
  cost_function->SetNumResiduals(m_wddSet.m_daysVector.size());
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

  oss << "  weightOffset optimized:" << std::endl;
  printOutput(oss,
	      initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      m_wddSet.m_daysVector, summary);

  // Next holding offset constant and optimizing gain
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockConstant(weightOffset.data());
  Solve(options, &problem, &summary);

  oss << "  actScalar optimized: " << std::endl;
  printOutput(oss,
	      initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      m_wddSet.m_daysVector, summary);

  // Last optimizing all variables
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  oss << "  Both parameters optimized: " << std::endl;
  printOutput(oss,
	      initial_weightOffset, weightOffset,
	      initial_actScalar, actScalar,
	      m_wddSet.m_daysVector, summary);

  m_solutionDescription = oss.str();

}




