import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import interp1d
from scipy.optimize import newton, minimize, brenth

from resources import K, EPS_0, eps_ca_max, bounds, Mtar
from timeit import default_timer as timer


class CrossSection:
    def __init__(self, h, num_timesteps, num_spacesteps=100):

        self._cc = None
        self._cc_pad = None
        self._cc_gf = None
        self._cc_i = None

        self._ft = None
        self._ft_pad = None
        self._ft_gf = None
        self._ft_i = None

        self._eps_ca_max = None

        self.l = 160  # [mm]
        self.h = h  # [mm]
        self.h_u = 0.5 * h  # [mm] upper half
        self.h_d = 0.5 * h  # [mm] lower half

        self.DIM = 2
        self.EPS = 0
        self.SIG = 1

        self.b = 1  # [mm]
        self.E = 60_000  # [MPa]
        self.I = (self.b * self.h**3) / 12  # [mm^4]
        self.W = (self.b * self.h**2) / 6  # [mm^3]

        self.num_timesteps = num_timesteps
        self.all_timesteps = np.arange(self.num_timesteps)

        self.f_cc = np.zeros(self.num_timesteps)
        self.f_ft = np.zeros(self.num_timesteps)
        self.df = np.zeros(self.num_timesteps)

        self.m0_cc = np.zeros(self.num_timesteps)
        self.m0_ft = np.zeros(self.num_timesteps)

        self.m1_cc = np.zeros(self.num_timesteps)
        self.m1_ft = np.zeros(self.num_timesteps)

        self.z_cc_na = np.zeros(self.num_timesteps)
        self.z_ft_na = np.zeros(self.num_timesteps)

        self.z_cc_ca = np.zeros(self.num_timesteps)
        self.z_ft_ca = np.zeros(self.num_timesteps)

        self.h_cc = np.zeros(self.num_timesteps)
        self.h_ft = np.zeros(self.num_timesteps)
        self.z_na_ca = np.zeros(self.num_timesteps)

        self.m_ca = np.zeros(self.num_timesteps)

        self.jac_cc = np.zeros(self.num_timesteps)
        self.jac_ft = np.zeros(self.num_timesteps)

        self.eps_cc = np.zeros(self.num_timesteps)
        self.eps_ft = np.zeros(self.num_timesteps)

        self.eps_ca = np.zeros(self.num_timesteps)
        self.kappa = np.zeros(self.num_timesteps)

    def mp(self, eps, _lm, ilm, _lm_pad):

        sig = ilm(eps)

        i = np.searchsorted(_lm[:, self.EPS], eps, side="left")  #

        _lm_pad[self.all_timesteps, i, self.EPS] = eps
        _lm_pad[self.all_timesteps, i, self.SIG] = sig

        _lm_pad[self.all_timesteps, i + 1, self.EPS] = eps
        _lm_pad[self.all_timesteps, i + 1, self.SIG] = 0

        mask = np.arange(_lm.shape[0] + 2) > (i[:, None] + 1)
        _lm_pad[mask] = 0

        return _lm_pad

    def m0(self, _lm_pad):
        x = _lm_pad[:, :, self.EPS]
        z = _lm_pad[:, :, self.SIG]
        return 0.5 * np.abs(np.sum(x[:, :-1] * z[:, 1:] - z[:, :-1] * x[:, 1:], axis=1))

    def m1(self, _lm_pad):
        x = _lm_pad[:, :, self.EPS]
        z = _lm_pad[:, :, self.SIG]
        return (1 / 6) * np.abs(
            np.sum(
                (x[:, :-1] + x[:, 1:]) * (x[:, :-1] * z[:, 1:] - x[:, 1:] * z[:, :-1]),
                axis=1,
            )
        )

    @property
    def cc(self):
        return self._cc

    @cc.setter
    def cc(self, _cc):

        self._cc = _cc
        # setup interpolator

        self._cc_i = interp1d(
            x=_cc[:, self.EPS],
            y=_cc[:, self.SIG],
            kind="linear",
            bounds_error=True,
            assume_sorted=True,
        )

        # setup padding
        self._cc_pad = np.zeros((self.num_timesteps, _cc.shape[0] + 2, self.DIM))
        self._cc_pad[:, :-2, :] = _cc

        # setup gf
        self._cc_gf = self.m0(_cc[None, :, :])

    @property
    def ft(self):
        return self._ft

    @ft.setter
    def ft(self, _ft):

        self._ft = _ft
        # setup interpolator

        self._ft_i = interp1d(
            x=_ft[:, self.EPS],
            y=_ft[:, self.SIG],
            kind="linear",
            bounds_error=True,
            assume_sorted=True,
        )

        # setup padding
        self._ft_pad = np.zeros((self.num_timesteps, _ft.shape[0] + 2, self.DIM))
        self._ft_pad[:, :-2, :] = _ft

        # setup gf
        self._ft_gf = self.m0(_ft[None, :, :])

    @property
    def eps_ca_max(self):
        return self._eps_ca_max

    @eps_ca_max.setter
    def eps_ca_max(self, _eps_ca_max):
        self._eps_ca_max = _eps_ca_max

    def objective(self, eps_ca, kappa, opt=True, write=False):

        eps_ca = np.clip(eps_ca, 0, self._eps_ca_max)

        eps_cc = abs((eps_ca - kappa * self.h_u))
        eps_ft = abs((eps_ca + kappa * self.h_d))

        eps_dt = eps_ft + eps_cc

        h_cc = abs((eps_cc / eps_dt) * self.h)
        h_ft = abs((eps_ft / eps_dt) * self.h)

        jac_cc = h_cc / eps_cc
        jac_ft = h_ft / eps_ft

        # TODO: implement eps as array

        _cc_pad = self.mp(eps_cc, self._cc, self._cc_i, self._cc_pad)
        _ft_pad = self.mp(eps_ft, self._ft, self._ft_i, self._ft_pad)

        if opt:

            m0_cc = self.m0(_cc_pad)
            m0_ft = self.m0(_ft_pad)

            f_cc = m0_cc * jac_cc
            f_ft = m0_ft * jac_ft

            return f_cc - f_ft

        else:
            m1_cc = self.m1(_cc_pad)
            m1_ft = self.m1(_ft_pad)

            # two ways to compute the moment
            # m_ca = f_cc * z_cc_ca + f_ft * z_ft_ca

            m_ca = m1_cc * jac_cc**2 + m1_ft * jac_ft**2

            if write:

                self.eps_cc = eps_cc
                self.eps_ft = eps_ft
                self.eps_ca = eps_ca
                self.kappa = kappa

                self.h_cc = h_cc
                self.h_ft = h_ft

                self.jac_cc = jac_cc
                self.jac_ft = jac_ft

                self.m0_cc = self.m0(_cc_pad)
                self.m0_ft = self.m0(_ft_pad)

                self.m1_cc = m1_cc
                self.m1_ft = m1_ft

                self.f_cc = self.m0_cc * self.jac_cc
                self.f_ft = self.m0_ft * self.jac_ft
                self.df = np.abs(self.f_ft - self.f_cc)

                # relative to the neutral axis
                self.z_cc_na = (self.m1_cc / self.m0_cc) * self.jac_cc
                self.z_ft_na = (self.m1_ft / self.m0_ft) * self.jac_ft

                # relative to the centroid axis
                self.z_cc_ca = self.h_u - (self.h_cc - self.z_cc_na)
                self.z_ft_ca = self.h_d - (self.h_ft - self.z_ft_na)

                # neutral axis relative to the centroid axis
                self.z_na_ca = self.h_d - self.h_ft
                self.m_ca = m_ca

            return m_ca


if __name__ == "__main__":
    cs = CrossSection(300, 200)

    cs.eps_ca_max = eps_ca_max * 0.9999
    cs.cc = np.array([[0 / 1000, 0], [3 / 1000, 180], [10 / 1000, 180]])
    cs.ft = np.array([[0 / 1000, 0], [2 / 1000, 50], [4 / 1000, 50], [8 / 1000, 75]])

    eps_ca_ini = np.linspace(0, cs.eps_ca_max, 200)

    start = timer()
    eps_ca_opt = newton(cs.objective, eps_ca_ini, args=(K,), tol=1e-10)
    end = timer()

    print(f"run of newt took {end - start} seconds")

    # start = timer()
    # res = minimize(
    #     cs.objective,
    #     eps_ca_ini,
    #     args=(K,),
    #     method="Nelder-Mead",
    #     bounds=bounds,
    #     tol=1e-3,
    # )
    # end = timer()
    # print(f"run of nelder-mead took {end - start} seconds")

    cs.objective(eps_ca_opt, K, opt=False, write=True)

    plt.plot(cs.kappa, cs.m_ca)
    plt.plot(K, Mtar)
    plt.show()

    plt.plot(K, cs.m_ca / Mtar)
    plt.show()
