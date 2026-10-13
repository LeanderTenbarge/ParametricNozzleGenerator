# ParametricNozzleGenerator
## Introduction
>A parametric nozzle geometry/meshing tool built using the OpenCascade C++ API and GMSH (.geo) scripting. 

## What it does/How it works?
>The program takes a set of inputs (listed in inputs.yaml). In order to effectively use this tool it is imperative to understand the coefficients for the spline interpolation are >dimensionless, as they move from 0.0 to 1.0 in a linear fashion, the inputs are listed as the following:
> Geometry/Nozzle:
>1. inlet_height
>2. throat_height
>3. exit_height
>4. inlet_length
>5. exit_length
>6. inlet_coefficients
>7. outlet_coefficients



> Geometry/Outlet
>1  height1
>2. height2
>3. domain_length



> Geometry/Wall:
>1. radius
>2. boundary_layer_thickness
>3. wall_thickness



> Geometry/Interpolation:
>1. interpolation_density
>2. arc_density
>3. num_iterations


Then with the inputs, the program converts the dimensionless coefficients into a Piecewise Cubic Hermite Interpolating Polynomial (PHIP) based spline interpolation algorithm. This specific algorithm was utilized in order to preserve monotonicity and minimize the change for sharp overshoots that would cause issues in mesh generation. 

<p align="center">
  <img src="images/comparison.png" alt="Comparison of Spline Methods width="500">
</p>
