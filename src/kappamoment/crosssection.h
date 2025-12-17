#pragma once
#include <cmath>

struct CrossSection {
    double length_mm;
    double height_mm;
    double b_mm;
    double E_mpa;

    explicit CrossSection(double h, double l = 160.0,double b = 1.0, double E = 60000.0)
        : length_mm(l), height_mm(h), b_mm(b), E_mpa(E) {}

    double h_u_mm() const { return 0.5 * height_mm; }
    double h_d_mm() const { return 0.5 * height_mm; }

    double I_mm4() const { return b_mm * height_mm * height_mm * height_mm / 12.0; }
    double W_mm3() const { return b_mm * height_mm * height_mm / 6.0; }
};
