#include <iostream>
#include <string>
#include "measuro.hpp"

int main(int argc, char * argv[])
{
    // The Registry object tracks all the app's metrics
    measuro::Registry reg;

    // Create a string metric called Hello with an initial value of an
    // empty string
    auto metric = reg.create_metric(measuro::STR::KIND, "Hello",
                      "My first metric", "");

    // Create a renderer that will render the registry's metrics as plain
    // text key-value pairs to stdout
    measuro::PlainRenderer renderer(std::cout);

    // Perform a render
    reg.render(renderer);

    // Give the Hello metric a new value
    *metric = "World";

    // Render again
    reg.render(renderer);

    return 0;
}

