
from math import log10, floor

class UncertainNumber:
    def __init__(self, n, e):
        self.n = n
        self.esq = e*e

    def __add__(self, ucn):
        self.n += ucn.n
        self.esq += ucn.esq
        return self
    
    @property
    def e(self):
        if self.esq == 0:
            return 0
        return self.esq**0.5

    def get_physics_numbers(self, e_signif_digits=2, max_precision=-2, latex=False):
        n, e = self.n, self.e
        if e!=e or n!=n:
            return str(n), str(e)
        decade_n = int(floor(log10(abs(n)))) if n != 0 else 0
        decade_e = int(floor(log10(e))) if e != 0 else 0
        prec = min(-1-decade_e+e_signif_digits, -max_precision)
        e = round(e, prec)
        n = round(n, max(prec, -1-decade_n+1)) # do not round n to 0

        if n > 1e6:
            exponent = 6+((log10(n)-6)//3)*3
            print prec, prec+exponent
            prec += exponent
            n /= 10**exponent
            e /= 10**exponent
            if latex:
                s = "%%.%if " % max(prec,0)
                sn, se = s%n, (s%e) + (" \cdot 10^{%i}"%exponent)
            else:
                s = "%%.%ife%i" % (max(prec,0), exponent)
                sn, se = s%n, s%e
        else:
            s = "%%.%if"%max(prec,0)
            sn, se = s%n, s%e
        return sn, se

    def latex(self):
        n, e = self.get_physics_numbers(latex=True)
        return "$%s \pm %s$" % self.get_physics_numbers(latex=True)

    def __str__(self):
        return "%s +- %s" % self.get_physics_numbers()


class CertainNumber:
    def __init__(self, n):
        self.n = n

    def __add__(self, cn):
        self.n += ucn.n
    
    def get_physics_number(self, latex=False):
        n = self.n
        if n < 1e6:
            return "%i" % n

        decade_n = int(floor(log10(abs(n))))
        exponent = 6+((log10(n)-6)//3)*3
        prec = 1 - decade_n//2 + exponent
        print exponent, decade_n, prec
        n /= 10**exponent
        if latex:
            s = "%%.%if " % max(prec,0)
            return (s%n) + (" \cdot 10^{%i}"%exponent)
        else:
            s = "%%.%ife%i" % (max(prec,0), exponent)
            return s%n
        return "%i" % n

    def latex(self):
        n = self.get_physics_number(latex=True)
        return "$%s$" % n

    def __str__(self):
        return "%s" % self.get_physics_number()


