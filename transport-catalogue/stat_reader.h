#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

namespace transport_catalogue {
    namespace output {
        void ProcessOutputRequest(TransportCatalogue& transport_catalogue, std::istream& input);
    } // namespace output
} // namespace transport_catalogue