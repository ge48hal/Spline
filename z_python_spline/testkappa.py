import numpy as np
import pandas as pd
import pytest
import time

import splinepy
from scipy.interpolate import interp1d


# ============================================================
# Python reference model (trimmed & vectorized)
# ============================================================

class PyCrossSectionRef:
    """
    Minimal Python reference implementation.
    Vectorized over timesteps (len(K)).
    """

    def __init__(self, h: float, num_timesteps: int):
        self.h = float(h)
        self.h_u = 0.5 * self.h
        self.h_d = 0.5 * self.h

        self.EPS = 0
        self.SIG = 1

        self.num_timesteps = int(num_timesteps)
        self.all_timesteps = np.arange(self.num_timesteps)

        self._cc = None
        self._ft = None
        self._cc_i = None
        self._ft_i = None

        # mutable pads (mp() writes into these)
        self._cc_pad = None
        self._ft_pad = None

        # templates to reset pads quickly
        self._cc_pad_template = None
        self._ft_pad_template = None

        self._eps_ca_max = None

    @property
    def eps_ca_max(self):
        return self._eps_ca_max

    @eps_ca_max.setter
    def eps_ca_max(self, v):
        self._eps_ca_max = float(v)

    @property
    def cc(self):
        return self._cc

    @cc.setter
    def cc(self, arr):
        arr = np.asarray(arr, dtype=np.float64)
        self._cc = arr

        # interpolator (assume_sorted=True helps when eps is sorted)
        self._cc_i = interp1d(
            arr[:, 0], arr[:, 1],
            kind="linear",
            bounds_error=True,
            assume_sorted=True,
        )

        pad = np.zeros((self.num_timesteps, arr.shape[0] + 2, 2), dtype=np.float64)
        pad[:, :-2, :] = arr

        self._cc_pad = pad
        self._cc_pad_template = pad.copy()

    @property
    def ft(self):
        return self._ft

    @ft.setter
    def ft(self, arr):
        arr = np.asarray(arr, dtype=np.float64)
        self._ft = arr

        self._ft_i = interp1d(
            arr[:, 0], arr[:, 1],
            kind="linear",
            bounds_error=True,
            assume_sorted=True,
        )

        pad = np.zeros((self.num_timesteps, arr.shape[0] + 2, 2), dtype=np.float64)
        pad[:, :-2, :] = arr

        self._ft_pad = pad
        self._ft_pad_template = pad.copy()

    def reset_pads(self):
        """mp() mutates pads in-place; reset for deterministic repeated evaluation."""
        # copy template back into working buffer
        self._cc_pad[...] = self._cc_pad_template
        self._ft_pad[...] = self._ft_pad_template

    def mp(self, eps_vec, lm, ilm, lm_pad):
        sig = ilm(eps_vec)
        i = np.searchsorted(lm[:, 0], eps_vec, side="left")

        lm_pad[self.all_timesteps, i, 0] = eps_vec
        lm_pad[self.all_timesteps, i, 1] = sig
        lm_pad[self.all_timesteps, i + 1, 0] = eps_vec
        lm_pad[self.all_timesteps, i + 1, 1] = 0.0

        mask = np.arange(lm.shape[0] + 2) > (i[:, None] + 1)
        lm_pad[mask] = 0.0
        return lm_pad

    @staticmethod
    def m0(pad):
        x = pad[:, :, 0]
        z = pad[:, :, 1]
        return 0.5 * np.abs(np.sum(x[:, :-1] * z[:, 1:] - z[:, :-1] * x[:, 1:], axis=1))

    @staticmethod
    def m1(pad):
        x = pad[:, :, 0]
        z = pad[:, :, 1]
        return (1.0 / 6.0) * np.abs(
            np.sum(
                (x[:, :-1] + x[:, 1:])
                * (x[:, :-1] * z[:, 1:] - x[:, 1:] * z[:, :-1]),
                axis=1
            )
        )

    def residual_and_moment(self, eps_ca, kappa):
        eps_ca = np.clip(np.asarray(eps_ca, dtype=np.float64), 0.0, self._eps_ca_max)
        kappa = np.asarray(kappa, dtype=np.float64)

        eps_cc = np.abs(eps_ca - kappa * self.h_u)
        eps_ft = np.abs(eps_ca + kappa * self.h_d)

        eps_cc = np.maximum(eps_cc, 1e-15)
        eps_ft = np.maximum(eps_ft, 1e-15)

        eps_dt = eps_cc + eps_ft
        h_cc = (eps_cc / eps_dt) * self.h
        h_ft = (eps_ft / eps_dt) * self.h

        jac_cc = h_cc / eps_cc
        jac_ft = h_ft / eps_ft

        cc_pad = self.mp(eps_cc, self._cc, self._cc_i, self._cc_pad)
        ft_pad = self.mp(eps_ft, self._ft, self._ft_i, self._ft_pad)

        A_cc = self.m0(cc_pad)
        A_ft = self.m0(ft_pad)
        residual = A_cc * jac_cc - A_ft * jac_ft

        M_cc = self.m1(cc_pad)
        M_ft = self.m1(ft_pad)
        moment = M_cc * jac_cc**2 + M_ft * jac_ft**2

        return residual, moment


# ============================================================
# Fixtures
# ============================================================

@pytest.fixture(scope="session")
def data():
    df = pd.read_csv("k_eps0_M.csv")
    return (
        df["k"].to_numpy(dtype=np.float64),
        df["eps_0"].to_numpy(dtype=np.float64),
        df["M_tar"].to_numpy(dtype=np.float64),
    )


@pytest.fixture(scope="session")
def setup_models(data):
    K, _, _ = data

    cs_cpp = splinepy.CrossSection(300.0, 160.0, 1.0, 60000.0)
    cc_cpp = splinepy.Points([0.0, 0.003, 0.010], [0.0, 180.0, 180.0])
    ft_cpp = splinepy.Points([0.0, 0.002, 0.004, 0.008], [0.0, 50.0, 50.0, 75.0])
    cal = splinepy.SectionCal(cs_cpp, cc_cpp, ft_cpp)

    ref = PyCrossSectionRef(h=300.0, num_timesteps=len(K))
    ref.eps_ca_max = 0.008
    ref.cc = np.array([[0.0, 0.0], [0.003, 180.0], [0.010, 180.0]], dtype=np.float64)
    ref.ft = np.array([[0.0, 0.0], [0.002, 50.0], [0.004, 50.0], [0.008, 75.0]], dtype=np.float64)

    return cal, ref


# ============================================================
# Correctness tests
# ============================================================

def test_batch_solver_success(setup_models, data):
    cal, _ = setup_models
    K, _, _ = data
    sols = cal.solve_eps_ca_for_kappa_batch(K.tolist(), 0.008)
    assert all(s.success for s in sols)


def test_equilibrium_residual_small(setup_models, data):
    cal, _ = setup_models
    K, _, _ = data
    sols = cal.solve_eps_ca_for_kappa_batch(K.tolist(), 0.008)
    residuals = np.array([s.residual for s in sols], dtype=np.float64)
    assert np.max(np.abs(residuals)) < 1e-3


def test_cpp_vs_python_reference_moment(setup_models, data):
    cal, ref = setup_models
    K, _, _ = data

    sols = cal.solve_eps_ca_for_kappa_batch(K.tolist(), 0.008)

    eps_ca = np.array([s.eps_ca for s in sols], dtype=np.float64)
    k_eff  = np.array([s.kappa_eff for s in sols], dtype=np.float64)
    m_cpp  = np.array([s.moment for s in sols], dtype=np.float64)

    # reset pads before eval to avoid mp() history effects
    ref.reset_pads()
    _, m_py = ref.residual_and_moment(eps_ca, k_eff)

    rel = np.abs(m_cpp - m_py) / np.maximum(1.0, np.abs(m_py))
    assert np.max(rel) < 1e-10


# ============================================================
# Performance test (100 repetitions)
# ============================================================

@pytest.mark.performance
def test_speed_cpp_batch_vs_python_reference(setup_models, data):
    cal, ref = setup_models
    K, _, _ = data
    EPS_MAX = 0.008
    NREP = 1000

    cpp_times = []
    py_times = []

    K_list = K.tolist()

    # warm-up
    cal.solve_eps_ca_for_kappa_batch(K_list, EPS_MAX)
    ref.reset_pads()
    # small warm-up eval using a dummy extraction
    sols0 = cal.solve_eps_ca_for_kappa_batch(K_list, EPS_MAX)
    eps0 = np.array([s.eps_ca for s in sols0], dtype=np.float64)
    ke0  = np.array([s.kappa_eff for s in sols0], dtype=np.float64)
    ref.residual_and_moment(eps0, ke0)

    for i in range(NREP):
        # ---- C++ solve timing ----
        t0 = time.perf_counter()
        sols = cal.solve_eps_ca_for_kappa_batch(K_list, EPS_MAX)
        t1 = time.perf_counter()
        cpp_times.append(t1 - t0)

        # Extract results (not included in Python timing; included in overall test overhead)
        eps_ca = np.array([s.eps_ca for s in sols], dtype=np.float64)
        k_eff  = np.array([s.kappa_eff for s in sols], dtype=np.float64)

        # ---- Python eval timing ----
        # mp() mutates pads, so reset EACH iteration for consistent work
        ref.reset_pads()
        t2 = time.perf_counter()
        ref.residual_and_moment(eps_ca, k_eff)
        t3 = time.perf_counter()
        py_times.append(t3 - t2)

        print(f"[{i:03d}] C++: {cpp_times[-1]:.6f}s | Py: {py_times[-1]:.6f}s")

    print("\n=== Summary (100 runs) ===")
    print("C++ batch mean:", float(np.mean(cpp_times)))
    print("Python eval mean:", float(np.mean(py_times)))
    print("Speedup (Py/C++):", float(np.mean(py_times) / np.mean(cpp_times)))

    assert np.all(np.isfinite(cpp_times))
    assert np.all(np.isfinite(py_times))
