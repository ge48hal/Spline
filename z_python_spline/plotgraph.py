import matplotlib.pyplot as plt
import numpy as np

def basicgraph(pairs, title: str = "eps-sig curve"):
    eps_values = [e for e, _ in pairs]
    sig_values = [s for _, s in pairs]

    plt.figure()
    plt.plot(eps_values, sig_values, marker=".", linewidth=1)
    plt.xlabel("epsilon")
    plt.ylabel("sigma")
    plt.title(title)
    plt.grid(True)
    plt.tight_layout()
    plt.show()


def plot_eps_sig_with_centroid(eps, sig, Cx, title="EPS-SIG Curve with Centroid"):
    eps = np.asarray(eps, dtype=float)
    sig = np.asarray(sig, dtype=float)

    # -----------------------------
    # 1) calculate Cy
    # -----------------------------
    # Find the index in the eps array where Cx should be inserted
    idx = np.searchsorted(eps, Cx)

    # If Cx is outside the data range
    if idx == 0 or idx == len(eps):
        raise ValueError("Cx is out of the eps range")

    # Points on both sides
    x0, x1 = eps[idx - 1], eps[idx]
    y0, y1 = sig[idx - 1], sig[idx]

    # Linear interpolation
    t = (Cx - x0) / (x1 - x0)
    Cy = y0 + t * (y1 - y0)

    # -----------------------------
    # 2) Plotting the graph
    # -----------------------------
    fig, ax = plt.subplots(figsize=(8, 5))

    ax.plot(eps, sig, label="eps-sig curve", color="blue")
    ax.scatter([Cx], [Cy], color="red", s=80, zorder=5)
    ax.plot([Cx, Cx], [Cy, 0], color="red", linestyle="--", alpha=0.7)

    ax.text(
        Cx,
        0,
        f"{Cx:.2f}",
        ha="center",
        va="top",
        color="red",
        fontsize=10,
    )

    ax.set_xlabel("epsilon (eps)")
    ax.set_ylabel("sigma (sig)")
    ax.set_title(title)
    ax.grid(True)
    ax.legend()

    ymin, ymax = ax.get_ylim()
    ax.set_ylim(0, ymax)

    plt.tight_layout()
    plt.show()
