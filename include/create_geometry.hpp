#pragma once

#include <iostream>
#include "parse_inputs.hpp"
#include <cmath>
#include <vector>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>
#include "pchip.hpp"

#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS_Edge.hxx>
#include <GC_MakeArcOfCircle.hxx>

#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepTools.hxx>

#include <Geom_BSplineCurve.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_HArray1OfPnt.hxx>

#include <STEPControl_Writer.hxx>
#include <IFSelect_ReturnStatus.hxx>

#define pi 3.14159265358979323846

void create_geometry
(
	const config& settings
)
{

	std::vector<double> inlet_coefficients = settings.inlet_coefficients;
	std::vector<double> exit_coefficients = settings.outlet_coefficients;


	std::vector<double> exit_x = {0.00, 0.25, 0.50,  0.75, 1.00};
	std::vector<double> inlet_x = {0.00, 0.25, 0.50, 0.75, 1.00};


	if (!(exit_x.size() == exit_coefficients.size()) || !(inlet_x.size() == inlet_coefficients.size()))
	{
		throw std::invalid_argument("Invalid Argument, Input vectors are unequal in length");
	}

	int n_exit = exit_x.size();
	int n_inlet = inlet_x.size();

	// Create Sampling Vectors:
	std::vector<double> inlet_sampled_x(settings.interpolation_density);
	std::vector<double> inlet_sampled_y(settings.interpolation_density);
	std::vector<double> inlet_sampled_bl_x(settings.interpolation_density);
	std::vector<double> inlet_sampled_bl_y(settings.interpolation_density);

	std::vector<double> exit_sampled_x(settings.interpolation_density);
	std::vector<double> exit_sampled_y(settings.interpolation_density);
	std::vector<double> exit_sampled_bl_x(settings.interpolation_density);
	std::vector<double> exit_sampled_bl_y(settings.interpolation_density);

	// Perform Pchip Interpolation:
	pchip_interpolation exit_interp = pchip_interpolation(exit_x, exit_coefficients);
	pchip_interpolation inlet_interp = pchip_interpolation(inlet_x, inlet_coefficients);

	// Direction Calculations:
	double d_exit_start = exit_interp.return_d_(0);
	double d_exit_end = exit_interp.return_d_(n_exit - 1);
	double d_inlet_end = inlet_interp.return_d_(n_inlet - 1);

	double exit_start_angle = atan2(d_exit_start, 1) - pi / 2;
	double exit_end_angle = atan2(exit_interp.return_d_(n_exit - 1), 1) - pi / 2;
	double inlet_end_angle = atan2(inlet_interp.return_d_(n_inlet - 1), -1) + pi / 2;

	double exit_start_x, exit_start_y;
	double exit_end_x, exit_end_y;
	double inlet_end_x, inlet_end_y;
	double inlet_end_bl_x, inlet_end_bl_y;
	double exit_start_bl_x, exit_start_bl_y;
	double exit_end_bl_x, exit_end_bl_y;

	// Iterations for Radius Offset:
	for (int iter = 0; iter < settings.num_iterations; iter++)
	{

		// Finding New Offsets:
		exit_start_x = settings.inlet_length + settings.radius * cos(exit_start_angle);
		exit_start_y = settings.throat_height + settings.radius + settings.radius * sin(exit_start_angle);

		exit_start_bl_x = settings.inlet_length + (settings.radius + settings.boundary_layer_thickness) * cos(exit_start_angle);
		exit_start_bl_y = settings.throat_height + settings.radius + (settings.radius + settings.boundary_layer_thickness) * sin(exit_start_angle);

		exit_end_x = settings.inlet_length + settings.exit_length - settings.radius * cos(exit_end_angle);
		exit_end_y = settings.exit_height - settings.radius * sin(exit_end_angle);
		exit_end_bl_x = settings.inlet_length + settings.exit_length + settings.boundary_layer_thickness * cos(exit_end_angle);
		exit_end_bl_y = settings.exit_height + settings.boundary_layer_thickness * sin(exit_end_angle);

		inlet_end_x = settings.inlet_length + settings.radius * cos(inlet_end_angle);
		inlet_end_y = settings.throat_height + settings.radius + settings.radius * sin(inlet_end_angle);

		inlet_end_bl_x = settings.inlet_length + (settings.radius + settings.boundary_layer_thickness) * cos(inlet_end_angle);
		inlet_end_bl_y = settings.throat_height + settings.radius + (settings.radius + settings.boundary_layer_thickness) * sin(inlet_end_angle);

		// Finding New Aspect Ratio from those Offsets:
		double sx_exit = exit_end_x - exit_start_x;
		double sy_exit = exit_end_y - exit_start_y;

		double sx_inlet = inlet_end_x;
		double sy_inlet = inlet_end_y - settings.inlet_height;

		// Adjusting for new Aspect ratio:
		double slope_exit_start = d_exit_start * sy_exit / sx_exit;
		double new_exit_start_angle = atan2(slope_exit_start, 1) - pi / 2;

		double slope_exit_end = d_exit_end * sy_exit / sx_exit;
		double new_exit_end_angle = atan2(slope_exit_end, 1) - pi / 2;

		double slope_inlet_end = d_inlet_end * sy_inlet / sx_inlet;
		double new_inlet_end_angle = atan2(slope_inlet_end, -1) + pi / 2;

		exit_start_angle = new_exit_start_angle;
		exit_end_angle = new_exit_end_angle;
		inlet_end_angle = new_inlet_end_angle;
	}

	double exit_half_angle = exit_end_angle / 2.0;

	auto exit_x_reference = [&exit_start_x, &exit_end_x, &settings](const double &query)
	{
    	return exit_start_x + query * (settings.inlet_length + settings.exit_length - exit_start_x);
	};

	auto exit_x_reference_bl = [&exit_start_bl_x, &exit_end_bl_x, &settings](const double &query)
	{
    	return exit_start_bl_x + query * (exit_end_bl_x - exit_start_bl_x);
	};

	auto exit_y_reference = [&exit_start_y, &exit_end_y, &settings](const double &query)
	{
		return exit_start_y + query * (settings.exit_height - exit_start_y);
	};

	auto exit_y_reference_bl = [&exit_start_bl_y, &exit_end_bl_y, &settings](const double &query)
	{
		return exit_start_bl_y + query * (exit_end_bl_y - exit_start_bl_y);
	};

	auto inlet_x_reference = [&inlet_end_x](const double &query)
	{
		return query * inlet_end_x;
	};

	auto inlet_x_reference_bl = [&inlet_end_bl_x](const double &query)
	{
		return query * inlet_end_bl_x;
	};

	auto inlet_y_reference = [&inlet_end_y, &settings](const double &query)
	{
		return inlet_end_y + query * (settings.inlet_height - inlet_end_y);
	};

	auto inlet_y_reference_bl = [&inlet_end_bl_y, &settings](const double &query)
	{
		return inlet_end_bl_y + query * (settings.inlet_height - inlet_end_bl_y - settings.boundary_layer_thickness);
	};


	for (int i = 0; i < settings.interpolation_density; i++)
	{
		double t = static_cast<double>(i) / static_cast<double>(settings.interpolation_density - 1);

		exit_sampled_x[i] = exit_x_reference(t);
		exit_sampled_y[i] = exit_y_reference(exit_interp(t));

		exit_sampled_bl_x[i] = exit_x_reference_bl(t);
		exit_sampled_bl_y[i] = exit_y_reference_bl(exit_interp(t));

		inlet_sampled_x[i] = inlet_x_reference(t);
		inlet_sampled_y[i] = inlet_y_reference(inlet_interp(t));

		inlet_sampled_bl_x[i] = inlet_x_reference_bl(t);
		inlet_sampled_bl_y[i] = inlet_y_reference_bl(inlet_interp(t));
	}

	double top_offset_x = exit_end_x - (settings.inlet_length + settings.exit_length);
	double top_offset_y = exit_end_y - settings.exit_height;

	double top_offset_bl_x = exit_end_bl_x - (settings.inlet_length + settings.exit_length);
	double top_offset_bl_y = exit_end_bl_y - settings.exit_height;

	gp_Pnt one_zero(0.0, settings.inlet_height - settings.boundary_layer_thickness, 0.0);
	gp_Pnt one_one(0.0, settings.inlet_height, 0.0);

	gp_Pnt two_zero(settings.inlet_length, settings.throat_height + settings.radius, 0.0);
	gp_Pnt two_one(inlet_end_x, inlet_end_y, 0.0);
	gp_Pnt two_two(settings.inlet_length, settings.throat_height, 0.0);
	gp_Pnt two_three(exit_start_x, exit_start_y, 0.0);
	gp_Pnt two_four(inlet_end_bl_x, inlet_end_bl_y, 0.0);
	gp_Pnt two_five(settings.inlet_length, settings.throat_height - settings.boundary_layer_thickness, 0.0);
	gp_Pnt two_six(exit_start_bl_x, exit_start_bl_y, 0);

	gp_Pnt three_zero(exit_end_x, exit_end_y, 0.0);
	gp_Pnt three_one(settings.inlet_length + settings.exit_length, settings.exit_height, 0.0);
	gp_Pnt three_two(exit_end_x + settings.radius * cos(exit_half_angle), exit_end_y + settings.radius * sin(exit_half_angle), 0.0);
	gp_Pnt three_three(exit_end_x + settings.radius, exit_end_y, 0.0);
	gp_Pnt three_four(exit_end_bl_x, exit_end_bl_y, 0);
	gp_Pnt three_five(exit_end_x + (settings.radius + settings.boundary_layer_thickness) * cos(exit_half_angle), exit_end_y + (settings.radius + settings.boundary_layer_thickness) * sin(exit_half_angle), 0.0);
	gp_Pnt three_six(exit_end_x + settings.radius + settings.boundary_layer_thickness, exit_end_y, 0);

	gp_Pnt four_zero(exit_end_x, exit_end_y + settings.wall_thickness, 0);
	gp_Pnt four_one(exit_end_x + settings.radius, exit_end_y + settings.wall_thickness, 0);
	gp_Pnt four_two(exit_end_x + 0.707106 * settings.radius, exit_end_y + settings.wall_thickness + 0.707106 * settings.radius, 0);
	gp_Pnt four_three(exit_end_x, exit_end_y + settings.radius + settings.wall_thickness, 0); //+ top_offset_x
	gp_Pnt four_four(exit_end_x + settings.radius + settings.boundary_layer_thickness, exit_end_y + settings.wall_thickness, 0);
	gp_Pnt four_five(exit_end_x + 0.707106 * (settings.radius + settings.boundary_layer_thickness), exit_end_y + settings.wall_thickness + 0.707106 * (settings.radius + settings.boundary_layer_thickness), 0);
	gp_Pnt four_six(exit_end_x, exit_end_y + settings.radius + settings.boundary_layer_thickness + settings.wall_thickness, 0);

	gp_Pnt five_zero(0.0, 0.0, 0.0);
	gp_Pnt five_one(exit_end_x,settings.outlet_height1, 0.0);
	gp_Pnt five_two(exit_end_x + 0.707106 * (settings.radius + settings.boundary_layer_thickness), settings.outlet_height1, 0.0);
	gp_Pnt five_three(settings.inlet_length + settings.exit_length + settings.domain_length, settings.outlet_height1, 0.0);
	gp_Pnt five_four(settings.inlet_length + settings.exit_length + settings.domain_length, exit_end_y + settings.wall_thickness + 0.707106 * (settings.radius + settings.boundary_layer_thickness), 0);
	gp_Pnt five_five(settings.inlet_length + settings.exit_length + settings.domain_length, exit_end_y + settings.wall_thickness, 0);
	gp_Pnt five_six(settings.inlet_length + settings.exit_length + settings.domain_length, exit_end_y, 0);
	gp_Pnt five_seven(settings.inlet_length + settings.exit_length + settings.domain_length, exit_end_y + (settings.radius + settings.boundary_layer_thickness) * sin(exit_half_angle), 0.0);
	gp_Pnt five_eight(settings.inlet_length + settings.exit_length + settings.domain_length, 0.0, 0.0);
	gp_Pnt five_nine(exit_end_x + (settings.radius + settings.boundary_layer_thickness) * cos(exit_half_angle), 0.0, 0.0);
	gp_Pnt five_ten(exit_end_x, 0.0, 0.0);
	gp_Pnt five_eleven(exit_start_bl_x, 0.0, 0.0);
	gp_Pnt five_twelve(inlet_end_bl_x, 0.0, 0.0);


	// Arc Helper Points, for the boundary layer
	gp_Pnt three_seven(exit_end_x + (settings.radius + settings.boundary_layer_thickness) * cos(exit_end_angle * 0.75), exit_end_y + (settings.radius + settings.boundary_layer_thickness) * sin(exit_end_angle * 0.75), 0.0);
	gp_Pnt three_eight(exit_end_x + (settings.radius + settings.boundary_layer_thickness) * cos(exit_end_angle * 0.25), exit_end_y + (settings.radius + settings.boundary_layer_thickness) * sin(exit_end_angle * 0.25), 0.0);
	gp_Pnt three_nine(exit_end_x + settings.radius * cos(exit_end_angle * 0.25), exit_end_y + settings.radius * sin(exit_end_angle * 0.25), 0.0);
	gp_Pnt three_ten(exit_end_x + settings.radius * cos(exit_end_angle * 0.75), exit_end_y + settings.radius * sin(exit_end_angle* 0.75), 0.0);

	gp_Pnt four_seven(exit_end_x + 0.3826834 * (settings.radius + settings.boundary_layer_thickness), exit_end_y + settings.wall_thickness + 0.9238795 * (settings.radius + settings.boundary_layer_thickness), 0);
	gp_Pnt four_eight(exit_end_x + 0.9238795 * (settings.radius + settings.boundary_layer_thickness), exit_end_y + settings.wall_thickness + 0.3826834 * (settings.radius + settings.boundary_layer_thickness), 0);
	gp_Pnt four_nine(exit_end_x + 0.3826834 * settings.radius, exit_end_y + settings.wall_thickness + 0.9238795 * settings.radius, 0);
	gp_Pnt four_ten(exit_end_x + 0.9238795 * settings.radius, exit_end_y + settings.wall_thickness + 0.3826834 * settings.radius, 0);





	std::vector<gp_Pnt> points =
		{
			one_zero,
			one_one,

			two_zero,
			two_one,
			two_two,
			two_three,
			two_four,
			two_five,
			two_six,

			three_zero,
			three_one,
			three_two,
			three_three,
			three_four,
			three_five,
			three_six,
			three_seven,
			three_eight,
			three_nine,
			three_ten,

			four_zero,
			four_one,
			four_two,
			four_three,
			four_four,
			four_five,
			four_six,
			four_seven,
			four_eight,
			four_nine,
			four_ten,

			five_zero,
			five_one,
			five_two,
			five_three,
			five_four,
			five_five,
			five_six,
			five_seven,
			five_eight,
			five_nine,
			five_ten,
			five_eleven,
			five_twelve
		};



	TColgp_Array1OfPnt one_pts(1, settings.interpolation_density);
	TColgp_Array1OfPnt two_pts(1,settings.interpolation_density);
	TColgp_Array1OfPnt three_pts(1, settings.interpolation_density);
	TColgp_Array1OfPnt four_pts(1, settings.interpolation_density);

	for (Standard_Integer i = 0; i < settings.interpolation_density; ++i)
	{
    	one_pts.SetValue(i + 1, gp_Pnt(inlet_sampled_x[i], inlet_sampled_y[i], 0.0));
		two_pts.SetValue(i + 1, gp_Pnt(inlet_sampled_bl_x[i], inlet_sampled_bl_y[i], 0.0));
		three_pts.SetValue(i + 1, gp_Pnt(exit_sampled_x[i], exit_sampled_y[i], 0.0));
		four_pts.SetValue(i + 1, gp_Pnt(exit_sampled_bl_x[i], exit_sampled_bl_y[i], 0.0));
	}

	GeomAPI_PointsToBSpline one_builder(one_pts);
	GeomAPI_PointsToBSpline two_builder(two_pts);
	GeomAPI_PointsToBSpline three_builder(three_pts);
	GeomAPI_PointsToBSpline four_builder(four_pts);


	Handle(Geom_BSplineCurve) one_spline = one_builder.Curve();
	TopoDS_Edge one = BRepBuilderAPI_MakeEdge(one_spline);

	Handle(Geom_BSplineCurve) two_spline = two_builder.Curve();
	TopoDS_Edge two = BRepBuilderAPI_MakeEdge(two_spline);

	Handle(Geom_BSplineCurve) three_spline = three_builder.Curve();
	TopoDS_Edge three = BRepBuilderAPI_MakeEdge(three_spline);

	Handle(Geom_BSplineCurve) four_spline = four_builder.Curve();
	TopoDS_Edge four = BRepBuilderAPI_MakeEdge(four_spline);



	TopoDS_Compound compound;
	BRep_Builder builder;
	builder.MakeCompound(compound);

	for (const auto &p : points)
	{
		TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(p);
		builder.Add(compound, v);
	}

	Handle(Geom_TrimmedCurve) arc1 = GC_MakeArcOfCircle(two_one, two_two, two_three);
	Handle(Geom_TrimmedCurve) arc2 = GC_MakeArcOfCircle(two_four, two_five, two_six);
	Handle(Geom_TrimmedCurve) arc3 = GC_MakeArcOfCircle(three_one, three_ten ,three_two);
	Handle(Geom_TrimmedCurve) arc4 = GC_MakeArcOfCircle(three_four, three_seven ,three_five);
	Handle(Geom_TrimmedCurve) arc5 = GC_MakeArcOfCircle(three_two, three_nine, three_three);
	Handle(Geom_TrimmedCurve) arc6 = GC_MakeArcOfCircle(three_five, three_eight, three_six);
	Handle(Geom_TrimmedCurve) arc7 = GC_MakeArcOfCircle(four_one, four_ten, four_two);
	Handle(Geom_TrimmedCurve) arc8 = GC_MakeArcOfCircle(four_two, four_nine, four_three);
	Handle(Geom_TrimmedCurve) arc9 = GC_MakeArcOfCircle(four_four, four_eight, four_five);
	Handle(Geom_TrimmedCurve) arc10 = GC_MakeArcOfCircle(four_five, four_seven, four_six);

	TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(five_zero, one_zero);
	TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(one_zero, one_one);
	TopoDS_Edge edge6 = BRepBuilderAPI_MakeEdge(five_one, four_six);
	TopoDS_Edge edge7 = BRepBuilderAPI_MakeEdge(three_three, four_one);
	TopoDS_Edge edge8 = BRepBuilderAPI_MakeEdge(three_six, four_four);
	TopoDS_Edge edge9 = BRepBuilderAPI_MakeEdge(four_three, four_six);
	TopoDS_Edge edge10 = BRepBuilderAPI_MakeEdge(arc1);
	TopoDS_Edge edge11 = BRepBuilderAPI_MakeEdge(arc2);
	TopoDS_Edge edge12 = BRepBuilderAPI_MakeEdge(arc3);
	TopoDS_Edge edge13 = BRepBuilderAPI_MakeEdge(arc4);
	TopoDS_Edge edge14 = BRepBuilderAPI_MakeEdge(arc5);
	TopoDS_Edge edge15 = BRepBuilderAPI_MakeEdge(arc6);

	TopoDS_Edge edge16 = BRepBuilderAPI_MakeEdge(two_one, two_four);
	TopoDS_Edge edge17 = BRepBuilderAPI_MakeEdge(two_three, two_six);

	TopoDS_Edge edge18 = BRepBuilderAPI_MakeEdge(three_one, three_four);
	TopoDS_Edge edge19 = BRepBuilderAPI_MakeEdge(three_three, three_six);

    TopoDS_Edge edge20 = BRepBuilderAPI_MakeEdge(four_one, four_four);

	TopoDS_Edge edge21 = BRepBuilderAPI_MakeEdge(two_four, five_twelve);
	TopoDS_Edge edge22 = BRepBuilderAPI_MakeEdge(five_twelve, five_zero);
	TopoDS_Edge edge23 = BRepBuilderAPI_MakeEdge(two_six, five_eleven);
	TopoDS_Edge edge24 = BRepBuilderAPI_MakeEdge(five_eleven, five_twelve);
	TopoDS_Edge edge25 = BRepBuilderAPI_MakeEdge(five_nine, five_ten);
	TopoDS_Edge edge26 = BRepBuilderAPI_MakeEdge(five_ten, five_eleven);
	TopoDS_Edge edge27 = BRepBuilderAPI_MakeEdge(three_five, five_nine);
	TopoDS_Edge edge28 = BRepBuilderAPI_MakeEdge(three_four, five_ten);
	TopoDS_Edge edge29 = BRepBuilderAPI_MakeEdge(five_nine, five_eight);
	TopoDS_Edge edge31 = BRepBuilderAPI_MakeEdge(three_five, five_seven);
	TopoDS_Edge edge32 = BRepBuilderAPI_MakeEdge(three_six, five_six);
	TopoDS_Edge edge33 = BRepBuilderAPI_MakeEdge(five_one, five_two);
	TopoDS_Edge edge34 = BRepBuilderAPI_MakeEdge(five_two, five_three);
	TopoDS_Edge edge35 = BRepBuilderAPI_MakeEdge(five_three, five_four);
	TopoDS_Edge edge36 = BRepBuilderAPI_MakeEdge(five_four, five_five);
	TopoDS_Edge edge37 = BRepBuilderAPI_MakeEdge(five_five, five_six);
	TopoDS_Edge edge38 = BRepBuilderAPI_MakeEdge(five_six, five_seven);
	TopoDS_Edge edge39 = BRepBuilderAPI_MakeEdge(five_seven, five_eight);
	TopoDS_Edge edge40 = BRepBuilderAPI_MakeEdge(five_five, four_four);
	TopoDS_Edge edge41 = BRepBuilderAPI_MakeEdge(five_four, four_five);
	TopoDS_Edge edge42 = BRepBuilderAPI_MakeEdge(four_five, five_two);
	TopoDS_Edge edge43 = BRepBuilderAPI_MakeEdge(three_two, three_five);
	TopoDS_Edge edge44 = BRepBuilderAPI_MakeEdge(four_two, four_five);
	TopoDS_Edge edge45 = BRepBuilderAPI_MakeEdge(arc7);
	TopoDS_Edge edge46 = BRepBuilderAPI_MakeEdge(arc8);
	TopoDS_Edge edge47 = BRepBuilderAPI_MakeEdge(arc9);
	TopoDS_Edge edge48 = BRepBuilderAPI_MakeEdge(arc10);




	builder.Add(compound, edge1);
	builder.Add(compound, edge2);
	builder.Add(compound, edge6);
	builder.Add(compound, edge7);
	builder.Add(compound, edge8);
	builder.Add(compound, edge9);
	builder.Add(compound, edge10);
	builder.Add(compound, edge11);
	builder.Add(compound, edge12);
	builder.Add(compound, edge13);
	builder.Add(compound, edge14);
	builder.Add(compound, edge15);
	builder.Add(compound, edge16);
	builder.Add(compound, edge17);
	builder.Add(compound, edge18);
	builder.Add(compound, edge19);
	builder.Add(compound, edge20);
	builder.Add(compound, edge21);
	builder.Add(compound, edge22);
	builder.Add(compound, edge23);
	builder.Add(compound, edge24);
	builder.Add(compound, edge25);
	builder.Add(compound, edge26);
	builder.Add(compound, edge27);
	builder.Add(compound, edge28);
	builder.Add(compound, edge29);
	builder.Add(compound, edge31);
	builder.Add(compound, edge32);
	builder.Add(compound, edge33);
	builder.Add(compound, edge34);
	builder.Add(compound, edge35);
	builder.Add(compound, edge36);
	builder.Add(compound, edge37);
	builder.Add(compound, edge38);
	builder.Add(compound, edge39);
	builder.Add(compound, edge40);
	builder.Add(compound, edge41);
	builder.Add(compound, edge42);
	builder.Add(compound, edge43);
	builder.Add(compound, edge44);
	builder.Add(compound, edge45);
	builder.Add(compound, edge46);
	builder.Add(compound, edge47);
	builder.Add(compound, edge48);


	builder.Add(compound, one);
	builder.Add(compound, two);
	builder.Add(compound, three);
    builder.Add(compound, four);


// Make Wires:
	// First Section:
	BRepBuilderAPI_MakeWire wire1_builder;

	wire1_builder.Add(edge2);
	wire1_builder.Add(one);
	wire1_builder.Add(edge16);
	wire1_builder.Add(two);

	TopoDS_Wire wire1 = wire1_builder.Wire();
	TopoDS_Face face1 = BRepBuilderAPI_MakeFace(wire1);

	// Second Section:
	BRepBuilderAPI_MakeWire wire2_builder;

	wire2_builder.Add(edge10);
	wire2_builder.Add(edge16);
	wire2_builder.Add(edge11);
	wire2_builder.Add(edge17);

	TopoDS_Wire wire2 = wire2_builder.Wire();
	TopoDS_Face face2 = BRepBuilderAPI_MakeFace(wire2);

	// Tertiary Section:
	BRepBuilderAPI_MakeWire wire3_builder;

	wire3_builder.Add(three);
	wire3_builder.Add(edge17);
	wire3_builder.Add(four);
	wire3_builder.Add(edge18);

	TopoDS_Wire wire3 = wire3_builder.Wire();
	TopoDS_Face face3 = BRepBuilderAPI_MakeFace(wire3);

	BRepBuilderAPI_MakeWire wire4_builder;
	wire4_builder.Add(edge1);
	wire4_builder.Add(two);
	wire4_builder.Add(edge21);
	wire4_builder.Add(edge22);


	TopoDS_Wire wire4 = wire4_builder.Wire();
	TopoDS_Face face4 = BRepBuilderAPI_MakeFace(wire4);

	BRepBuilderAPI_MakeWire wire5_builder;
	wire5_builder.Add(edge21);
	wire5_builder.Add(edge11);
	wire5_builder.Add(edge23);
	wire5_builder.Add(edge24);


	TopoDS_Wire wire5 = wire5_builder.Wire();
	TopoDS_Face face5 = BRepBuilderAPI_MakeFace(wire5);

	BRepBuilderAPI_MakeWire wire6_builder;
	wire6_builder.Add(edge23);
	wire6_builder.Add(four);
	wire6_builder.Add(edge28);
	wire6_builder.Add(edge26);


	TopoDS_Wire wire6 = wire6_builder.Wire();
	TopoDS_Face face6 = BRepBuilderAPI_MakeFace(wire6);

	BRepBuilderAPI_MakeWire wire7_builder;
	wire7_builder.Add(edge12);
	wire7_builder.Add(edge43);
	wire7_builder.Add(edge13);
	wire7_builder.Add(edge18);

	TopoDS_Wire wire7 = wire7_builder.Wire();
	TopoDS_Face face7 = BRepBuilderAPI_MakeFace(wire7);

	BRepBuilderAPI_MakeWire wire8_builder;
	wire8_builder.Add(edge13);
	wire8_builder.Add(edge27);
	wire8_builder.Add(edge25);
	wire8_builder.Add(edge28);

	TopoDS_Wire wire8 = wire8_builder.Wire();
	TopoDS_Face face8 = BRepBuilderAPI_MakeFace(wire8);

	BRepBuilderAPI_MakeWire wire9_builder;
	wire9_builder.Add(edge14);
	wire9_builder.Add(edge19);
	wire9_builder.Add(edge15);
	wire9_builder.Add(edge43);


	TopoDS_Wire wire9 = wire9_builder.Wire();
	TopoDS_Face face9 = BRepBuilderAPI_MakeFace(wire9);

	BRepBuilderAPI_MakeWire wire10_builder;
	wire10_builder.Add(edge7);
	wire10_builder.Add(edge19);
	wire10_builder.Add(edge8);
	wire10_builder.Add(edge20);



	TopoDS_Wire wire10 = wire10_builder.Wire();
	TopoDS_Face face10 = BRepBuilderAPI_MakeFace(wire10);

	BRepBuilderAPI_MakeWire wire11_builder;
	wire11_builder.Add(edge45);
	wire11_builder.Add(edge20);
	wire11_builder.Add(edge47);
	wire11_builder.Add(edge44);


	TopoDS_Wire wire11 = wire11_builder.Wire();
	TopoDS_Face face11 = BRepBuilderAPI_MakeFace(wire11);

	BRepBuilderAPI_MakeWire wire12_builder;
	wire12_builder.Add(edge46);
	wire12_builder.Add(edge44);
	wire12_builder.Add(edge48);
	wire12_builder.Add(edge9);


	TopoDS_Wire wire12 = wire12_builder.Wire();
	TopoDS_Face face12 = BRepBuilderAPI_MakeFace(wire12);

	BRepBuilderAPI_MakeWire wire13_builder;
	wire13_builder.Add(edge48);
	wire13_builder.Add(edge6);
	wire13_builder.Add(edge33);
	wire13_builder.Add(edge42);


	TopoDS_Wire wire13 = wire13_builder.Wire();
	TopoDS_Face face13 = BRepBuilderAPI_MakeFace(wire13);

	BRepBuilderAPI_MakeWire wire14_builder;
	wire14_builder.Add(edge34);
	wire14_builder.Add(edge35);
	wire14_builder.Add(edge41);
	wire14_builder.Add(edge42);


	TopoDS_Wire wire14 = wire14_builder.Wire();
	TopoDS_Face face14 = BRepBuilderAPI_MakeFace(wire14);

	BRepBuilderAPI_MakeWire wire15_builder;
	wire15_builder.Add(edge41);
	wire15_builder.Add(edge36);
	wire15_builder.Add(edge40);
	wire15_builder.Add(edge47);


	TopoDS_Wire wire15 = wire15_builder.Wire();
	TopoDS_Face face15 = BRepBuilderAPI_MakeFace(wire15);

	BRepBuilderAPI_MakeWire wire16_builder;
	wire16_builder.Add(edge40);
	wire16_builder.Add(edge37);
	wire16_builder.Add(edge32);
	wire16_builder.Add(edge8);


	TopoDS_Wire wire16 = wire16_builder.Wire();
	TopoDS_Face face16 = BRepBuilderAPI_MakeFace(wire16);

	BRepBuilderAPI_MakeWire wire17_builder;
	wire17_builder.Add(edge32);
	wire17_builder.Add(edge38);
	wire17_builder.Add(edge31);
	wire17_builder.Add(edge15);


	TopoDS_Wire wire17 = wire17_builder.Wire();
	TopoDS_Face face17 = BRepBuilderAPI_MakeFace(wire17);

	BRepBuilderAPI_MakeWire wire18_builder;
	wire18_builder.Add(edge31);
	wire18_builder.Add(edge39);
	wire18_builder.Add(edge29);
	wire18_builder.Add(edge27);


	TopoDS_Wire wire18 = wire18_builder.Wire();
	TopoDS_Face face18 = BRepBuilderAPI_MakeFace(wire18);




	builder.Add(compound, face1);
	builder.Add(compound, face2);
	builder.Add(compound, face3);
	builder.Add(compound, face4);
	builder.Add(compound, face5);
	builder.Add(compound, face6);
	builder.Add(compound, face7);
	builder.Add(compound, face8);
	builder.Add(compound, face9);
	builder.Add(compound, face10);
	builder.Add(compound, face11);
	builder.Add(compound, face12);
	builder.Add(compound, face13);
	builder.Add(compound, face14);
	builder.Add(compound, face15);
	builder.Add(compound, face16);
	builder.Add(compound, face17);
	builder.Add(compound, face18);




	if (!BRepTools::Write(compound, "data/geometry.brep"))
	{
		std::cerr << "Failed to write BREP file.\n";
	}
}
