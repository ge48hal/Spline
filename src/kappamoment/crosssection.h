#pragma once
#include <cmath>

struct CrossSection {
    enum class CrossSectionType { SQUARE, TRAPEZOID };

    double height_mm;
    double length_mm;
    CrossSectionType cs_type;

    explicit CrossSection(double h,
                          double l = 160.0,
                          CrossSectionType type = CrossSectionType::SQUARE)
        : height_mm(h), length_mm(l), cs_type(type) {}

    virtual ~CrossSection() = default;

    // ---- common geometry ----

    // Second moment of area (centroidal)
    virtual double I_mm4() const = 0;

    // Distance from neutral axis to top fiber
    virtual double h_u_mm() const = 0;

    // Distance from neutral axis to bottom fiber
    virtual double h_d_mm() const = 0;

    // ---- trapezoid-related (also virtual, same names as before) ----

    // Width slope (db/dz)
    virtual double slope() const = 0;

    // Neutral axis position measured from bottom
    virtual double z_na_mm() const = 0;
};

struct Square : CrossSection {
    double b_mm;

    explicit Square(double h, double b = 1.0, double l = 160.0)
        : CrossSection(h, l, CrossSectionType::SQUARE),
        b_mm(b) {}

    double I_mm4() const override {
        return b_mm * height_mm * height_mm * height_mm / 12.0;
    }

    double h_u_mm() const override {
        return 0.5 * height_mm;
    }

    double h_d_mm() const override {
        return 0.5 * height_mm;
    }

    // Degenerate trapezoid behavior 
    double slope() const override {
        return 0.0;
    }

    double z_na_mm() const override {
        return 0.5 * height_mm;
    }
};

struct Trapezoid : CrossSection {
    double blo_mm;
    double bhi_mm;

    explicit Trapezoid(double h, double b_lo, double b_hi, double l = 160.0)
        : CrossSection(h, l, CrossSectionType::TRAPEZOID),
          blo_mm(b_lo),
          bhi_mm(b_hi) {}

    double slope() const override {
        return (blo_mm - bhi_mm) / height_mm;
    }

    double z_na_mm() const override {
        return height_mm * (2.0 * blo_mm + bhi_mm)
             / (3.0 * (blo_mm + bhi_mm));
    }

    double h_d_mm() const override {
        return z_na_mm();
    }

    double h_u_mm() const override {
        return height_mm - z_na_mm();
    }

    double I_mm4() const override {
        return 0.0;
    }
};
