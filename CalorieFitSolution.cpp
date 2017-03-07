#include "CalorieFitSolution.h"
#include "CalFitCostFunctor.h"

using ceres::DynamicAutoDiffCostFunction;
using ceres::Problem;
using ceres::Solver;

int CalorieFitSolution::solve()
{
  std::stringstream oss;
  oss << "*** CALORIE ESTIMATE ***" << std::endl;

  double initial_weightOffset = 0.0;
  std::vector<double> weightOffset(1);
  weightOffset[0] = initial_weightOffset;

  double aveCal = 0;
  for (auto &wdd : m_wddSet.m_daysVector)
    aveCal += wdd.m_calIntake;
  aveCal = aveCal / m_wddSet.m_daysVector.size();

  Problem problem;
  double initial_cal = 2000.0;
  std::vector<double> cal(1);
  cal[0] = initial_cal;
  weightOffset[0] = initial_weightOffset;
  CalFitCostFunctor* lcf = new CalFitCostFunctor(m_wddSet.m_daysVector);
  DynamicAutoDiffCostFunction<CalFitCostFunctor>* cost_function =
    new DynamicAutoDiffCostFunction<CalFitCostFunctor>(lcf);
  cost_function->AddParameterBlock(1);
  cost_function->AddParameterBlock(1);
  cost_function->SetNumResiduals(m_wddSet.m_daysVector.size());
  problem.AddResidualBlock(cost_function, NULL, 
			   cal.data(),
			   weightOffset.data());

  Solver::Options options;
//  options.linear_solver_type = ceres::DENSE_QR;
//  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;

  Solve(options, &problem, &summary);

  oss << "  Calories estimated (as check) : " << std::endl;
  oss << "    average cal from input data : " << aveCal << std::endl;
  oss << "    cal : " << initial_cal 
	    << " -> " << cal[0] << std::endl;
  oss << "    weightOffset : " << initial_weightOffset
	    << " -> " << weightOffset[0] << std::endl;
  oss << "    cost : " << summary.initial_cost
	    << " -> " << summary.final_cost << std::endl;

  m_solutionDescription = oss.str();
}




