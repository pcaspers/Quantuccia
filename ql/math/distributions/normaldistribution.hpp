/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2010 Kakhkhor Abdijalilov

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

/*! \file normaldistribution.hpp
    \brief normal, cumulative and inverse cumulative distributions
*/

#ifndef quantlib_normal_distribution_hpp
#define quantlib_normal_distribution_hpp

#include <ql/math/errorfunction.hpp>
#include <ql/errors.hpp>
#include <ql/math/comparison.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/math/distributions/normal.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

namespace QuantLib {

    //! Normal distribution function
    /*! Given x, it returns its probability in a Gaussian normal distribution.
        It provides the first derivative too.

        \test the correctness of the returned value is tested by
              checking it against numerical calculations. Cross-checks
              are also performed against the
              CumulativeNormalDistribution and InverseCumulativeNormal
              classes.
    */
    class NormalDistribution : public std::unary_function<Real,Real> {
      public:
        NormalDistribution(Real average = 0.0,
                           Real sigma = 1.0);
        // function
        Real operator()(Real x) const;
        Real derivative(Real x) const;
      private:
        Real average_, sigma_, normalizationFactor_, denominator_,
            derNormalizationFactor_;
    };

    typedef NormalDistribution GaussianDistribution;


    //! Cumulative normal distribution function
    /*! Given x it provides an approximation to the
        integral of the gaussian normal distribution:
        formula here ...

        For this implementation see M. Abramowitz and I. Stegun,
        Handbook of Mathematical Functions,
        Dover Publications, New York (1972)
    */
    class CumulativeNormalDistribution
    : public std::unary_function<Real,Real> {
      public:
        CumulativeNormalDistribution(Real average = 0.0,
                                     Real sigma   = 1.0);
        // function
        Real operator()(Real x) const;
        Real derivative(Real x) const;
      private:
        Real average_, sigma_;
        NormalDistribution gaussian_;
        ErrorFunction errorFunction_;
    };


    //! Inverse cumulative normal distribution function
    /*! Given x between zero and one as
      the integral value of a gaussian normal distribution
      this class provides the value y such that
      formula here ...

      It use Acklam's approximation:
      by Peter J. Acklam, University of Oslo, Statistics Division.
      URL: http://home.online.no/~pjacklam/notes/invnorm/index.html

      This class can also be used to generate a gaussian normal
      distribution from a uniform distribution.
      This is especially useful when a gaussian normal distribution
      is generated from a low discrepancy uniform distribution:
      in this case the traditional Box-Muller approach and its
      variants would not preserve the sequence's low-discrepancy.

    */
    class InverseCumulativeNormal
        : public std::unary_function<Real,Real> {
      public:
        InverseCumulativeNormal(Real average = 0.0,
                                Real sigma   = 1.0);
        // function
        Real operator()(Real x) const {
            return average_ + sigma_*standard_value(x);
        }
        // value for average=0, sigma=1
        /* Compared to operator(), this method avoids 2 floating point
           operations (we use average=0 and sigma=1 most of the
           time). The speed difference is noticeable.
        */
        static Real standard_value(Real x) {
            Real z;
            if (x < x_low_() || x_high_() < x) {
                z = tail_value(x);
            } else {
                z = x - 0.5;
                Real r = z*z;
                z = (((((a1_()*r+a2_())*r+a3_())*r+a4_())*r+a5_())*r+a6_())*z /
                    (((((b1_()*r+b2_())*r+b3_())*r+b4_())*r+b5_())*r+1.0);
            }

            // The relative error of the approximation has absolute value less
            // than 1.15e-9.  One iteration of Halley's rational method (third
            // order) gives full machine precision.
            // #define REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
            #ifdef REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
            // error (f_(z) - x) divided by the cumulative's derivative
            const Real r = (f_(z) - x) * M_SQRT2 * M_SQRTPI * exp(0.5 * z*z);
            //  Halley's method
            z -= r/(1+0.5*z*r);
            #endif

            return z;
        }
      private:
        /* Handling tails moved into a separate method, which should
           make the inlining of operator() and standard_value method
           easier. tail_value is called rarely and doesn't need to be
           inlined.
        */
        static Real tail_value(Real x);
        #if defined(QL_PATCH_SOLARIS)
        CumulativeNormalDistribution f_;
        #else
        static const CumulativeNormalDistribution f_;
        #endif
        Real average_, sigma_;
        // Coefficients for the rational approximation.
        static Real a1_() { return -3.969683028665376e+01; }
        static Real a2_() { return 2.209460984245205e+02; }
        static Real a3_() { return -2.759285104469687e+02; }
        static Real a4_() { return 1.383577518672690e+02; }
        static Real a5_() { return -3.066479806614716e+01; }
        static Real a6_() { return 2.506628277459239e+00; }

        static Real b1_() { return -5.447609879822406e+01; }
        static Real b2_() { return 1.615858368580409e+02; }
        static Real b3_() { return -1.556989798598866e+02; }
        static Real b4_() { return 6.680131188771972e+01; }
        static Real b5_() { return -1.328068155288572e+01; }

        static Real c1_() { return -7.784894002430293e-03; }
        static Real c2_() { return -3.223964580411365e-01; }
        static Real c3_() { return -2.400758277161838e+00; }
        static Real c4_() { return -2.549732539343734e+00; }
        static Real c5_() { return 4.374664141464968e+00; }
        static Real c6_() { return 2.938163982698783e+00; }

        static Real d1_() { return 7.784695709041462e-03; }
        static Real d2_() { return 3.224671290700398e-01; }
        static Real d3_() { return 2.445134137142996e+00; }
        static Real d4_() { return 3.754408661907416e+00; }

        // Limits of the approximation regions
        static Real x_low_() { return 0.02425; }
        static Real x_high_() { return 1.0 - x_low_(); }
    };

    // backward compatibility
    typedef InverseCumulativeNormal InvCumulativeNormalDistribution;

    //! Moro Inverse cumulative normal distribution class
    /*! Given x between zero and one as
        the integral value of a gaussian normal distribution
        this class provides the value y such that
        formula here ...

        It uses Beasly and Springer approximation, with an improved
        approximation for the tails. See Boris Moro,
        "The Full Monte", 1995, Risk Magazine.

        This class can also be used to generate a gaussian normal
        distribution from a uniform distribution.
        This is especially useful when a gaussian normal distribution
        is generated from a low discrepancy uniform distribution:
        in this case the traditional Box-Muller approach and its
        variants would not preserve the sequence's low-discrepancy.

        Peter J. Acklam's approximation is better and is available
        as QuantLib::InverseCumulativeNormal
    */
    class MoroInverseCumulativeNormal
    : public std::unary_function<Real,Real> {
      public:
        MoroInverseCumulativeNormal(Real average = 0.0,
                                    Real sigma   = 1.0);
        // function
        Real operator()(Real x) const;
      private:
        Real average_, sigma_;
        static Real a0_() { return 2.50662823884; }
        static Real a1_() { return -18.61500062529; }
        static Real a2_() { return 41.39119773534; }
        static Real a3_() { return -25.44106049637; }

        static Real b0_() { return -8.47351093090; }
        static Real b1_() { return 23.08336743743; }
        static Real b2_() { return -21.06224101826; }
        static Real b3_() { return 3.13082909833; }

        static Real c0_() { return 0.3374754822726147; }
        static Real c1_() { return 0.9761690190917186; }
        static Real c2_() { return 0.1607979714918209; }
        static Real c3_() { return 0.0276438810333863; }
        static Real c4_() { return 0.0038405729373609; }
        static Real c5_() { return 0.0003951896511919; }
        static Real c6_() { return 0.0000321767881768; }
        static Real c7_() { return 0.0000002888167364; }
        static Real c8_() { return 0.0000003960315187; }
    };

    //! Maddock's Inverse cumulative normal distribution class
    /*! Given x between zero and one as
        the integral value of a gaussian normal distribution
        this class provides the value y such that
        formula here ...

        From the boost documentation:
         These functions use a rational approximation devised by
         John Maddock to calculate an initial approximation to the
         result that is accurate to ~10^-19, then only if that has
         insufficient accuracy compared to the epsilon for type double,
         do we clean up the result using Halley iteration.
    */
    class MaddockInverseCumulativeNormal
    : public std::unary_function<Real,Real> {
      public:
        MaddockInverseCumulativeNormal(Real average = 0.0,
                                       Real sigma   = 1.0);
        Real operator()(Real x) const;

      private:
        const Real average_, sigma_;
    };

    //! Maddock's cumulative normal distribution class
    class MaddockCumulativeNormal : public std::unary_function<Real,Real> {
      public:
        MaddockCumulativeNormal(Real average = 0.0,
                                       Real sigma   = 1.0);
        Real operator()(Real x) const;

      private:
        const Real average_, sigma_;
    };


    // inline definitions

    inline NormalDistribution::NormalDistribution(Real average,
                                                  Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");

        normalizationFactor_ = M_SQRT_2*M_1_SQRTPI/sigma_;
        derNormalizationFactor_ = sigma_*sigma_;
        denominator_ = 2.0*derNormalizationFactor_;
    }

    inline Real NormalDistribution::operator()(Real x) const {
        Real deltax = x-average_;
        Real exponent = -(deltax*deltax)/denominator_;
        // debian alpha had some strange problem in the very-low range
        return exponent <= -690.0 ? 0.0 :  // exp(x) < 1.0e-300 anyway
            normalizationFactor_*std::exp(exponent);
    }

    inline Real NormalDistribution::derivative(Real x) const {
        return ((*this)(x) * (average_ - x)) / derNormalizationFactor_;
    }

    inline CumulativeNormalDistribution::CumulativeNormalDistribution(
                                                 Real average, Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");
    }

    inline Real CumulativeNormalDistribution::derivative(Real x) const {
        Real xn = (x - average_) / sigma_;
        return gaussian_(xn) / sigma_;
    }

    inline InverseCumulativeNormal::InverseCumulativeNormal(
                                                 Real average, Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");
    }

    inline MoroInverseCumulativeNormal::MoroInverseCumulativeNormal(
                                                 Real average, Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");
    }

    // implementation

    inline Real CumulativeNormalDistribution::operator()(Real z) const {
        //QL_REQUIRE(!(z >= average_ && 2.0*average_-z > average_),
        //           "not a real number. ");
        z = (z - average_) / sigma_;

        Real result = 0.5 * ( 1.0 + errorFunction_( z*M_SQRT_2 ) );
        if (result<=1e-8) { //todo: investigate the threshold level
            // Asymptotic expansion for very negative z following (26.2.12)
            // on page 408 in M. Abramowitz and A. Stegun,
            // Pocketbook of Mathematical Functions, ISBN 3-87144818-4.
            Real sum=1.0, zsqr=z*z, i=1.0, g=1.0, x, y,
                 a=QL_MAX_REAL, lasta;
            do {
                lasta=a;
                x = (4.0*i-3.0)/zsqr;
                y = x*((4.0*i-1)/zsqr);
                a = g*(x-y);
                sum -= a;
                g *= y;
                ++i;
                a = std::fabs(a);
            } while (lasta>a && a>=std::fabs(sum*QL_EPSILON));
            result = -gaussian_(z)/z*sum;
        }
        return result;
    }

    // #if !defined(QL_PATCH_SOLARIS)
    // const CumulativeNormalDistribution InverseCumulativeNormal::f_;
    // #endif

    inline Real InverseCumulativeNormal::tail_value(Real x) {
        if (x <= 0.0 || x >= 1.0) {
            // try to recover if due to numerical error
            if (close_enough(x, 1.0)) {
                return QL_MAX_REAL; // largest value available
            } else if (std::fabs(x) < QL_EPSILON) {
                return QL_MIN_REAL; // largest negative value available
            } else {
                QL_FAIL("InverseCumulativeNormal(" << x
                        << ") undefined: must be 0 < x < 1");
            }
        }

        Real z;
        if (x < x_low_()) {
            // Rational approximation for the lower region 0<x<u_low
            z = std::sqrt(-2.0*std::log(x));
            z = (((((c1_()*z+c2_())*z+c3_())*z+c4_())*z+c5_())*z+c6_()) /
                ((((d1_()*z+d2_())*z+d3_())*z+d4_())*z+1.0);
        } else {
            // Rational approximation for the upper region u_high<x<1
            z = std::sqrt(-2.0*std::log(1.0-x));
            z = -(((((c1_()*z+c2_())*z+c3_())*z+c4_())*z+c5_())*z+c6_()) /
                ((((d1_()*z+d2_())*z+d3_())*z+d4_())*z+1.0);
        }

        return z;
    }

    inline Real MoroInverseCumulativeNormal::operator()(Real x) const {
        QL_REQUIRE(x > 0.0 && x < 1.0,
                   "MoroInverseCumulativeNormal(" << x
                   << ") undefined: must be 0<x<1");

        Real result;
        Real temp=x-0.5;

        if (std::fabs(temp) < 0.42) {
            // Beasley and Springer, 1977
            result=temp*temp;
            result=temp*
                (((a3_()*result+a2_())*result+a1_())*result+a0_()) /
                ((((b3_()*result+b2_())*result+b1_())*result+b0_())*result+1.0);
        } else {
            // improved approximation for the tail (Moro 1995)
            if (x<0.5)
                result = x;
            else
                result=1.0-x;
            result = std::log(-std::log(result));
            result = c0_()+result*(c1_()+result*(c2_()+result*(c3_()+result*
                                   (c4_()+result*(c5_()+result*(c6_()+result*
                                                       (c7_()+result*c8_())))))));
            if (x<0.5)
                result=-result;
        }

        return average_ + result*sigma_;
    }

    inline MaddockInverseCumulativeNormal::MaddockInverseCumulativeNormal(
        Real average, Real sigma)
    : average_(average), sigma_(sigma) {}

    inline Real MaddockInverseCumulativeNormal::operator()(Real x) const {
        return boost::math::quantile(
            boost::math::normal_distribution<Real>(average_, sigma_), x);
    }

    inline MaddockCumulativeNormal::MaddockCumulativeNormal(
        Real average, Real sigma)
    : average_(average), sigma_(sigma) {}

    inline Real MaddockCumulativeNormal::operator()(Real x) const {
        return boost::math::cdf(
            boost::math::normal_distribution<Real>(average_, sigma_), x);
    }
    

}


#endif
