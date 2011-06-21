#ifndef _ALORENTZVECTOR_H_
#define _ALORENTZVECTOR_H_

#include <cmath>

#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.141592653589793238462643383279502884197

class ALorentzVector
{
  public:
    double px, py, pz, E;

    ALorentzVector(): px(0), py(0), pz(0), E(0) {};
    ALorentzVector(double p1, double p2, double p3, double E): px(p1), py(p2), pz(p3), E(E) {};
    ALorentzVector(const ALorentzVector& p): px(p.px), py(p.py), pz(p.pz), E(p.E) {};

    double p()   const {return sqrt(px*px + py*py + pz*pz);}
    double p2()  const {return px*px + py*py + pz*pz;}
    double pt()  const {return sqrt(px*px + py*py);}
    double m2()  const {return E*E - p2();}
    double m()   const {return sqrt(E*E - p2());}
    double mt()  const {return sqrt((E-pz)*(E+pz));}
    double y()   const {return 0.5*log((E + pz)/(E - pz));}
    // TODO: find out... inline double et()  const {return e()/cosh();}
    inline double e()  const {return E;}
    double eta() const {return 0.5*log((p() + pz)/(p() - pz));}
    double phi() const
    {
        double r = atan2(py,px);
        if(r < 0) r += 2*M_PI;
        return r;
    }

    bool operator== (const ALorentzVector & rhs) const {
        return px == rhs.px && py == rhs.py && pz == rhs.pz && E == rhs.E;
    };

    ALorentzVector & operator+=(const ALorentzVector &rhs) {
        px += rhs.px;
        py += rhs.py;
        pz += rhs.pz;
        E += rhs.E;
        return *this;
    }
    ALorentzVector & operator-=(const ALorentzVector &rhs) {
        px -= rhs.px;
        py -= rhs.py;
        pz -= rhs.pz;
        E -= rhs.E;
        return *this;
    }
    ALorentzVector & operator*=(const double &sf) {
        px *= sf;
        py *= sf;
        pz *= sf;
        E *= sf;
        return *this;
    }
    const ALorentzVector operator+(const ALorentzVector &other) const {
        ALorentzVector result = *this;
        result += other;
        return result;
    }

    const ALorentzVector operator-(const ALorentzVector &other) const {
        ALorentzVector result = *this;
        result -= other;
        return result;
    }
    const ALorentzVector operator*(const double &sf) const {
        ALorentzVector result = *this;
        result *= sf;
        return result;
    }
};

ALorentzVector operator*(const double &sf, const ALorentzVector &rhs) {
    ALorentzVector result = rhs;
    result *= sf;
    return result;
}


#endif
