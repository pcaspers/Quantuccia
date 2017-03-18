/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file gaussianorthogonalpolynomial.hpp
    \brief orthogonal polynomials for gaussian quadratures
*/

#ifndef quantlib_gaussian_orthogonal_polynomial_hpp
#define quantlib_gaussian_orthogonal_polynomial_hpp

#include <ql/types.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/errors.hpp>
#include <cmath>

namespace QuantLib {

    //! orthogonal polynomial for Gaussian quadratures
    /*! References:
        Gauss quadratures and orthogonal polynomials

        G.H. Gloub and J.H. Welsch: Calculation of Gauss quadrature rule.
        Math. Comput. 23 (1986), 221-230

        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        The polynomials are defined by the three-term recurrence relation
        \f[
        P_{k+1}(x)=(x-\alpha_k) P_k(x) - \beta_k P_{k-1}(x)
        \f]
        and
        \f[
        \mu_0 = \int{w(x)dx}
        \f]
    */
    class GaussianOrthogonalPolynomial {
      public:
        virtual ~GaussianOrthogonalPolynomial() {}
        virtual Real mu_0()        const = 0;
        virtual Real alpha(Size i) const = 0;
        virtual Real beta(Size i)  const = 0;
        virtual Real w(Real x)     const = 0;

        Real value(Size i, Real x) const;
        Real weightedValue(Size i, Real x) const;
    };

    //! Gauss-Laguerre polynomial
    class GaussLaguerrePolynomial : public GaussianOrthogonalPolynomial {
      public:
        GaussLaguerrePolynomial(Real s = 0.0);

        Real mu_0() const;
        Real alpha(Size i) const;
        Real beta(Size i) const;
        Real w(Real x) const;

      private:
        const Real s_;
    };

    //! Gauss-Hermite polynomial
    class GaussHermitePolynomial : public GaussianOrthogonalPolynomial {
      public:
        GaussHermitePolynomial(Real mu = 0.0);

        Real mu_0()const;
        Real alpha(Size i) const;
        Real beta(Size i) const;
        Real w(Real x) const;

      private:
        const Real mu_;
    };

    //! Gauss-Jacobi polynomial
    class GaussJacobiPolynomial : public GaussianOrthogonalPolynomial {
      public:
        GaussJacobiPolynomial(Real alpha, Real beta);

        Real mu_0() const;
        Real alpha(Size i) const;
        Real beta(Size i) const;
        Real w(Real x) const;

      private:
        const Real alpha_;
        const Real beta_;
    };

    //! Gauss-Legendre polynomial
    class GaussLegendrePolynomial : public GaussJacobiPolynomial {
      public:
        GaussLegendrePolynomial();
    };

    //! Gauss-Chebyshev polynomial
    class GaussChebyshevPolynomial : public GaussJacobiPolynomial {
      public:
        GaussChebyshevPolynomial();
    };

    //! Gauss-Chebyshev polynomial (second kind)
    class GaussChebyshev2ndPolynomial : public GaussJacobiPolynomial {
      public:
        GaussChebyshev2ndPolynomial();
    };

    //! Gauss-Gegenbauer polynomial
    class GaussGegenbauerPolynomial : public GaussJacobiPolynomial {
      public:
        GaussGegenbauerPolynomial(Real lambda);
    };

    //! Gauss hyperbolic polynomial
    class GaussHyperbolicPolynomial : public GaussianOrthogonalPolynomial {
      public:
        Real mu_0()const;
        Real alpha(Size i) const;
        Real beta(Size i) const;
        Real w(Real x) const;
    };

    // implementation

    inline Real GaussianOrthogonalPolynomial::value(Size n, Real x) const {
        if (n > 1) {
            return  (x-alpha(n-1)) * value(n-1, x)
                       - beta(n-1) * value(n-2, x);
        }
        else if (n == 1) {
            return x-alpha(0);
        }

        return 1;
    }

    inline Real GaussianOrthogonalPolynomial::weightedValue(Size n, Real x) const {
        return std::sqrt(w(x))*value(n, x);
    }

    inline GaussLaguerrePolynomial::GaussLaguerrePolynomial(Real s)
    : s_(s) {
        QL_REQUIRE(s > -1.0, "s must be bigger than -1");
    }

    inline Real GaussLaguerrePolynomial::mu_0() const {
        return std::exp(GammaFunction().logValue(s_+1));
    }

    inline Real GaussLaguerrePolynomial::alpha(Size i) const {
        return 2*i+1+s_;
    }

    inline Real GaussLaguerrePolynomial::beta(Size i) const {
        return i*(i+s_);
    }

    inline Real GaussLaguerrePolynomial::w(Real x) const {
        return std::pow(x, s_)*std::exp(-x);
    }


    inline GaussHermitePolynomial::GaussHermitePolynomial(Real mu)
    : mu_(mu) {
        QL_REQUIRE(mu > -0.5, "mu must be bigger than -0.5");
    }

    inline Real GaussHermitePolynomial::mu_0() const {
        return std::exp(GammaFunction().logValue(mu_+0.5));
    }

    inline Real GaussHermitePolynomial::alpha(Size) const {
        return 0.0;
    }

    inline Real GaussHermitePolynomial::beta(Size i) const {
        return (i%2)? i/2.0 + mu_ : i/2.0;
    }

    inline Real GaussHermitePolynomial::w(Real x) const {
        return std::pow(std::fabs(x), 2*mu_)*std::exp(-x*x);
    }

    inline GaussJacobiPolynomial::GaussJacobiPolynomial(Real alpha, Real beta)
    : alpha_(alpha), beta_ (beta) {
        QL_REQUIRE(alpha_+beta_ > -2.0,"alpha+beta must be bigger than -2");
        QL_REQUIRE(alpha_       > -1.0,"alpha must be bigger than -1");
        QL_REQUIRE(beta_        > -1.0,"beta  must be bigger than -1");
    }

    inline Real GaussJacobiPolynomial::mu_0() const {
        return std::pow(2.0, alpha_+beta_+1)
            * std::exp( GammaFunction().logValue(alpha_+1)
                        +GammaFunction().logValue(beta_ +1)
                        -GammaFunction().logValue(alpha_+beta_+2));
    }

    inline Real GaussJacobiPolynomial::alpha(Size i) const {
        Real num = beta_*beta_ - alpha_*alpha_;
        Real denom = (2.0*i+alpha_+beta_)*(2.0*i+alpha_+beta_+2);

        if (!denom) {
            if (num != 0.0) {
                QL_FAIL("can't compute a_k for jacobi integration\n");
            }
            else {
                // l'Hospital
                num  = 2*beta_;
                denom= 2*(2.0*i+alpha_+beta_+1);

                QL_ASSERT(denom, "can't compute a_k for jacobi integration\n");
            }
        }

        return num / denom;
    }

    inline Real GaussJacobiPolynomial::beta(Size i) const {
        Real num = 4.0*i*(i+alpha_)*(i+beta_)*(i+alpha_+beta_);
        Real denom = (2.0*i+alpha_+beta_)*(2.0*i+alpha_+beta_)
                   * ((2.0*i+alpha_+beta_)*(2.0*i+alpha_+beta_)-1);

        if (!denom) {
            if (num != 0.0) {
                QL_FAIL("can't compute b_k for jacobi integration\n");
            } else {
                // l'Hospital
                num  = 4.0*i*(i+beta_)* (2.0*i+2*alpha_+beta_);
                denom= 2.0*(2.0*i+alpha_+beta_);
                denom*=denom-1;
                QL_ASSERT(denom, "can't compute b_k for jacobi integration\n");
            }
        }
        return num / denom;
    }

    inline Real GaussJacobiPolynomial::w(Real x) const {
        return std::pow(1-x, alpha_)*std::pow(1+x, beta_);
    }


    inline GaussLegendrePolynomial::GaussLegendrePolynomial()
    : GaussJacobiPolynomial(0.0, 0.0) {
    }

    inline GaussChebyshev2ndPolynomial::GaussChebyshev2ndPolynomial()
    : GaussJacobiPolynomial(0.5, 0.5) {
    }

    inline GaussChebyshevPolynomial::GaussChebyshevPolynomial()
    : GaussJacobiPolynomial(-0.5, -0.5) {
    }

    inline GaussGegenbauerPolynomial::GaussGegenbauerPolynomial(Real lambda)
    : GaussJacobiPolynomial(lambda-0.5, lambda-0.5){
    }

    inline Real GaussHyperbolicPolynomial::mu_0() const {
        return M_PI;
    }

    inline Real GaussHyperbolicPolynomial::alpha(Size) const {
        return 0.0;
    }

    inline Real GaussHyperbolicPolynomial::beta(Size i) const {
        return i ? M_PI_2*M_PI_2*i*i : M_PI;
    }

    inline Real GaussHyperbolicPolynomial::w(Real x) const {
        return 1/std::cosh(x);
    }

}

#endif
