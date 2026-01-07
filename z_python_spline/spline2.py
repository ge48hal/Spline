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
TOL     = 1e-10
MAX_IT  = 80

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------
if __name__ == "__main__":
    # Convert once (exclude conversion overhead from timing if desired)
    K_list = K.tolist()

    # --------------------------------------------------------
    # Batch solve in C++
    # --------------------------------------------------------
    t0 = timer()
    sols = cal.solve_eps_ca_for_kappa_batch(K_list, float(EPS_MAX), float(TOL), int(MAX_IT))
    t1 = timer()

    # Unpack results (needed for plotting)
    N = len(sols)
    ok   = np.fromiter((s.success for s in sols), dtype=np.bool_, count=N)
    m_ca = np.fromiter((s.moment  for s in sols), dtype=np.float64, count=N)

    # --------------------------------------------------------
    # Plotting (only)
    # --------------------------------------------------------
    t2 = timer()

    # Mask: successful + finite + meaningful target
    mask = ok & np.isfinite(m_ca) & np.isfinite(Mtar) & (np.abs(Mtar) > 0.0)

    plt.figure()
    plt.plot(K[mask], m_ca[mask], label="C++ batch solver m_ca")
    plt.plot(K, Mtar, "--", label="Target M")
    plt.xlabel("kappa")
    plt.ylabel("Moment")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

    # Ratio plot (avoid division by ~0)
    mask_ratio = mask & (np.abs(Mtar) > 1e-12)

    plt.figure()
    plt.plot(K[mask_ratio], (m_ca[mask_ratio] / Mtar[mask_ratio]))
    plt.xlabel("kappa")
    plt.ylabel("m_ca / Mtar")
    plt.grid(True)
    plt.tight_layout()
    plt.show()

    t3 = timer()

    # --------------------------------------------------------
    # Timing output only
    # --------------------------------------------------------
    print(f"C++ batch solve time: {t1 - t0:.6f} s   (N={N})")
    print(f"Plotting time:        {t3 - t2:.6f} s   (2 figures)")
