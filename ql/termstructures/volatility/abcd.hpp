/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_abcd_hpp
#define quantlib_abcd_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/math/abcdmathfunction.hpp>

namespace QuantLib {
    
    //! %Abcd functional form for instantaneous volatility
    /*! \f[ f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d \f]
        following Rebonato's notation. */
    class AbcdFunction : public AbcdMathFunction {

      public:
        AbcdFunction(Real a = -0.06,
                     Real b =  0.17,
                     Real c =  0.54,
                     Real d =  0.17);

        //! maximum value of the volatility function
        Real maximumVolatility() const { return maximumValue(); }

        //! volatility function value at time 0: \f[ f(0) \f]
        Real shortTermVolatility() const { return (*this)(0.0); }

        //! volatility function value at time +inf: \f[ f(\inf) \f]
        Real longTermVolatility() const { return longTermValue(); }

        /*! instantaneous covariance function at time t between T-fixing and
            S-fixing rates \f[ f(T-t)f(S-t) \f] */
        Real covariance(Time t, Time T, Time S) const;

        /*! integral of the instantaneous covariance function between
            time t1 and t2 for T-fixing and S-fixing rates
            \f[ \int_{t1}^{t2} f(T-t)f(S-t)dt \f] */
        Real covariance(Time t1, Time t2, Time T, Time S) const;

         /*! average volatility in [tMin,tMax] of T-fixing rate:
            \f[ \sqrt{ \frac{\int_{tMin}^{tMax} f^2(T-u)du}{tMax-tMin} } \f] */
        Real volatility(Time tMin, Time tMax, Time T) const;

        /*! variance between tMin and tMax of T-fixing rate:
            \f[ \frac{\int_{tMin}^{tMax} f^2(T-u)du}{tMax-tMin} \f] */
        Real variance(Time tMin, Time tMax, Time T) const;
        

        
        // INSTANTANEOUS
        /*! instantaneous volatility at time t of the T-fixing rate:
            \f[ f(T-t) \f] */
        Real instantaneousVolatility(Time t, Time T) const;

        /*! instantaneous variance at time t of T-fixing rate:
            \f[ f(T-t)f(T-t) \f] */
        Real instantaneousVariance(Time t, Time T) const;

        /*! instantaneous covariance at time t between T and S fixing rates:
            \f[ f(T-u)f(S-u) \f] */
        Real instantaneousCovariance(Time u, Time T, Time S) const;

        // PRIMITIVE
        /*! indefinite integral of the instantaneous covariance function at
            time t between T-fixing and S-fixing rates
            \f[ \int f(T-t)f(S-t)dt \f] */
        Real primitive(Time t, Time T, Time S) const;
        
    };

    
    // Helper class used by unit tests
    class AbcdSquared : public std::unary_function<Real,Real> {
      
      public:
        AbcdSquared(Real a, Real b, Real c, Real d, Time T, Time S);
        Real operator()(Time t) const;
      
      private:
        boost::shared_ptr<AbcdFunction> abcd_;
        Time T_, S_;
    };

    inline Real abcdBlackVolatility(Time u, Real a, Real b, Real c, Real d) {
        AbcdFunction model(a,b,c,d);
        return model.volatility(0.,u,u);
    }
}

#endif


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/comparison.hpp>

namespace QuantLib {

    AbcdFunction::AbcdFunction(Real a, Real b, Real c, Real d)
    : AbcdMathFunction(a, b, c, d) {}

    Real AbcdFunction::volatility(Time tMin, Time tMax, Time T) const {
        if (tMax==tMin)
            return instantaneousVolatility(tMax, T);
        QL_REQUIRE(tMax>tMin, "tMax must be > tMin");
        return std::sqrt(variance(tMin, tMax, T)/(tMax-tMin));
    }

    Real AbcdFunction::variance(Time tMin, Time tMax, Time T) const {
        return covariance(tMin, tMax, T, T);
    }

    Real AbcdFunction::covariance(Time t, Time T, Time S) const {
        return (*this)(T-t) * (*this)(S-t);
    }

    Real AbcdFunction::covariance(Time t1, Time t2, Time T, Time S) const {
        QL_REQUIRE(t1<=t2,
                   "integrations bounds (" << t1 <<
                   "," << t2 << ") are in reverse order");
        Time cutOff = std::min(S,T);
        if (t1>=cutOff) {
            return 0.0;
        } else {
            cutOff = std::min(t2, cutOff);
            return primitive(cutOff, T, S) - primitive(t1, T, S);
        }
    }

    // INSTANTANEOUS
    Real AbcdFunction::instantaneousVolatility(Time u, Time T) const {
        return std::sqrt(instantaneousVariance(u, T));
    }

    Real AbcdFunction::instantaneousVariance(Time u, Time T) const {
        return instantaneousCovariance(u, T, T);
    }
    Real AbcdFunction::instantaneousCovariance(Time u, Time T, Time S) const {
        return (*this)(T-u)*(*this)(S-u);
    }

    // PRIMITIVE
    Real AbcdFunction::primitive(Time t, Time T, Time S) const {
        if (T<t || S<t) return 0.0;

        if (close(c_,0.0)) {
            Real v = a_+d_;
            return t*(v*v+v*b_*S+v*b_*T-v*b_*t+b_*b_*S*T-0.5*b_*b_*t*(S+T)+b_*b_*t*t/3.0);
        }

        Real k1=std::exp(c_*t), k2=std::exp(c_*S), k3=std::exp(c_*T);

        return (b_*b_*(-1 - 2*c_*c_*S*T - c_*(S + T)
                     + k1*k1*(1 + c_*(S + T - 2*t) + 2*c_*c_*(S - t)*(T - t)))
                + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                         +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*t)
                + 2*b_*c_*(a_*(-1 - c_*(S + T) + k1*k1*(1 + c_*(S + T - 2*t)))
                         -2*d_*(k3*(1 + c_*S) + k2*(1 + c_*T)
                               - k1*k3*(1 + c_*(S - t))
                               - k1*k2*(1 + c_*(T - t)))
                         )
                ) / (4*c_*c_*c_*k2*k3);
    }

//===========================================================================//
//                               AbcdSquared                                //
//===========================================================================//

    AbcdSquared::AbcdSquared(Real a, Real b, Real c, Real d, Time T, Time S)
    : abcd_(new AbcdFunction(a,b,c,d)),
      T_(T), S_(S) {}

    Real AbcdSquared::operator()(Time t) const {
        return abcd_->covariance(t, T_, S_);
    }
}