#include "Solution.h"
#include "ceres/ceres.h"

using ceres::Solver;

std::ostream& Solution::printOutput(std::ostream& oss,
				    double initial_weightOffset,
				    const std::vector<double>& weightOffset,
				    double initial_actScalar,
				    const std::vector<double>& actScalar,
				    const std::vector<WeightDataDay>& daysVector,
				    const Solver::Summary& summary)
{
#ifdef DEBUG_OUTPUT
  oss << summary.BriefReport() << "\n";
  oss << summary.FullReport() << "\n";
#endif

  oss << "    actScalar : " << initial_actScalar;
  for (int i = 0; i < actScalar.size(); i++)
  {
    oss << " -> " << actScalar[i];
    for (auto &wdd : daysVector)
      if (wdd.m_parameterDim == i)
      {
	oss << " (day " << wdd.m_dayNum << ")";
	break;
      }
  }
  oss << std::endl;
  oss << "    weightOffset : " << initial_weightOffset;
  oss << " -> " << weightOffset[0];
  oss << std::endl;
  oss << "    cost : " << summary.initial_cost
	    << " -> " << summary.final_cost << std::endl;
  return oss;
}  
