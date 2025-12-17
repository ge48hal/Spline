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


def python_ref_area_momentum(eps,ilm, sig):


    arr = np.column_stack((eps, sig))
    lm = spline_ref.prep(eps[-1], arr, ilm)

    area = spline_ref.m0_reduced(lm)
    mom = spline_ref.m1_reduced(lm)
    return area, mom


@pytest.mark.parametrize("i", range(N_TESTS))
def test_random_curve_area_momentum_match(i):
    global total_simd, total_reg, total_ref

    # 1. Generate random eps-sig
    n_points = 10000
    n_keypoints = np.random.randint(3, 100)

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
    


    # ---- SIMD ----
    t0 = time.perf_counter()
    points = splinepy.Points(eps, sig)
    pts = splinepy.preprocess(eps[-1], points)
    a_simd, b_simd = splinepy.cal_area_momentum_simd(pts)
    total_simd += time.perf_counter() - t0


    # ---- Regular C++ ----
    t0 = time.perf_counter()
    points = splinepy.Points(eps, sig)
    pts = splinepy.preprocess(eps[-1], points)
    a_reg, b_reg = splinepy.cal_area_momentum(pts)
    total_reg += time.perf_counter() - t0
    

    # ---- Python Reference ----
    t0 = time.perf_counter()
    eps = np.asarray(eps, dtype=float)
    sig = np.asarray(sig, dtype=float)
    
    ilm = interpolate.interp1d(
        eps,
        sig,
        kind="linear",
        assume_sorted=True,
        bounds_error=True,
    )

    a_ref, b_ref = python_ref_area_momentum(eps,ilm, sig)
    total_ref += time.perf_counter() - t0

    # 3. Compare results
    rtol = 1e-10
    atol = 1e-10

    assert np.isclose(a_simd, a_reg, rtol=rtol, atol=atol)
    assert np.isclose(a_simd, a_ref, rtol=rtol, atol=atol)

    assert np.isclose(b_simd, b_reg, rtol=rtol, atol=atol)
    assert np.isclose(b_simd, b_ref, rtol=rtol, atol=atol)

    # 4. Only at the final iteration, print total results
    if i == N_TESTS - 1:
        print("\n========== Performance Summary ==========")
        print(f"Total SIMD time     : {total_simd:.6f} s")
        print(f"Total Regular time  : {total_reg:.6f} s")
        print(f"Total Python ref    : {total_ref:.6f} s")
        print("=========================================\n")
