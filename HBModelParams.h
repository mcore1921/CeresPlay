#pragma once
#include <ctime>

class HBModelParams
{
public:
  enum Gender { UNSPECIFIED = 0,
		MALE = 1,
		FEMALE = 2 };

  HBModelParams(HBModelParams::Gender g, double h, time_t dob)
    : m_gender(g)
    , m_height(h)
    , m_dob(dob)
    {}

  HBModelParams()
    : m_gender(UNSPECIFIED)
    , m_height(0)
    , m_dob(time_t(0))
    {}

  Gender m_gender;
  double m_height;
  time_t m_dob;

};
