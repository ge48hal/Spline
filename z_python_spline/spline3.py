import numpy as np
import pandas as pd
from timeit import default_timer as timer
import matplotlib.pyplot as plt
from scipy.optimize import newton

import splinepy

# ------------------------------------------------------------
# Load CSV file (columns: k, eps_0, M_tar)
# ------------------------------------------------------------
df = pd.read_csv("k_eps0_M.csv")

K     = df["k"].astype(np.float64).to_numpy()
EPS_0 = df["eps_0"].astype(np.float64).to_numpy()   # 참고용
Mtar  = df["M_tar"].astype(np.float64).to_numpy()

# ------------------------------------------------------------
# Constants
# ------------------------------------------------------------
EPS_MAX = 0.008

# ------------------------------------------------------------
# Initialize C++ objects
# ------------------------------------------------------------
cs = splinepy.CrossSection(300.0, 160.0, 1.0, 60000.0)
cc = splinepy.Points([0.0, 0.003, 0.010], [0.0, 180.0, 180.0])
ft = splinepy.Points([0.0, 0.002, 0.004, 0.008], [0.0, 50.0, 50.0, 75.0])

# ✅ 네가 바꾼 API: eps_max를 생성자에 넣는 버전
cal = splinepy.SectionCal(cs, cc, ft, float(EPS_MAX))

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------
if __name__ == "__main__":

    # (기존 방식) eps_ca 초기 guess를 벡터로 둠
    # 너의 ref처럼 0~eps_max linspace를 쓰거나 EPS_0를 쓰면 됨
    eps_ca_ini = np.linspace(0.0, EPS_MAX, K.size).astype(np.float64)

    # --------------------------------------------------------
    # A) SciPy newton (vector) + C++ objective
    # --------------------------------------------------------
    start = timer()
    eps_ca_opt = newton(
        cal.objective,          # ✅ pybind에서 objective_batch를 objective로 노출한 것
        eps_ca_ini,
        args=(K,),
        tol=1e-10,
        maxiter=50
    )
    end = timer()
    print(f"SciPy newton (vector) finished in {end-start:.6f} s")

    # --------------------------------------------------------
    # B) residual 체크 (원하면)
    # --------------------------------------------------------
    start = timer()
    res_vec = np.asarray(cal.objective(eps_ca_opt, K), dtype=np.float64)
    end = timer()
    print(f"residual eval finished in {end-start:.6f} s")
    print("residual stats:",
          "max|r| =", float(np.max(np.abs(res_vec))),
          "mean|r| =", float(np.mean(np.abs(res_vec))))

    # --------------------------------------------------------
    # C) moment 계산 (현재 C++에 moment_batch가 없으면 루프로 계산)
    #    - 이 부분이 느리면, C++에 moment_batch 추가하는 게 맞음.
    # --------------------------------------------------------
    start = timer()
    m_ca = np.empty_like(K, dtype=np.float64)
    for i in range(K.size):
        m_ca[i] = cal.moment(float(eps_ca_opt[i]), float(K[i]))
    end = timer()
    print(f"moment loop finished in {end-start:.6f} s")

    # --------------------------------------------------------
    # Plotting
    # --------------------------------------------------------
    mask = np.isfinite(m_ca) & np.isfinite(Mtar) & (np.abs(Mtar) > 1e-12)

    plt.figure()
    plt.plot(K[mask], m_ca[mask], label="C++ objective + SciPy newton -> m_ca")
    plt.plot(K, Mtar, "--", label="Target M")
    plt.xlabel("kappa")
    plt.ylabel("Moment")
    plt.legend()
    plt.grid(True)
    plt.show()

    plt.figure()
    plt.plot(K[mask], m_ca[mask] / Mtar[mask])
    plt.xlabel("kappa")
    plt.ylabel("m_ca / Mtar")
    plt.grid(True)
    plt.show()
