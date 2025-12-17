import numpy as np
import pandas as pd
from scipy.optimize import newton
from timeit import default_timer as timer
import splinepy
import matplotlib.pyplot as plt

# ------------------------------------------------------------
# Load CSV file (columns: K, EPS_0, Mtar)
# ------------------------------------------------------------
df = pd.read_csv("k_eps0_M.csv")

K = df["k"].astype(np.float64).to_numpy()
EPS_0 = df["eps_0"].astype(np.float64).to_numpy()
Mtar = df["M_tar"].astype(np.float64).to_numpy()

eps_ca_max = float(EPS_0.max())  # Data-based upper bound (for reference)

# ------------------------------------------------------------
# Initialize C++ / pybind objects
# ------------------------------------------------------------
cs = splinepy.CrossSection(300.0, 160.0, 1.0, 60000.0)

cc = splinepy.Points([0.0, 0.003, 0.010], [0.0, 180.0, 180.0])
ft = splinepy.Points([0.0, 0.002, 0.004, 0.008], [0.0, 50.0, 50.0, 75.0])

cal = splinepy.SectionCal(cs, cc, ft)

# Upper bound for eps_cut allowed by preprocess
# (current tensile material curve is defined up to 0.008)
EPS_MAX = 0.008

h_u = float(cs.h_u_mm())  # Upper half height (150)
h_d = float(cs.h_d_mm())  # Lower half height (150)

# ------------------------------------------------------------
# Residual function for Newton solver (uses C++ forceresidual)
# - eps_ca is clipped
# - kappa is clipped according to eps_ca (temporary workaround)
# ------------------------------------------------------------
def residual_vec(eps_ca_vec, kappa_vec):
    eps_ca_vec = np.asarray(eps_ca_vec, dtype=np.float64)
    kappa_vec  = np.asarray(kappa_vec, dtype=np.float64)


    out = np.empty_like(eps_ca_vec)
    for i in range(eps_ca_vec.size):
        out[i] = cal.forceresidual(float(eps_ca_vec[i]), float(kappa_vec[i]))
    return out


if __name__ == "__main__":
    # Initial guess: starting from EPS_0 (data-based) is usually more stable
    eps_ca_ini = np.clip(EPS_0.copy(), 0.0, EPS_MAX)

    start = timer()
    eps_ca_opt = newton(
        residual_vec,
        eps_ca_ini,
        args=(K,),
        tol=1e-10,
        maxiter=50
    )
    end = timer()

    print(f"Newton finished in {end-start:.6f} s")

    # Use the effective kappa for moment computation
    eps_ca_opt = np.clip(np.asarray(eps_ca_opt, dtype=np.float64), 0.0, EPS_MAX)

    kmax_ft = (EPS_MAX - eps_ca_opt) / h_d
    kmax_cc = (EPS_MAX + eps_ca_opt) / h_u
    kmax = np.minimum(kmax_ft, kmax_cc)
    kmax = np.maximum(kmax, 0.0)
    K_eff = np.clip(np.asarray(K, dtype=np.float64), 0.0, kmax)

    m_ca = np.empty_like(eps_ca_opt)
    for i in range(len(K_eff)):
        m_ca[i] = cal.moment(float(eps_ca_opt[i]), float(K_eff[i]))

    # Example of section state (first data point)
    st0 = cal.eval(float(eps_ca_opt[0]), float(K_eff[0]))

    # ------------------------------------------------------------
    # 📈 Plotting
    # ------------------------------------------------------------

    # 1) Moment–curvature relationship
    plt.figure()
    plt.plot(K, m_ca, label="C++ / pybind m_ca")
    plt.plot(K, Mtar, "--", label="Target M")
    plt.xlabel("kappa")
    plt.ylabel("Moment")
    plt.legend()
    plt.grid(True)
    plt.show()

    # 2) Ratio plot
    plt.figure()
    plt.plot(K, m_ca / Mtar)
    plt.xlabel("kappa")
    plt.ylabel("m_ca / Mtar")
    plt.grid(True)
    plt.show()
