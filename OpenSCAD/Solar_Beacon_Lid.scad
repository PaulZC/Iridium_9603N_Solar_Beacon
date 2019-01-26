// Simple Lid for the Iridium Beacon Cover without Radio Board

$fn=50; // fragments
wall = 2; // cover wall thickness
thickness = 2; // lid end thickness
internal_width = 42; // internal width (X) of the cover
internal_depth = 23.5; // internal depth (Y) of the cover
gap = 0.3; // clearance gap (per side)
insert_height = 3; // extra height for the lid insert
internal_corner_radius = 1.0; // radius of the internal corner cylinder

sma_r = 3.75; // radius of the hole for the sma connector
sma_x_offset = 1.5; // X offset of the hole for the sma connector w.r.t. the X center of the void
sma_y_offset = 2.25; // y offset of the hole for the sma connector w.r.t. the Y center of the void
sma_wall_thickness = 3.0; // wall thickness around the SMA connectors

if (gap > internal_corner_radius) echo("Error! Gap cannot be greater than the internal corner radius!");

width = internal_width + (2 * wall); // lid external width (X)
depth = internal_depth + (2 * wall); // lid external depth (Y)

total_height = wall + insert_height; // total height of the lid

sma_x = (width / 2) + sma_x_offset; // x position of the sma connector
sma_y = (depth / 2) + sma_y_offset; // y position of the sma connector

external_corner_radius = internal_corner_radius + wall; // external corner radius

module outer()
{
    translate([external_corner_radius, external_corner_radius, 0])
        minkowski() {
            cube([(width - (2 * external_corner_radius)), (depth - (2 * external_corner_radius)), (thickness / 2)]);
            cylinder(h=(thickness / 2), r=external_corner_radius);
        }
}

module inner()
// Thicker than required to avoid zero thickness joints
{
    translate([external_corner_radius, external_corner_radius, 0])
        minkowski() {
            cube([(width - (2 * external_corner_radius)), (depth - (2 * external_corner_radius)), (total_height / 2)]);
            cylinder(h=(total_height / 2), r=(internal_corner_radius - gap));
        }
}

module recess()
// Thicker than required to avoid zero thickness joints
{
    translate([(internal_corner_radius + wall + wall), (internal_corner_radius + wall + wall), sma_wall_thickness])
        minkowski() {
            cube([(width - (2 * (wall + wall + internal_corner_radius))), (depth - (2 * (wall + wall + internal_corner_radius))), (total_height / 2)]);
            cylinder(h=(total_height / 2), r=internal_corner_radius);
        }
}

module lid()
{
    union() {
        outer();
        inner();
    }
}

module sma()
// Cylinder is taller than required to avoid zero thickness skins
{
    translate([sma_x, sma_y, -1]) {
        cylinder(h=(total_height + 2),r=sma_r);
    }
}

module pcb_slot()
// Cube is taller amd wider than required to avoid zero thickness skins
{
    translate([0, (sma_y - sma_r), sma_wall_thickness]) {
        cube([width, (sma_r * 2), total_height]);
    }
}

module finished_lid()
{
    difference() {
        lid();
        sma();
        recess();
        pcb_slot();
    }
}

finished_lid();
