# tests/test_curve_consistency.py

import numpy as np
import pytest
from scipy import interpolate
import time

import splinepy
import spline_ref
import rand_graph_gen

N_TESTS = 3000   # iterations for random tests


# Global accumulators for total timing
total_simd = 0.0
total_reg = 0.0
total_ref = 0.0


def python_ref_area(eps, ilm , sig, arr):

    lm = spline_ref.prep(eps[-1], arr, ilm)

    area = spline_ref.m0_reduced(lm)
    return area


@pytest.mark.parametrize("i", range(N_TESTS))
def test_random_curve_area_momentum_match(i):
    global total_simd, total_reg, total_ref

    # 1. Generate random eps-sig
    n_points = 10000
    n_keypoints = np.random.randint(3, 10)

    pairs = rand_graph_gen.generate_random_points(
        n_points=n_points,
        n_keypoints=n_keypoints,
        eps_min=0.1,
        eps_max=100.0,
        sig_min=0.1,
        sig_max=100000.0,
    )

    eps = [p[0] for p in pairs]
    sig = [p[1] for p in pairs]
    
    
    points = splinepy.Points(eps, sig)
    # ---- Regular C++ ----
    t0 = time.perf_counter()
    pts = splinepy.preprocess(eps[-1], points)
    a_reg = splinepy.cal_area(pts)
    total_reg += time.perf_counter() - t0


    eps_np = np.asarray(eps, dtype=float)
    sig_np = np.asarray(sig, dtype=float)
    
    arr = np.column_stack((eps_np, sig_np))
    
    ilm = interpolate.interp1d(
        eps_np,
        sig_np,
        kind="linear",
        assume_sorted=True,
        bounds_error=True,
    )

    
    # ---- Python Reference ----
    t0 = time.perf_counter()
    a_ref = python_ref_area(eps_np ,ilm, sig_np, arr)
    total_ref += time.perf_counter() - t0

    # 3. Compare results
    rtol = 1e-10
    atol = 1e-10

    assert np.isclose(a_reg, a_ref, rtol=rtol, atol=atol)


    # 4. Only at the final iteration, print total results
    if i == N_TESTS - 1:
        print("\n========== Performance Summary ==========")
        print(f"Total Regular time  : {total_reg:.6f} s")
        print(f"Total Python ref    : {total_ref:.6f} s")
        print("=========================================\n")
