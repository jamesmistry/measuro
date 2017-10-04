#include <iostream>
#include <string>

#include "measuro.hpp"

int main(int argc, char * argv[])
{
    measuro::Registry reg;

    // Create a boolean metric called "example_bool" for which true maps to
    // "yes" and false maps to "no"
    auto bool_metric = reg.create_metric(measuro::BOOL::KIND, "example_bool",
        "An example boolean metric", false, "yes", "no");

    // Outputs false = no
    std::cout << "false = " << std::string(*bool_metric) << "\n";

    *bool_metric = true;

    // Outputs true = yes
    std::cout << "true = " << std::string(*bool_metric) << "\n";
}
