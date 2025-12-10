import numpy as np
from timeit import default_timer as timer

EPS = 0
SIG = 1



def prep(eps, _lm, ilm):

    __lm = np.vstack(
        (
            _lm[_lm[:, 0] < eps],  # trim the polyline
            [eps, ilm(eps)],  # add the intersection point
            [eps, 0],  # project the intersection point to the x-axis
            [_lm[0, 0], _lm[0, 1]],  # add the origin, e.g. close the polyline
        )
    )
    return __lm


def m0_reduced(_lm):
    x = _lm[:,EPS]
    z = _lm[:,SIG]
    return 0.5 * np.abs(np.sum(x[:-1] * z[1:] - z[:-1] * x[1:]))


def m1_reduced(_lm):
    x = _lm[:,EPS]
    z = _lm[:,SIG]
    return (1 / 6) * np.abs(
        np.sum(
            (x[:-1] + x[1:]) * (x[:-1] * z[1:] - x[1:] * z[:-1]))
    )
