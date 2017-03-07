#include "LinearSolution.h"
#include "LinearCostFunctor.h"

using ceres::DynamicAutoDiffCostFunction;
using ceres::Problem;
using ceres::Solver;

int LinearSolution::solve()
{

  double initial_actScalar = 1.0;
  std::vector<double> actScalar(m_wddSet.m_numParameterDims);
  double initial_weightOffset = 0.0;
  std::vector<double> weightOffset(1);
  for (auto &d : actScalar)
    d = initial_actScalar;
  weightOffset[0] = initial_weightOffset;

  std::stringstream oss;
  oss << "*** LINEAR SOLUTION ***" << std::endl;

  Problem problem;
  LinearCostFunctor* lcf = new LinearCostFunctor(m_wddSet.m_daysVector);
  DynamicAutoDiffCostFunction<LinearCostFunctor>* cost_function =
    new DynamicAutoDiffCostFunction<LinearCostFunctor>(lcf);
  cost_function->AddParameterBlock(1);
  cost_function->AddParameterBlock(1);
  cost_function->SetNumResiduals(m_wddSet.m_daysVector.size());
  problem.AddResidualBlock(cost_function, NULL, 
			   actScalar.data(), 
			   weightOffset.data());

  Solver::Options options;
//  options.linear_solver_type = ceres::DENSE_QR;
//  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;
  double linInit_actScalar = (m_wddSet.m_daysVector[0].m_weight-m_wddSet.m_daysVector[m_wddSet.m_daysVector.size()-1].m_weight)/(double)m_wddSet.m_daysVector.size();

  // Optimizing Offset
  actScalar[0] = linInit_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockConstant(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  oss << "  weightOffset optimized " << std::endl;
  printOutput(oss,
	      initial_weightOffset, weightOffset,
	      linInit_actScalar, actScalar,
	      m_wddSet.m_daysVector, summary);

  // Optimizing Offset
  actScalar[0] = linInit_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockConstant(weightOffset.data());
  Solve(options, &problem, &summary);

  oss << "  actScalar optimized " << std::endl;
  printOutput(oss,
	      initial_weightOffset, weightOffset,
	      linInit_actScalar, actScalar,
	      m_wddSet.m_daysVector, summary);


  // Optimizing all variables
  actScalar[0] = linInit_actScalar;
  weightOffset[0] = initial_weightOffset;
  problem.SetParameterBlockVariable(actScalar.data());
  problem.SetParameterBlockVariable(weightOffset.data());
  Solve(options, &problem, &summary);

  oss << "  Both parameters optimized: " << std::endl;
  printOutput(oss, 
	      initial_weightOffset, weightOffset,
	      linInit_actScalar, actScalar,
	      m_wddSet.m_daysVector, summary);

  m_solutionDescription = oss.str();

}




