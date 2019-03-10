// Simple Spacer for the Solar Beacon Antenna

$fn=50; // fragments
outer = 19.0; // Spacer outer diameter (mm)
inner = 6.75; // Diameter of hole for SMA connector
thickness = 3.0; // Total thickness of spacer and PCB
PCB_thickness = 1.55; // PCB thickness - subtracted from 'thickness' - change to 1.0mm if required

module spacer()
{
    difference() {
        cylinder(h=(thickness - PCB_thickness), r=(outer / 2));
        translate([0, 0, (0-(PCB_thickness / 2))])
            cylinder(h=(thickness), r=(inner / 2));
    }
}

spacer();
