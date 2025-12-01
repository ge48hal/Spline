# Spline Toolkit

A small C++20 project for reading ε–σ data, preprocessing a polyline at a given strain cutoff, constructing a closed polygon, and computing geometric quantities (moments) using shoelace.

src

├── geom/           # geometric utilities (shoelace.)

├── inputreader/    # file parsing utilities ( polyline input)

├── points/         # Points class (ε-σ points class)

├── simulation/     # computation pipeline for each time slice

└── Spline.cpp      # main executable / example usage

test

## Instructions

### Requirements

cmake version 3.28.3, C++ 20,g++ 13.2.0,clang version 18.1.3

### Build and Run

`mkdir build`
`cd build `
`cmake ..`
`make `
`./Spline`

Run tests

`ctest` or `./tests`
