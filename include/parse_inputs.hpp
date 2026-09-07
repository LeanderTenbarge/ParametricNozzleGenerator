#pragma once
#include "fkYAML/single_include/fkYAML/node.hpp"
#include <fstream>
#include <iostream>

struct config
{
    double inlet_height;
    double throat_height;
    double exit_height;

    double inlet_length;
    double exit_length;

    double outlet_height1;
    double outlet_height2;
    double domain_length;

    double radius;
    double boundary_layer_thickness;
    double wall_thickness;

    int interpolation_density;
    int arc_density;
    int num_iterations;

    std::vector<double> inlet_coefficients;
    std::vector<double> outlet_coefficients;
};

config parse(const std::string& filepath)
{
    std::ifstream file(filepath);

    if (!file)
    {
        throw std::runtime_error("Could not open " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    fkyaml::node root =
        fkyaml::node::deserialize(buffer.str());

    config settings;

    // Nozzle
    settings.inlet_height =
        root["Geometry"]["Nozzle"]["inlet_height"]
            .get_value<double>();

    settings.throat_height =
        root["Geometry"]["Nozzle"]["throat_height"]
            .get_value<double>();

    settings.exit_height =
        root["Geometry"]["Nozzle"]["exit_height"]
            .get_value<double>();

    settings.inlet_length =
        root["Geometry"]["Nozzle"]["inlet_length"]
            .get_value<double>();

    settings.exit_length =
        root["Geometry"]["Nozzle"]["exit_length"]
            .get_value<double>();

    // Outlet
    settings.outlet_height1 =
        root["Geometry"]["Outlet"]["height1"]
            .get_value<double>();

    settings.outlet_height2 =
        root["Geometry"]["Outlet"]["height2"]
            .get_value<double>();

    settings.domain_length =
        root["Geometry"]["Outlet"]["domain_length"]
            .get_value<double>();

    // Wall
    settings.radius =
        root["Geometry"]["Wall"]["radius"]
            .get_value<double>();


    settings.boundary_layer_thickness =
        root["Geometry"]["Wall"]["boundary_layer_thickness"]
            .get_value<double>();

    settings.wall_thickness =
        root["Geometry"]["Wall"]["wall_thickness"]
            .get_value<double>();

    // Interpolation
    settings.interpolation_density =
        root["Geometry"]["Interpolation"]["interpolation_density"]
            .get_value<int>();

    settings.arc_density =
        root["Geometry"]["Interpolation"]["arc_density"]
            .get_value<int>();

    settings.num_iterations =
        root["Geometry"]["Interpolation"]["num_iterations"]
            .get_value<int>();

    for (auto& node : root["Geometry"]["Nozzle"]["inlet_coefficients"])
    {
        settings.inlet_coefficients.push_back(node.get_value<double>());
    }

    for (auto& node : root["Geometry"]["Nozzle"]["outlet_coefficients"])
    {
        settings.outlet_coefficients.push_back(node.get_value<double>());
    }

    return settings;
}
