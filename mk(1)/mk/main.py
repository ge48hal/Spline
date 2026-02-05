import numpy as np
from shapely.geometry import Polygon

from concreteproperties.stress_strain_profile import StressStrainProfile
from concreteproperties.material import Material
from concreteproperties.pre import CPGeom

from sectionproperties.pre.geometry import CompoundGeometry

from concreteproperties import ConcreteSection

from concreteproperties.post import si_n_mm, si_kn_m
from concreteproperties.results import MomentCurvatureResults


n = 15
s = 100

# ---- trapezoid geometry (mm) ----
b_Lo = 400.0  # bottom width
b_Hi = 200.0  # top width
h = 900.0  # height

poly = Polygon([(-b_Lo / 2, 0.0),(+b_Lo / 2, 0.0),(+b_Hi / 2, h),(-b_Hi / 2, h)])

eps_cc = np.linspace(-0.01, 0, n)
eps_ft = np.linspace(0, 0.002, n)

sig_cc = np.linspace(-40, 0, n)
sig_ft  = np.linspace(0, 3, n)



strains = np.hstack((eps_cc[:-2], eps_ft[1:]))
stresses = np.hstack((sig_cc[:-2], sig_ft[1:]))


#--------------------------------------------------
material = StressStrainProfile(strains=strains,stresses=stresses)

c = Material(
    name="n25p3",
    density=0,
    stress_strain_profile=material,
    colour="blue",
    meshed=True,
)

cs = ConcreteSection(CompoundGeometry([CPGeom(poly, material=c).to_sp_geom()]))
results = cs.moment_curvature_analysis(progress_bar=True)



MomentCurvatureResults.plot_multiple_results(
    moment_curvature_results=[results],
    labels=["cs.name"],
    fmt="-",
    eng=True,
    units=si_kn_m,
)

with open("moment_curvature_results.csv", "w") as f:
    f.write('kappa, mx, n\n')
    for k,m,n in zip(results.kappa, results.m_x, results.n):
        f.write(f'{k:.15f},{m:.3f},{n:.3f}\n')