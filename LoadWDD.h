#pragma once

#include "WeightDataDay.h"
#include <vector>

// Returns 0 on success, nonzero on failure
int loadWDD(const char* filename, std::vector<WeightDataDay>& retval);
