#include "ceres/ceres.h"
#include "glog/logging.h"
#include <map>

using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;

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
std::vector<T> estDailyLosses(std::vector<WeightDataDay>& days, const T* const x)
{
  std::vector<T> r;
  if (days.size() < 1)
    return r;
  r.push_back(T(0));
  std::cout << 0 << " " << r[0] << std::endl;
  for (int i = 1; i < days.size(); i++)
  {
    r.push_back(estDailyLoss(T(days[i-1].m_weight), 
			     T(days[i-1].m_actEstimate) * x[0],
			     T(days[i-1].m_calIntake)));
    std::cout << i << " " << r[i] << std::endl;
  }
}

std::vector<WeightDataDay> s_days;

struct CostFunctor {
   template <typename T>
   bool operator()(const T* const x, T* residual) const {
     residual[0] = T(0);
     T rollingWeightEstimate = T(s_days[0].m_weight);
     std::vector<T> lossVector = estDailyLosses(s_days, x);
     for (int i = 0; i < s_days.size(); i++)
     {
       rollingWeightEstimate -= lossVector[i];
       T dailyError = T(s_days[i].m_weight) - rollingWeightEstimate;
       residual[0] += T(dailyError);
       std::cout << "inside CostFunctor:" << i << " " << x[0]
		 << " " << s_days[i].m_weight
		 << " " << rollingWeightEstimate 
		 << " " << dailyError 
		 << " " << residual[0] 
		 << std::endl;
     }
//     residual[0] = T(10.0) - x[0];
     return true;
   }
};

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  int c = 1;
  s_days.push_back(WeightDataDay(c++, 242, 1800, 1.2));
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
  s_days.push_back(WeightDataDay(c++, 238.0, 1130, 1.2));
//  estDailyLoss(s_days);

  // The variable to solve for with its initial value.
  double initial_x = 1.0;
  double x = initial_x;

  // Build the problem.
  Problem problem;

  // Set up the only cost function (also known as residual). This uses
  // auto-differentiation to obtain the derivative (jacobian).
  CostFunction* cost_function =
      new AutoDiffCostFunction<CostFunctor, 1, 1>(new CostFunctor);
  problem.AddResidualBlock(cost_function, NULL, &x);

  // Run the solver!
  Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;
  Solve(options, &problem, &summary);

  std::cout << summary.BriefReport() << "\n";
  std::cout << "x : " << initial_x
            << " -> " << x << "\n";
  return 0;
}
