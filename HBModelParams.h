#pragma once

class HBModelParams
{
public:
  enum Gender { UNSPECIFIED = 0,
		MALE = 1,
		FEMALE = 2 };

  HBModelParams(HBModelParams::Gender g, double h, double a)
    : m_gender(g)
    , m_height(h)
    , m_age(a)
    {}

  HBModelParams()
    : m_gender(UNSPECIFIED)
    , m_height(0)
    , m_age(0)
    {}

  Gender m_gender;
  double m_height;
  double m_age;

};
