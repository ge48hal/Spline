#pragma once

#include <vector>
#include <span>
#include "points/points.h"
#include "geom/shoelace.h"


std::vector<double> computeAreaTimeslices(std::span<const Points> slices);

std::vector<double> computeMomentumTimeslices(std::span<const Points> slices);


