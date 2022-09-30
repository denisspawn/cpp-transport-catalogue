#pragma once

#include <cmath>

namespace geo {

const double EPSILON = 1e-6;
const int EARTH_RADIUS = 6371000;

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return std::abs(lat - other.lat) < EPSILON && std::abs(lng - other.lng) < EPSILON;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);
    
} // namespace geo