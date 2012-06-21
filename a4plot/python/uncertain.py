
from math import log10, floor, sqrt

def add_tuples(t1, t2):
    if not t1 or not t2:
        return t1 if t1 else t2
    assert len(t1) == len(t2)
    return tuple(map(sum, zip(t1, t2)))

class UncertainNumber:
    def __init__(self, n=0.0, e=0.0, s={}):
        self.n = n
        self.stat_sq = e*e
        self.syst = dict(s)

    def __add__(self, ucn):
        self.n += ucn.n
        if ucn.stat_sq == ucn.stat_sq: # do not pick up NaNs
            self.stat_sq += ucn.stat_sq
        for sn in set(self.syst.keys()).union(set(ucn.syst.keys())):
            self.syst[sn] = add_tuples(self.syst.get(sn, ()), ucn.syst.get(sn, ()))
        return self
    
    def set_syst(self, syst_name, syst_value):
        assert isinstance(syst_name, tuple)
        assert isinstance(syst_value, tuple)
        self.syst[syst_name] = syst_value
    
    @property
    def e(self):
        if self.stat_sq <= 0 or self.stat_sq != self.stat_sq:
            return 0
        return sqrt(self.stat_sq)

    @property
    def s(self):
        if not self.syst:
            return 0
        return sqrt(sum(((sum(map(abs,sv))/len(sv))**2) for sn, sv in self.syst.iteritems() if len(sv) > 0))

    def get_physics_numbers(self, e_signif_digits=2, max_precision=-2, latex=False):
        n, e, s = self.n, self.e, self.s
        if e!=e or n!=n or s!=s:
            if self.syst: return str(n), str(e), str(s)
            else: return str(n), str(e)
        decade_n = int(floor(log10(abs(n)))) if n != 0 else 0
        decade_e = int(floor(log10(e))) if e != 0 else 0
        decade_s = int(floor(log10(s))) if s != 0 else 0
        prec = min( - 1 - decade_e + e_signif_digits, - max_precision)
        e = round(e, prec)
        s = round(s, prec)
        n = round(n, max(prec, -1-decade_n+1)) # do not round n to 0

        if False: #n > 1e6:
            exponent = 6+((log10(n)-6)//3)*3
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
            st = "%%.%if"%max(prec,0)
            sn, se, ss = st%n, st%e, st%s
        if self.syst: return sn, se, ss
        else: return sn, se

    def latex(self):
        if not self.syst:
            n, e = self.get_physics_numbers(latex=True)
            return "$%s \pm %s$" % (n, e)
        else:
            n, e, s = self.get_physics_numbers(latex=True)
            return "$%s \pm %s \pm %s$" % (n, e, s)

    def __str__(self):
        if not self.syst:
            n, e = self.get_physics_numbers(latex=True)
            return "%s +- %s" % (n, e)
        else:
            n, e, s = self.get_physics_numbers(latex=True)
            return "%s +- %s +- %s" % (n, e, s)

class CertainNumber:
    def __init__(self, n=0.0):
        self.n = n

    def __add__(self, cn):
        self.n += cn.n
        return self
    
    @property
    def e(self):
        return self.n**0.5

    def get_physics_number(self, latex=False):
        return "%i" % self.n
        """
        n = self.n
        if n < 1e6:
            return "%i" % n

        decade_n = int(floor(log10(abs(n))))
        exponent = 6+((log10(n)-6)//3)*3
        prec = 1 - decade_n//2 + exponent
        n /= 10**exponent
        if latex:
            s = "%%.%if " % max(prec,0)
            return (s%n) + (" \cdot 10^{%i}"%exponent)
        else:
            s = "%%.%ife%i" % (max(prec,0), exponent)
            return s%n
        return "%i" % n
        """
    def get_physics_numbers(self, latex=False):
        ### i know there is a difference between certain and unvcertain numbers but for easier use i named the functions the same. 
        return "%i" % self.n

    def latex(self):
        n = self.get_physics_number(latex=True)
        return "$%s$" % n

    def __str__(self):
        return "%s" % self.get_physics_number()


