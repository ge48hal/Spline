import numpy as np
from scipy.optimize import minimize


def avg(arr):
    return 0.5 * (arr[:-1] + arr[1:])

n = 30
s = n

# ---- trapezoid geometry (mm) ----
b_Lo = 400.0  # bottom width
b_Hi = 200.0  # top width
h = 600.0  # height


# z centroid of trapezoid
z_ca =  h / 3 * (2 * b_Lo + b_Hi) / (b_Lo + b_Hi)

dz = h / s

eps_cc = np.linspace(-0.01, 0, n)
eps_ft = np.linspace(0, 0.002, n)

sig_cc = np.linspace(-40, 0, n)
sig_ft  = np.linspace(0, 3, n)

# combine compression and tension branches

strains = np.hstack((eps_cc[:-2], eps_ft[1:]))
stresses = np.hstack((sig_cc[:-2], sig_ft[1:])) # OK 

def get_strains(eps_hi, eps_lo):
    eps_z = np.linspace(eps_hi, eps_lo, s+1)

    # get neutral axis
    z_na = (eps_lo/(eps_lo-eps_hi)) * h
    Z_na = np.linspace(-z_na, h - z_na, s+1)

    dy = (b_Hi + ((b_Lo - b_Hi) / h) * z_na) + ((b_Lo - b_Hi) / h) * Z_na

    return avg(eps_z), avg(dy)*dz, avg( Z_na)

def get_stresses(eps_z):
    return np.interp(eps_z, strains, stresses) # prep 

def obj_kappa1(eps, eps_lo):

    eps_zna, dA_na, dz_na = get_strains(eps, eps_lo)
    sig_zna = get_stresses(eps_zna)
    dN = sig_zna * dA_na
    return abs(np.sum(dN))

def obj_kappa2(eps, eps_hi):

    eps_zna, dA_na, dz_na = get_strains(eps_hi, eps)
    sig_zna = get_stresses(eps_zna)
    dN = sig_zna * dA_na
    return abs(np.sum(dN))

def get_kappa_max(**kwargs):
    # ultimate strain at top fiber, ... assuming linear strain distribution

    # ultimate strain at bottom fiber, ... assuming linear strain distribution
    res_1 = minimize(obj_kappa1, 0, method = 'Nelder-Mead', args = (eps_ft[-1]), options = {'xatol': 1e-10})
    res_2 = minimize(obj_kappa2, 0, method = 'Nelder-Mead', args = (eps_cc[0]), options = {'xatol': 1e-10})

    # kappa_max is given by the case, when either top fiber or bottom fiber reaches its ultimate strain and the other fiber is within its limits

    eps_cc_hi = res_1.x[0]
    eps_ft_lo = res_2.x[0]

    #check limits
    check = (eps_cc[-1] > eps_cc_hi) & (eps_cc_hi > eps_cc[0])
    if check:
        kappa_max = (eps_ft[-1]-eps_cc_hi)/h
        return kappa_max

    check = (eps_ft[0] < eps_ft_lo) & (eps_ft_lo < eps_ft[-1])
    if check:
        kappa_max = (eps_ft_lo - eps_cc[0])/h
        return kappa_max

    raise ValueError('something went wrong...')


if __name__ == '__main__':
    print(get_kappa_max())