import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from timeit import default_timer as timer

from scipy import interpolate
import plotgraph
import spline_ref

import splinepy


if __name__ == "__main__":
    
    df = pd.read_csv("random_points.csv")
    
    eps = df["eps"].astype(np.float64).to_list()
    sig = df["sig"].astype(np.float64).to_list()
    
    
    points = splinepy.Points(eps, sig)
    
    
    #res = splinepy.cal_momentum(points)
    start = timer()
    pts = splinepy.preprocess(eps[-1], points)
    a, b = splinepy.cal_area_momentum_simd(pts)
    end = timer()
    print(f"SIMD Area-Momentum computation time: {end - start:.6f} seconds")
    
    start = timer()
    pts = splinepy.preprocess(eps[-1], points)
    c,d = splinepy.cal_area_momentum(pts)
    end = timer()
    print(f"Regular Area-Momentum computation time: {end - start:.6f}seconds")
    
    ilm = interpolate.interp1d(
    eps, sig,
    kind="linear",
    assume_sorted=True,
    bounds_error=True,
    )
    
    arr = np.column_stack((eps, sig))
    start = timer()
    lm = spline_ref.prep(eps[-1], arr, ilm)
    e = spline_ref.m0_reduced(lm)
    f = spline_ref.m1_reduced(lm)
    end = timer()
    print(f"Reference Area-Momentum computation time: {end - start:.6f} seconds")
    
    #print("Momentum result:", res)
    print("Shoelace area (a):", a , c , e)
    print("Shoelace momentum (b):", b, d, f)
    print("Centroid eps:", b / a)
    print("Total points loaded from CSV:", points.size())
    
    ok_area = np.allclose([a, c, e], [c, e, a])
    ok_mom  = np.allclose([b, d, f], [d, f, b])
    print("Area close:", ok_area)
    print("Momentum close:", ok_mom)
    
    Cx = b / a
    
    plotgraph.plot_eps_sig_with_centroid(eps, sig, Cx, title="EPS-SIG Curve with Centroid")
