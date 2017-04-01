/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2005 Gary Kennedy

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

/*! \file gaussianquadratures.hpp
    \brief Integral of a 1-dimensional function using the Gauss quadratures
*/

#ifndef quantlib_gaussian_quadratures_hpp
#define quantlib_gaussian_quadratures_hpp

#include <ql/math/array.hpp>
#include <ql/math/integrals/gaussianorthogonalpolynomial.hpp>
#include <ql/math/matrixutilities/tqreigendecomposition.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>

namespace QuantLib {
    class GaussianOrthogonalPolynomial;

    //! Integral of a 1-dimensional function using the Gauss quadratures method
    /*! References:
        Gauss quadratures and orthogonal polynomials

        G.H. Gloub and J.H. Welsch: Calculation of Gauss quadrature rule.
        Math. Comput. 23 (1986), 221-230

        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        \test the correctness of the result is tested by checking it
              against known good values.
    */
    class GaussianQuadrature {
      public:
        GaussianQuadrature(Size n,
                           const GaussianOrthogonalPolynomial& p);

        template <class F>
        Real operator()(const F& f) const {
            Real sum = 0.0;
            for (Integer i = order()-1; i >= 0; --i) {
                sum += w_[i] * f(x_[i]);
            }
            return sum;
        }

        Size order() const { return x_.size(); }
        const Array& weights() { return w_; }
        const Array& x()       { return x_; }
        
      protected:
        Array x_, w_;
    };


    //! generalized Gauss-Laguerre integration
    /*! This class performs a 1-dimensional Gauss-Laguerre integration.
        \f[
        \int_{0}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;s)=x^s \exp{-x}
        \f]
        and \f[ s > -1 \f]
    */
    class GaussLaguerreIntegration : public GaussianQuadrature {
      public:
        GaussLaguerreIntegration(Size n, Real s = 0.0)
        : GaussianQuadrature(n, GaussLaguerrePolynomial(s)) {}
    };

    //! generalized Gauss-Hermite integration
    /*! This class performs a 1-dimensional Gauss-Hermite integration.
        \f[
        \int_{-\inf}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;\mu)=|x|^{2\mu} \exp{-x*x}
        \f]
        and \f[ \mu > -0.5 \f]
    */
    class GaussHermiteIntegration : public GaussianQuadrature {
      public:
        GaussHermiteIntegration(Size n, Real mu = 0.0)
        : GaussianQuadrature(n, GaussHermitePolynomial(mu)) {}
    };

    //! Gauss-Jacobi integration
    /*! This class performs a 1-dimensional Gauss-Jacobi integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;\alpha,\beta)=(1-x)^\alpha (1+x)^\beta
        \f]
    */
    class GaussJacobiIntegration : public GaussianQuadrature {
      public:
        GaussJacobiIntegration(Size n, Real alpha, Real beta)
        : GaussianQuadrature(n, GaussJacobiPolynomial(alpha, beta)) {}
    };

    //! Gauss-Hyperbolic integration
    /*! This class performs a 1-dimensional Gauss-Hyperbolic integration.
        \f[
        \int_{-\inf}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=1/cosh(x)
        \f]
    */
    class GaussHyperbolicIntegration : public GaussianQuadrature {
      public:
        GaussHyperbolicIntegration(Size n)
        : GaussianQuadrature(n, GaussHyperbolicPolynomial()) {}
    };

    //! Gauss-Legendre integration
    /*! This class performs a 1-dimensional Gauss-Legendre integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=1
        \f]
    */
    class GaussLegendreIntegration : public GaussianQuadrature {
      public:
        GaussLegendreIntegration(Size n)
        : GaussianQuadrature(n, GaussJacobiPolynomial(0.0, 0.0)) {}
    };

    //! Gauss-Chebyshev integration
    /*! This class performs a 1-dimensional Gauss-Chebyshev integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=(1-x^2)^{-1/2}
        \f]
    */
    class GaussChebyshevIntegration : public GaussianQuadrature {
      public:
        GaussChebyshevIntegration(Size n)
        : GaussianQuadrature(n, GaussJacobiPolynomial(-0.5, -0.5)) {}
    };

    //! Gauss-Chebyshev integration (second kind)
    /*! This class performs a 1-dimensional Gauss-Chebyshev integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=(1-x^2)^{1/2}
        \f]
    */
    class GaussChebyshev2ndIntegration : public GaussianQuadrature {
      public:
        GaussChebyshev2ndIntegration(Size n)
      : GaussianQuadrature(n, GaussJacobiPolynomial(0.5, 0.5)) {}
    };

    //! Gauss-Gegenbauer integration
    /*! This class performs a 1-dimensional Gauss-Gegenbauer integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=(1-x^2)^{\lambda-1/2}
        \f]
    */
    class GaussGegenbauerIntegration : public GaussianQuadrature {
      public:
        GaussGegenbauerIntegration(Size n, Real lambda)
        : GaussianQuadrature(n, GaussJacobiPolynomial(lambda-0.5, lambda-0.5))
        {}
    };


    //! tabulated Gauss-Legendre quadratures
    class TabulatedGaussLegendre {
      public:
        TabulatedGaussLegendre(Size n = 20) { order(n); }
        template <class F>
        Real operator() (const F& f) const {
            QL_ASSERT(w_!=0, "Null weights" );
            QL_ASSERT(x_!=0, "Null abscissas");
            Size startIdx;
            Real val;

            const Size isOrderOdd = order_ & 1;

            if (isOrderOdd) {
              QL_ASSERT((n_>0), "assume at least 1 point in quadrature");
              val = w_[0]*f(x_[0]);
              startIdx=1;
            } else {
              val = 0.0;
              startIdx=0;
            }

            for (Size i=startIdx; i<n_; ++i) {
                val += w_[i]*f( x_[i]);
                val += w_[i]*f(-x_[i]);
            }
            return val;
        }

        void order(Size);
        Size order() const { return order_; }

      private:
        Size order_;

        const Real* w_;
        const Real* x_;
        Size  n_;

        // Abscissas and Weights from Abramowitz and Stegun

        /* order 6 */
        static const Real* x6() {
            static const Real tmp[] = {0.238619186083197, 0.661209386466265, 0.932469514203152};
            return tmp;
        }

        static const Real* w6() {
            static const Real tmp[] = {0.467913934572691, 0.360761573048139, 0.171324492379170};
            return tmp;
        }

        static const Size n6 = 3;

        /* order 7 */
        static const Real* x7() {
            static const Real tmp[] = {0.000000000000000, 0.405845151377397, 0.741531185599394, 0.949107912342759};
            return tmp;
        }

        static const Real* w7() {
            static const Real tmp[] = {0.417959183673469, 0.381830050505119, 0.279705391489277, 0.129484966168870};
            return tmp;
        }

        static const Size n7 = 4;

        /* order 12 */
        static const Real* x12() {
            static const Real tmp[] = {0.125233408511469, 0.367831498998180, 0.587317954286617,
                                  0.769902674194305, 0.904117256370475, 0.981560634246719};
            return tmp;
        }

        static const Real* w12() {
            static const Real tmp[] = {0.249147045813403, 0.233492536538355, 0.203167426723066,
                                  0.160078328543346, 0.106939325995318, 0.047175336386512};
            return tmp;
        }

        static const Size n12 = 6;

        /* order 20 */
        static const Real* x20() {
            static const Real tmp[] = {0.076526521133497, 0.227785851141645, 0.373706088715420, 0.510867001950827,
                                0.636053680726515, 0.746331906460151, 0.839116971822219, 0.912234428251326,
                                0.963971927277914, 0.993128599185095};
            return tmp;
        }

        static const Real* w20() {
            static const Real tmp[] = {0.152753387130726, 0.149172986472604, 0.142096109318382, 0.131688638449177,
                                0.118194531961518, 0.101930119817240, 0.083276741576704, 0.062672048334109,
                                0.040601429800387, 0.017614007139152};
            return tmp;
        }

        static const Size n20 = 10;
    };

    // implementation

    inline GaussianQuadrature::GaussianQuadrature(
                                Size n,
                                const GaussianOrthogonalPolynomial& orthPoly)
    : x_(n), w_(n) {

        // set-up matrix to compute the roots and the weights
        Array e(n-1);

        Size i;
        for (i=1; i < n; ++i) {
            x_[i] = orthPoly.alpha(i);
            e[i-1] = std::sqrt(orthPoly.beta(i));
        }
        x_[0] = orthPoly.alpha(0);

        TqrEigenDecomposition tqr(
                               x_, e,
                               TqrEigenDecomposition::OnlyFirstRowEigenVector,
                               TqrEigenDecomposition::Overrelaxation);

        x_ = tqr.eigenvalues();
        const Matrix& ev = tqr.eigenvectors();

        Real mu_0 = orthPoly.mu_0();
        for (i=0; i<n; ++i) {
            w_[i] = mu_0*ev[0][i]*ev[0][i] / orthPoly.w(x_[i]);
        }
    }


    inline void TabulatedGaussLegendre::order(Size order) {
        switch(order) {
          case(6):
              order_=order; x_=x6(); w_=w6(); n_=n6;
            break;
          case(7):
              order_=order; x_=x7(); w_=w7(); n_=n7;
            break;
          case(12):
              order_=order; x_=x12(); w_=w12(); n_=n12;
            break;
          case(20):
              order_=order; x_=x20(); w_=w20(); n_=n20;
            break;
          default:
            QL_FAIL("order " << order << " not supported");
        }
    }

}

#endif
