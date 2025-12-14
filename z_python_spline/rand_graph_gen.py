import csv
import random
import numpy as np
from numpy import float64
import plotgraph
from scipy import interpolate


def generate_random_points(
    n_points: int,
    n_keypoints: int,
    eps_min: float64,
    eps_max: float64,
    sig_min: float64,
    sig_max: float64,
):
    
    if eps_min <= 0 or sig_min <= 0:
        raise ValueError("eps_min, sig_min must be > 0")

    keypoints =[]
    keypoints.append((0,0))
    
    for _ in range(n_keypoints-1):
        eps = random.uniform(eps_min, eps_max)
        sig = random.uniform(sig_min, sig_max)
        keypoints.append((eps, sig))
        
    keypoints.sort(key=lambda x: x[0])
    
    
    kp = np.array(keypoints, dtype=float64)
    eps_arr = kp[:, 0]
    sig_arr = kp[:, 1]
    
    itp = interpolate.interp1d(eps_arr, sig_arr, kind= 'linear'
                                        ,bounds_error=True, assume_sorted=True)
    
    
    eps_min_val = eps_arr[0]
    eps_max_val = eps_arr[-1]
    
    eps_samples = np.linspace(eps_min_val, eps_max_val, n_points, dtype=float64)
    
    sig_samples = itp(eps_samples)
    
    
    return list(zip(eps_samples, sig_samples))

def points_to_csv(pairs, filename: str):
    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["eps", "sig"])
        for eps, sig in pairs:
            writer.writerow([eps, sig])
    
    print("CSV file generated:", filename)

if __name__ == "__main__":
    name = "random_graph.csv"
    n = 20000
    n_keypoints = random.randint(3, 10)
    e_min = 0.1
    e_max = 1000.0
    s_min = 0.1
    s_max = 100000.0
    pairs = generate_random_points(n, n_keypoints, e_min, e_max, s_min, s_max)
    points_to_csv(pairs, name)
    
    plotgraph.basicgraph(pairs, title=f"Random eps-sig curve with {n_keypoints} keypoints")
    