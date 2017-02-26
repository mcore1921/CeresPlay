#pragma once

#include "WeightDataDay.h"
#include "HBModelParams.h"
#include <vector>

// Returns 0 on success, nonzero on failure
int loadWDD(const char* filename, 
	    HBModelParams& params,
	    std::vector<WeightDataDay>& retval);
