SetFactory("OpenCASCADE");
Merge "../data/geometry.brep";

Geometry.Tolerance = 1e-8;
globalScale = 1e-2;
Coherence;
Mesh.ScalingFactor = 1e-2;


min_size = 0.001;
Mesh.CharacteristicLengthMin = min_size ;
Mesh.CharacteristicLengthMax = 0.01;


number_divisions = 2500;
number_lateral_divisons = 50;
arc_div =  8;
n_bl = 25;
bl_progression  = 1.05;


Transfinite Curve {-103, 105} = n_bl Using Progression bl_progression;
Transfinite Curve {104, 106} = number_divisions;
Transfinite Surface {1};

Transfinite Curve {105, 109} = n_bl Using Progression bl_progression;
Transfinite Curve {107, 108} = 3;
Transfinite Surface {2};

Transfinite Curve {109, 112} = n_bl Using Progression bl_progression;
Transfinite Curve {110, 111} = number_divisions;
Transfinite Surface {3};

Transfinite Curve {113, 114} = number_lateral_divisons;
Transfinite Curve {115, 106} = number_divisions;
Transfinite Surface {4};

Transfinite Curve {114, 116} = number_lateral_divisons;
Transfinite Curve {108, 117} = 3;
Transfinite Surface {5};

Transfinite Curve {116, 118} = number_lateral_divisons;
Transfinite Curve {111, 119} = number_divisions;
Transfinite Surface {6};

Transfinite Curve {112, 121} = n_bl Using Progression bl_progression;
Transfinite Curve {120, 122} = arc_div;
Transfinite Surface {7};

Transfinite Curve {118, 123} = number_lateral_divisons;
Transfinite Curve {122, 124} = arc_div;
Transfinite Surface {8};

Transfinite Curve {121, 126} = n_bl Using Progression bl_progression;
Transfinite Curve {125, 127} = arc_div;
Transfinite Surface {9};

Transfinite Curve {126, 130} = n_bl Using Progression bl_progression;
Transfinite Curve {128, 129} = arc_div * 2;
Transfinite Surface {10};

Transfinite Curve {130, 133} = n_bl Using Progression bl_progression;
Transfinite Curve {131, 132} = arc_div;
Transfinite Surface {11};

Transfinite Curve {133, 136} = n_bl Using Progression bl_progression;
Transfinite Curve {134, 135} = arc_div;
Transfinite Surface {12};

Transfinite Curve {137, 139} = 5 * number_lateral_divisons;
Transfinite Curve {135, 138} = arc_div;
Transfinite Surface {13};

Transfinite Curve {139, 141} = 5 * number_lateral_divisons;
Transfinite Curve {140, 142} = number_lateral_divisons * 40;
Transfinite Surface {14};


Transfinite Curve {142, 144} = number_lateral_divisons * 40;
Transfinite Curve {132, 143} = arc_div;
Transfinite Surface {15};

Transfinite Curve {144, 146} = number_lateral_divisons * 40 ;
Transfinite Curve {129, 145} = arc_div * 2;
Transfinite Surface {16};

Transfinite Curve {146, 148} = number_lateral_divisons * 40 ;
Transfinite Curve {127, 147} = arc_div;
Transfinite Surface {17};

Transfinite Curve {148, 150} = number_lateral_divisons * 40;
Transfinite Curve {123, 149} = number_lateral_divisons;
Transfinite Surface {18};





Recombine Surface {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};



//  Extrude {{1, 0, 0}, {0, 0, 0}, Pi/80} {
//  Surface{1, 2, 3, 4, 5, 6, 7};
//  Layers{1};
//  Recombine;
//  }


//+
Physical Surface("domain", 151) = {1, 4, 2, 5, 3, 6, 8, 7, 9, 17, 10, 11, 12, 13, 14, 15, 16, 18};
//+
Physical Curve("wall", 152) = {104, 107, 110, 120, 125, 128, 131, 134};
//+
Physical Curve("outlet", 153) = {136, 137, 138, 140, 141, 143, 145, 147, 149};
//+
Physical Curve("axis", 154) = {150, 124, 119, 117, 115};
//+
Physical Curve("inlet", 155) = {113};

Mesh 2;
Mesh.MshFileVersion = 2.2;
Save "../data/mesh.msh";

