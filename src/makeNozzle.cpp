#include <fstream>
#include <iostream>
#include "../thirdParty/fkYAML/single_include/fkYAML/node.hpp"
#include "../include/parse_inputs.hpp"
#include "../include/create_geometry.hpp"

int main()
{
    config settings = parse("inputs.yaml");
    std::cout << settings.inlet_coefficients[1] << "\n";
    create_geometry(settings);
}
