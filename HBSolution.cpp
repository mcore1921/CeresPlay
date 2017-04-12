#include "HBSolution.h"
#include "HBCostFunctor.h"
#include <ctime>

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
  double offset_woOnly;
  std::map<int, double> gain_asOnly;
  double offset_both;
  std::map<int, double> gain_both;

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
  offset_woOnly = weightOffset[0];

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
  for (int i = 0; i < actScalar.size(); i++)
  {
    for (auto &wdd : m_wddSet.m_daysVector)
      if (wdd.m_parameterDim == i)
      {
	gain_asOnly[wdd.m_dayNum] = actScalar[i];
	break;
      }
  }

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

  offset_both = weightOffset[0];
  for (int i = 0; i < actScalar.size(); i++)
  {
    for (auto &wdd : m_wddSet.m_daysVector)
      if (wdd.m_parameterDim == i)
      {
	gain_both[wdd.m_dayNum] = actScalar[i];
	break;
      }
  }

  m_solutionDescription = oss.str();

  time_t rawtime = time(NULL);
  struct tm * pTime = localtime(&rawtime);
  char buffer[256];
  strftime(buffer, 256, "%Y-%m-%d_%H:%M:%S", pTime);

  std::stringstream importss;
  importss << buffer << std::endl;
  importss << std::endl;

  importss << "** OFFSET_ONLY **" << std::endl;
  importss << "OFFSET" << std::endl;
  importss << offset_woOnly << std::endl;
  importss << std::endl;

  importss << "** GAIN_ONLY **" << std::endl;
  importss << "GAIN" << std::endl;
  for (auto &p : gain_asOnly)
    importss << p.first << " ";
  importss << std::endl;
  for (auto &p : gain_asOnly)
    importss << p.second << " ";
  importss << std::endl;
  importss << std::endl;
  
  importss << "** BOTH **" << std::endl;
  importss << "OFFSET" << std::endl;
  importss << offset_both << std::endl;
  importss << std::endl;
  importss << "GAIN" << std::endl;
  for (auto &p : gain_both)
    importss << p.first << " ";
  importss << std::endl;
  for (auto &p : gain_both)
    importss << p.second << " ";
  importss << std::endl;
  importss << std::endl;
  
  m_importString = importss.str();

}




