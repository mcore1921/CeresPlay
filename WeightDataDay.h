#pragma once

class WeightDataDay
{
public:
  WeightDataDay(int dn, double w, double ci, double ae)
    : m_dayNum(dn), m_weight(w), m_calIntake(ci), m_actEstimate(ae) 
    { m_parameterDim = 0; }
  int m_dayNum;
  double m_weight;
  double m_calIntake;
  double m_actEstimate;
  int m_parameterDim;
};
