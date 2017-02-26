#pragma once
#include <ctime>

class WeightDataDay
{
public:
  WeightDataDay(int dn, time_t d, double w, double ci, double ae)
    : m_dayNum(dn), m_date(d), m_weight(w), m_calIntake(ci), m_actEstimate(ae) 
    { m_parameterDim = 0; }
  int m_dayNum;
  time_t m_date;
  double m_weight;
  double m_calIntake;
  double m_actEstimate;
  int m_parameterDim;
};
