/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Paolo Mazzocchi

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

#ifndef quantlib_abcd_math_function_hpp
#define quantlib_abcd_math_function_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    //! %Abcd functional form
    /*! \f[ f(t) = [ a + b*t ] e^{-c*t} + d \f]
        following Rebonato's notation. */
    class AbcdMathFunction : public std::unary_function<Time, Real> {

      public:
        AbcdMathFunction(Real a = 0.002,
                         Real b = 0.001, 
                         Real c = 0.16,
                         Real d = 0.0005);
        AbcdMathFunction(const std::vector<Real>& abcd);

        //! function value at time t: \f[ f(t) \f]
        Real operator()(Time t) const;

        //! time at which the function reaches maximum (if any)
        Time maximumLocation() const;

        //! maximum value of the function
        Real maximumValue() const;

        //! function value at time +inf: \f[ f(\inf) \f]
        Real longTermValue() const { return d_; }

        /*! first derivative of the function at time t
            \f[ f'(t) = [ (b-c*a) + (-c*b)*t) ] e^{-c*t} \f] */
        Real derivative(Time t) const;
        
        /*! indefinite integral of the function at time t
            \f[ \int f(t)dt = [ (-a/c-b/c^2) + (-b/c)*t ] e^{-c*t} + d*t \f] */
        Real primitive(Time t) const;
        
        /*! definite integral of the function between t1 and t2
            \f[ \int_{t1}^{t2} f(t)dt \f] */
        Real definiteIntegral(Time t1, Time t2) const;

        /*! Inspectors */
        Real a() const { return a_; }
        Real b() const { return b_; }
        Real c() const { return c_; }
        Real d() const { return d_; }
        const std::vector<Real>& coefficients() { return abcd_; }
        const std::vector<Real>& derivativeCoefficients() { return dabcd_; }
        // the primitive is not abcd

        /*! coefficients of a AbcdMathFunction defined as definite
            integral on a rolling window of length tau, with tau = t2-t */
        std::vector<Real> definiteIntegralCoefficients(Time t,
                                                       Time t2) const;

        /*! coefficients of a AbcdMathFunction defined as definite
            derivative on a rolling window of length tau, with tau = t2-t */
        std::vector<Real> definiteDerivativeCoefficients(Time t,
                                                         Time t2) const;

        static void validate(Real a,
                             Real b,
                             Real c,
                             Real d);
      protected:
        Real a_, b_, c_, d_;
      private:
        void initialize_();
        std::vector<Real> abcd_;
        std::vector<Real> dabcd_;
        Real da_, db_;
        Real pa_, pb_, K_;

        Real dibc_, diacplusbcc_;
    };

    // inline AbcdMathFunction
    inline Real AbcdMathFunction::operator()(Time t) const {
        //return (a_ + b_*t)*std::exp(-c_*t) + d_;
        return t<0 ? 0.0 : (a_ + b_*t)*std::exp(-c_*t) + d_;
    }

    inline Real AbcdMathFunction::derivative(Time t) const {
        //return (da_ + db_*t)*std::exp(-c_*t);
        return t<0 ? 0.0 : (da_ + db_*t)*std::exp(-c_*t);
    }

    inline Real AbcdMathFunction::primitive(Time t) const {
        //return (pa_ + pb_*t)*std::exp(-c_*t) + d_*t + K_;
        return t<0 ? 0.0 : (pa_ + pb_*t)*std::exp(-c_*t) + d_*t + K_;
    }

    inline Real AbcdMathFunction::maximumValue() const {
        if (b_==0.0 || a_<=0.0)
            return d_;
        return this->operator()(maximumLocation());
    }

}

#endif


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Paolo Mazzocchi

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


namespace QuantLib {

    void AbcdMathFunction::validate(Real a,
                                    Real b,
                                    Real c,
                                    Real d) {
        QL_REQUIRE(c>0, "c (" << c << ") must be positive");
        QL_REQUIRE(d>=0, "d (" << d << ") must be non negative");
        QL_REQUIRE(a+d>=0,
                   "a+d (" << a << "+" << d << ") must be non negative");

        if (b>=0.0)
            return;

        // the one and only stationary point...
        Time zeroFirstDerivative = 1.0/c-a/b;
        if (zeroFirstDerivative>=0.0) {
            // ... is a minimum
            // must be abcd(zeroFirstDerivative)>=0
            QL_REQUIRE(b>=-(d*c)/std::exp(c*a/b-1.0),
                       "b (" << b << ") less than " <<
                       -(d*c)/std::exp(c*a/b-1.0) << ": negative function"
                       " value at stationary point " << zeroFirstDerivative);
        }

    }

    void AbcdMathFunction::initialize_() {
        validate(a_, b_, c_, d_);
        da_ = b_ - c_*a_;
        db_ = -c_*b_;
        dabcd_[0]=da_;
        dabcd_[1]=db_;
        dabcd_[2]=c_;
        dabcd_[3]=0.0;

        pa_ = -(a_ + b_/c_)/c_;
        pb_ = -b_/c_;
        K_ = 0.0;

        dibc_ = b_/c_;
        diacplusbcc_ = a_/c_ + dibc_/c_;
    }

    AbcdMathFunction::AbcdMathFunction(Real aa, Real bb, Real cc, Real dd)
    : a_(aa), b_(bb), c_(cc), d_(dd), abcd_(4), dabcd_(4) {
        abcd_[0]=a_;
        abcd_[1]=b_;
        abcd_[2]=c_;
        abcd_[3]=d_;
        initialize_();
    }

    AbcdMathFunction::AbcdMathFunction(const std::vector<Real>& abcd)
    : abcd_(abcd), dabcd_(4) {
        a_=abcd_[0];
        b_=abcd_[1];
        c_=abcd_[2];
        d_=abcd_[3];
        initialize_();
    }

    Time AbcdMathFunction::maximumLocation() const {
        if (b_==0.0) {
            if (a_>=0.0)
                return 0.0;
            else
                return QL_MAX_REAL;
        }

        // stationary point
        // TODO check if minimum
        // TODO check if maximum at +inf
        Real zeroFirstDerivative = 1.0/c_-a_/b_;
        return (zeroFirstDerivative>0.0 ? zeroFirstDerivative : 0.0);
    }

    Real AbcdMathFunction::definiteIntegral(Time t1,
                                            Time t2) const {
        return primitive(t2)-primitive(t1);
    }

    std::vector<Real>
    AbcdMathFunction::definiteIntegralCoefficients(Time t,
                                                   Time t2) const {
        Time dt = t2 - t;
        Real expcdt = std::exp(-c_*dt);
        std::vector<Real> result(4);
        result[0] = diacplusbcc_ - (diacplusbcc_ + dibc_*dt)*expcdt;
        result[1] = dibc_ * (1.0 - expcdt);
        result[2] = c_;
        result[3] = d_*dt;
        return result;
    }

    std::vector<Real>
    AbcdMathFunction::definiteDerivativeCoefficients(Time t,
                                                     Time t2) const {
        Time dt = t2 - t;
        Real expcdt = std::exp(-c_*dt);
        std::vector<Real> result(4);
        result[1] = b_*c_/(1.0-expcdt);
        result[0] = a_*c_ - b_ + result[1]*dt*expcdt;
        result[0] /= 1.0-expcdt;
        result[2] = c_;
        result[3] = d_/dt;
        return result;
    }

}

