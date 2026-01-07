# tests/test_curve_consistency.py

import numpy as np
import pytest
from scipy import interpolate
import time

import splinepy
import spline_ref
import rand_graph_gen

N_TESTS = 3000  # iterations for random tests

# Global accumulators for total timing
total_reg = 0.0
total_ref = 0.0
total_prefix = 0.0


def python_ref_area(eps, sig):
    eps = np.asarray(eps, dtype=float)
    sig = np.asarray(sig, dtype=float)

    arr = np.column_stack((eps, sig))
    ilm = interpolate.interp1d(
        eps,
        sig,
        kind="linear",
        assume_sorted=True,
        bounds_error=True,
    )

    poly = spline_ref.prep(eps[-1], arr, ilm)
    return spline_ref.m0_reduced(poly)


@pytest.mark.parametrize("i", range(N_TESTS))
def test_random_curve_m0_match(i):
    global total_reg, total_ref, total_prefix

    # 1) Generate random eps-sig
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
    eps_cut = float(eps[-1])

    # Build C++ polyline once
    lm = splinepy.Points(eps, sig)


    # ------------------------------------------------------------
    # B) C++ Prefix-sum member API (no polygon rebuild)
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    sh = splinepy.Shoelace(lm)
    cut_pair = splinepy.preprocess_cut_pair(eps_cut, lm)  # (idx, sigma_cut)
    a_pref = sh.calculate_area(eps_cut, cut_pair)
    total_prefix += time.perf_counter() - t0
    
    # ------------------------------------------------------------
    # A) C++ Regular (polygon rebuild)
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    poly = splinepy.preprocess(eps_cut, lm)
    a_reg, _ = splinepy.cal_area_momentum(poly)  # take m0 only
    total_reg += time.perf_counter() - t0


    # ------------------------------------------------------------
    # C) Python Reference (m0 only)
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    a_ref = python_ref_area(eps, sig)
    total_ref += time.perf_counter() - t0

    # 3) Compare results (m0 only)
    rtol = 1e-10
    atol = 1e-10

    assert np.isclose(a_reg, a_ref, rtol=rtol, atol=atol)
    assert np.isclose(a_pref, a_ref, rtol=rtol, atol=atol)
    assert np.isclose(a_pref, a_reg, rtol=rtol, atol=atol)

    # 4) Print totals once
    if i == N_TESTS - 1:
        print("\n========== Performance Summary ==========")
        print(f"Total Regular time  : {total_reg:.6f} s")
        print(f"Total Prefix time   : {total_prefix:.6f} s")
        print(f"Total Python ref    : {total_ref:.6f} s")
        print("=========================================\n")
