import numpy as np
import pandas as pd
from timeit import default_timer as timer
import splinepy
import matplotlib.pyplot as plt

# ------------------------------------------------------------
# Load CSV file (columns: k, eps_0, M_tar)
# ------------------------------------------------------------
df = pd.read_csv("k_eps0_M.csv")

K     = df["k"].astype(np.float64).to_numpy()
EPS_0 = df["eps_0"].astype(np.float64).to_numpy()   # unused (kept for reference)
Mtar  = df["M_tar"].astype(np.float64).to_numpy()

# ------------------------------------------------------------
# Initialize C++ / pybind objects
# ------------------------------------------------------------
cs = splinepy.CrossSection(300.0, 160.0, 1.0, 60000.0)
cc = splinepy.Points([0.0, 0.003, 0.010], [0.0, 180.0, 180.0])
ft = splinepy.Points([0.0, 0.002, 0.004, 0.008], [0.0, 50.0, 50.0, 75.0])

cal = splinepy.SectionCal(cs, cc, ft)

EPS_MAX = 0.008

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------
if __name__ == "__main__":

    # Convert once (exclude conversion overhead from timing if you want)
    K_list = K.tolist()

    # --------------------------------------------------------
    # Batch solve in C++
    # --------------------------------------------------------
    start = timer()
    sols = cal.solve_eps_ca_for_kappa_batch(K_list, float(EPS_MAX), 1e-10, 80)
    end = timer()

    print(f"C++ batch solve finished in {end-start:.6f} s")
    print(f"returned: {len(sols)} results")

    # --------------------------------------------------------
    # Unpack results
    # --------------------------------------------------------
    N = len(sols)

    ok        = np.fromiter((s.success   for s in sols), dtype=np.bool_,   count=N)
    iters     = np.fromiter((s.iters     for s in sols), dtype=np.int32,   count=N)
    residuals = np.fromiter((s.residual  for s in sols), dtype=np.float64, count=N)

    eps_ca_opt = np.fromiter((s.eps_ca    for s in sols), dtype=np.float64, count=N)
    K_eff      = np.fromiter((s.kappa_eff for s in sols), dtype=np.float64, count=N)
    m_ca       = np.fromiter((s.moment    for s in sols), dtype=np.float64, count=N)

    print(f"success: {int(ok.sum())} / {N}")
    print(f"max iters (ok): {int(iters[ok].max()) if np.any(ok) else None}")

    # --------------------------------------------------------
    # Example diagnostics (first successful point)
    #   (eval() 제거된 버전 기준: residual/moment로 확인)
    # --------------------------------------------------------
    idx_ok = np.flatnonzero(ok)
    if idx_ok.size > 0:
        i0 = int(idx_ok[0])
        r0 = cal.forceresidual(float(eps_ca_opt[i0]), float(K_eff[i0]))
        m0 = cal.moment(float(eps_ca_opt[i0]), float(K_eff[i0]))
        print("example idx:", i0)
        print("  kappa:", float(K[i0]), "k_eff:", float(K_eff[i0]))
        print("  eps_ca:", float(eps_ca_opt[i0]))
        print("  residual(check):", float(r0), "residual(stored):", float(residuals[i0]))
        print("  moment(check):", float(m0), "moment(stored):", float(m_ca[i0]))

    # --------------------------------------------------------
    # Plotting
    # --------------------------------------------------------
    mask = ok & np.isfinite(m_ca) & np.isfinite(Mtar) & (np.abs(Mtar) > 0.0)

    plt.figure()
    plt.plot(K[mask], m_ca[mask], label="C++ batch solver m_ca")
    plt.plot(K, Mtar, "--", label="Target M")
    plt.xlabel("kappa")
    plt.ylabel("Moment")
    plt.legend()
    plt.grid(True)
    plt.show()

    # ratio plot (avoid division by ~0)
    mask_ratio = mask & (np.abs(Mtar) > 1e-12)

    plt.figure()
    plt.plot(K[mask_ratio], (m_ca[mask_ratio] / Mtar[mask_ratio]))
    plt.xlabel("kappa")
    plt.ylabel("m_ca / Mtar")
    plt.grid(True)
    plt.show()

    # --------------------------------------------------------
    # Diagnostics
    # --------------------------------------------------------
    print("Mtar min/max:", float(np.min(Mtar)), float(np.max(Mtar)))
    print("count |Mtar|<1e-12:", int(np.sum(np.abs(Mtar) < 1e-12)))
    print("fraction clipped:", float(np.mean(np.abs(K_eff - K) > 1e-15)))
    print("max |K_eff-K|:", float(np.max(np.abs(K_eff - K))))
    print("max |residual| (ok):", float(np.max(np.abs(residuals[ok]))) if np.any(ok) else None)
    print("max |residual| (all):", float(np.max(np.abs(residuals[np.isfinite(residuals)]))) if np.any(np.isfinite(residuals)) else None)
