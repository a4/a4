#ifndef _ALORENTZVECTOR_H_
#define _ALORENTZVECTOR_H_

#include <cmath>

#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.141592653589793238462643383279502884197

struct ALorentzVector
{
    double px, py, pz, E;

    ALorentzVector(): px(0), py(0), pz(0), E(0) {};
    ALorentzVector(const double &p1, const double &p2, const double &p3, const double &E): px(p1), py(p2), pz(p3), E(E) {};
    ALorentzVector(const ALorentzVector& p): px(p.px), py(p.py), pz(p.pz), E(p.E) {};

    template<typename V>
    static ALorentzVector from(const V&v) {
       return ALorentzVector(v.px(), v.py(), v.pz(), v.e());
    }

    template<typename V>
    static ALorentzVector from_met(const V&v) {
       ALorentzVector lv(v.x(), v.y(), 0.0, 0.0);
       lv.E = lv.pt();
       return lv;
    }

    static ALorentzVector from_met(const float& x, const float& y) {
       ALorentzVector lv(x, y, 0.0, 0.0);
       lv.E = lv.pt();
       return lv;
    }
    
    template<typename V>
    static ALorentzVector from_ptetaphie(const double& pt, 
      const double& eta, const double& phi, const double& e) {
        return std::move(ALorentzVector(
            pt*cos(phi), pt*sin(phi), pt*sinh(eta), e));
    }
    
    template<typename V>
    static ALorentzVector from_ptetaphie(const V& v) {
        return std::move(ALorentzVector::from_ptetaphie(
            v.pt(), v.eta(), v.phi(), v.e()));
    }

    double p()   const {return sqrt(px*px + py*py + pz*pz);}
    double p2()  const {return px*px + py*py + pz*pz;}
    double pt()  const {return hypot(px, py);}
    double pt2()  const {return px*px + py*py;}
    double m2()  const {return (E - p()) * (E + p());}
    double m()   const {double msq = m2(); return msq < 0 ? -sqrt(-msq) : sqrt(msq);}///sqrt(fabs(msq));}
    double mt()  const {return sqrt((E - pz)  * (E + pz));}
    double y()   const {return 0.5*log((E + pz)/(E - pz));}
    inline double et() const {return E/p()*pt();}
    inline double e()  const {return E;}
    double eta() const {return 0.5*log((p() + pz)/(p() - pz));}
    double phi() const {return atan2(py, px); };

    double delta_phi(const double & p_phi) const
    {
        double d_phi = phi() - p_phi;
        while (d_phi >= M_PI) d_phi -= 2*M_PI;
        while (d_phi < -M_PI) d_phi += 2*M_PI;
        return d_phi;
    }
    double delta_phi(const ALorentzVector &p) const { return delta_phi(p.phi()); };

    double delta_r(const double & p_eta, const double & p_phi) const
    {
        double d_eta = eta() - p_eta;
        double d_phi = delta_phi(p_phi);
        return hypot(d_eta, d_phi);
    }
    double delta_r(const ALorentzVector &p) const
    {
        double d_eta = eta() - p.eta();
        double d_phi = delta_phi(p);
        return hypot(d_eta, d_phi);
    }

    bool operator==(const ALorentzVector & rhs) const {
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
