# tests/test_curve_consistency.py

import numpy as np
import pytest
from scipy import interpolate
import time

import splinepy
import spline_ref
import rand_graph_gen

N_TESTS = 300   # iterations for random tests

# Global accumulators for total timing
total_simd = 0.0
total_reg = 0.0
total_ref = 0.0
total_prefix = 0.0  # Shoelace(prefix) member API


def python_ref_area_momentum(eps, sig, ilm):
    # ref prep expects: eps_cut, arr(eps,sig), interpolator
    arr = np.column_stack((eps, sig))
    poly = spline_ref.prep(eps[-1], arr, ilm)
    area = spline_ref.m0_reduced(poly)
    mom = spline_ref.m1_reduced(poly)
    return area, mom


@pytest.mark.parametrize("i", range(N_TESTS))
def test_random_curve_area_momentum_match(i):
    global total_simd, total_reg, total_ref, total_prefix

    # 1) Generate random eps-sig
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
    eps_cut = float(eps[-1])

    # ------------------------------------------------------------
    # A) C++ SIMD (static API, polygon input)
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    lm = splinepy.Points(eps, sig)
    poly = splinepy.preprocess(eps_cut, lm)
    a_simd, b_simd = splinepy.cal_area_momentum_simd(poly)
    total_simd += time.perf_counter() - t0


    # ------------------------------------------------------------
    # C) C++ Prefix-sum member API (no polygon rebuild)
    #    - uses (idx, sigma_at_eps_cut) from preprocess_cut_pair
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    lm = splinepy.Points(eps, sig)
    sh = splinepy.Shoelace(lm)
    cut_pair = splinepy.preprocess_cut_pair(eps_cut, lm)  # (idx, sigma_cut)
    a_pref = sh.calculate_area(eps_cut, cut_pair)
    b_pref = sh.calculate_momentum(eps_cut, cut_pair)
    total_prefix += time.perf_counter() - t0

    # ------------------------------------------------------------
    # B) C++ Regular (static API, polygon input)
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    lm = splinepy.Points(eps, sig)
    poly = splinepy.preprocess(eps_cut, lm)
    a_reg, b_reg = splinepy.cal_area_momentum(poly)
    total_reg += time.perf_counter() - t0

    # ------------------------------------------------------------
    # D) Python reference
    # ------------------------------------------------------------
    t0 = time.perf_counter()
    eps_np = np.asarray(eps, dtype=float)
    sig_np = np.asarray(sig, dtype=float)

    ilm = interpolate.interp1d(
        eps_np,
        sig_np,
        kind="linear",
        assume_sorted=True,
        bounds_error=True,
    )

    a_ref, b_ref = python_ref_area_momentum(eps_np, sig_np, ilm)
    total_ref += time.perf_counter() - t0

    # ------------------------------------------------------------
    # 3) Compare results
    # ------------------------------------------------------------
    rtol = 1e-10
    atol = 1e-10

    # SIMD vs Regular
    assert np.isclose(a_simd, a_reg, rtol=rtol, atol=atol)
    assert np.isclose(b_simd, b_reg, rtol=rtol, atol=atol)

    # Prefix vs Static
    assert np.isclose(a_pref, a_ref, rtol=rtol, atol=atol)
    assert np.isclose(b_pref, b_ref, rtol=rtol, atol=atol)

    # C++ vs Python ref
    assert np.isclose(a_reg, a_ref, rtol=rtol, atol=atol)
    assert np.isclose(b_reg, b_ref, rtol=rtol, atol=atol)

    # ------------------------------------------------------------
    # 4) Only at the final iteration, print totals
    # ------------------------------------------------------------
    if i == N_TESTS - 1:
        print("\n========== Performance Summary ==========")
        print(f"Total SIMD time       : {total_simd:.6f} s")
        print(f"Total Regular time    : {total_reg:.6f} s")
        print(f"Total Prefix(member)  : {total_prefix:.6f} s")
        print(f"Total Python ref      : {total_ref:.6f} s")
        print("=========================================\n")
