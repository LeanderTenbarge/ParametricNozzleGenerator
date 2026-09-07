
#include <fstream>
#include <iostream>
#include "../include/fkYAML/single_include/fkYAML/node.hpp"
#include "../include/parse_inputs.hpp"
#include "../include/create_geometry.hpp"

int main()
{
    config settings = parse("inputs.yaml");
    create_geometry(settings);
}
