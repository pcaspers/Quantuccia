/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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


#ifndef quantlib_sobol_brownian_generator_hpp
#define quantlib_sobol_brownian_generator_hpp

#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/math/randomnumbers/inversecumulativersg.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/methods/montecarlo/brownianbridge.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <boost/iterator/permutation_iterator.hpp>
#include <vector>

namespace QuantLib {

    //! Sobol Brownian generator for market-model simulations
    /*! Incremental Brownian generator using a Sobol generator,
        inverse-cumulative Gaussian method, and Brownian bridging.
    */
    class SobolBrownianGenerator : public BrownianGenerator {
      public:
        enum Ordering {
            Factors,  /*!< The variates with the best quality will be
                           used for the evolution of the first factor. */
            Steps,    /*!< The variates with the best quality will be
                           used for the largest steps of all factors. */
            Diagonal  /*!< A diagonal schema will be used to assign
                           the variates with the best quality to the
                           most important factors and the largest
                           steps. */
        };
        SobolBrownianGenerator(
                           Size factors,
                           Size steps,
                           Ordering ordering,
                           unsigned long seed = 0,
                           SobolRsg::DirectionIntegers directionIntegers
                                                        = SobolRsg::Jaeckel);

        Real nextPath();
        Real nextStep(std::vector<Real>&);

        Size numberOfFactors() const;
        Size numberOfSteps() const;
        
        // test interface
        const std::vector<std::vector<Size> >& orderedIndices() const;
        std::vector<std::vector<Real> > transform(
                              const std::vector<std::vector<Real> >& variates);

      private:
        Size factors_, steps_;
        Ordering ordering_;
        InverseCumulativeRsg<SobolRsg,InverseCumulativeNormal> generator_;
        BrownianBridge bridge_;
        // work variables
        Size lastStep_;
        std::vector<std::vector<Size> > orderedIndices_;
        std::vector<std::vector<Real> > bridgedVariates_;
    };

    class SobolBrownianGeneratorFactory : public BrownianGeneratorFactory {
      public:
        SobolBrownianGeneratorFactory(
                           SobolBrownianGenerator::Ordering ordering,
                           unsigned long seed = 0,
                           SobolRsg::DirectionIntegers directionIntegers
                                                         = SobolRsg::Jaeckel);
        boost::shared_ptr<BrownianGenerator> create(Size factors,
                                                    Size steps) const;
      private:
        SobolBrownianGenerator::Ordering ordering_;
        unsigned long seed_;
        SobolRsg::DirectionIntegers integers_;
    };

    // implementation

    namespace {

       inline  void fillByFactor(std::vector<std::vector<Size> >& M,
                          Size factors, Size steps) {
            Size counter = 0;
            for (Size i=0; i<factors; ++i)
                for (Size j=0; j<steps; ++j)
                    M[i][j] = counter++;
        }

        inline void fillByStep(std::vector<std::vector<Size> >& M,
                        Size factors, Size steps) {
            Size counter = 0;
            for (Size j=0; j<steps; ++j)
                for (Size i=0; i<factors; ++i)
                    M[i][j] = counter++;
        }

        // variate 2 is used for the second factor's full path
        inline void fillByDiagonal(std::vector<std::vector<Size> >& M,
                            Size factors, Size steps) {
            // starting position of the current diagonal
            Size i0 = 0, j0 = 0;
            // current position
            Size i=0, j=0;
            Size counter = 0;
            while (counter < factors*steps) {
                M[i][j] = counter++;
                if (i == 0 || j == steps-1) {
                    // we completed a diagonal and have to start a new one
                    if (i0 < factors-1) {
                        // we start the path of the next factor
                        i0 = i0+1;
                        j0 = 0;
                    } else {
                        // we move along the path of the last factor
                        i0 = factors-1;
                        j0 = j0+1;
                    }
                    i = i0;
                    j = j0;
                } else {
                    // we move along the diagonal
                    i = i-1;
                    j = j+1;
                }
            }
        }

        /*
        // variate 2 is used for the first factor's half path
        void fillByDiagonal(std::vector<std::vector<Size> >& M,
                            Size factors, Size steps) {
            // starting position of the current diagonal
            Size i0 = 0, j0 = 0;
            // current position
            Size i=0, j=0;
            Size counter = 0;
            while (counter < factors*steps) {
                M[i][j] = counter++;
                if (j == 0 || i == factors-1) {
                    // we completed a diagonal and have to start a new one
                    if (j0 < steps-1) {
                        // we move along the path of the first factor
                        i0 = 0;
                        j0 = j0+1;
                    } else {
                        // we complete the next path
                        i0 = i0+1;
                        j0 = steps-1;
                    }
                    i = i0;
                    j = j0;
                } else {
                    // we move along the diagonal
                    i = i+1;
                    j = j-1;
                }
            }
        }
        */

    }


    inline SobolBrownianGenerator::SobolBrownianGenerator(
                                        Size factors,
                                        Size steps,
                                        Ordering ordering,
                                        unsigned long seed,
                                        SobolRsg::DirectionIntegers integers)
    : factors_(factors), steps_(steps), ordering_(ordering),
      generator_(SobolRsg(factors*steps, seed, integers),
                 InverseCumulativeNormal()),
      bridge_(steps), lastStep_(0),
      orderedIndices_(factors, std::vector<Size>(steps)),
      bridgedVariates_(factors, std::vector<Real>(steps)) {

        switch (ordering_) {
          case Factors:
            fillByFactor(orderedIndices_, factors_, steps_);
            break;
          case Steps:
            fillByStep(orderedIndices_, factors_, steps_);
            break;
          case Diagonal:
            fillByDiagonal(orderedIndices_, factors_, steps_);
            break;
          default:
            QL_FAIL("unknown ordering");
        }
    }


    inline Real SobolBrownianGenerator::nextPath() {
        typedef InverseCumulativeRsg<SobolRsg,
                                     InverseCumulativeNormal>::sample_type
            sample_type;

        const sample_type& sample = generator_.nextSequence();
        // Brownian-bridge the variates according to the ordered indices
        for (Size i=0; i<factors_; ++i) {
            bridge_.transform(boost::make_permutation_iterator(
                                                  sample.value.begin(),
                                                  orderedIndices_[i].begin()),
                              boost::make_permutation_iterator(
                                                  sample.value.begin(),
                                                  orderedIndices_[i].end()),
                              bridgedVariates_[i].begin());
        }
        lastStep_ = 0;
        return sample.weight;
    }
    
    
    inline const std::vector<std::vector<Size> >& 
    SobolBrownianGenerator::orderedIndices() const {
        return orderedIndices_;
    }

    inline std::vector<std::vector<Real> > SobolBrownianGenerator::transform(
                            const std::vector<std::vector<Real> >& variates) {
        
        QL_REQUIRE(   (variates.size() == factors_*steps_),
                   "inconsistent variate vector");

        const Size dim    = factors_*steps_;
        const Size nPaths = variates.front().size();
        
        std::vector<std::vector<Real> > 
                       retVal(factors_, std::vector<Real>(nPaths*steps_));
        
        for (Size j=0; j < nPaths; ++j) {
            std::vector<Real> sample(steps_*factors_);
            for (Size k=0; k < dim; ++k) {
                sample[k] = variates[k][j];
            }
            for (Size i=0; i<factors_; ++i) {
                bridge_.transform(boost::make_permutation_iterator(
                                                  sample.begin(),
                                                  orderedIndices_[i].begin()),
                          boost::make_permutation_iterator(
                                                  sample.begin(),
                                                  orderedIndices_[i].end()),
                          retVal[i].begin()+j*steps_);
            }
        }
        
        return retVal;
    }

    inline Real SobolBrownianGenerator::nextStep(std::vector<Real>& output) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(output.size() == factors_, "size mismatch");
        QL_REQUIRE(lastStep_<steps_, "sequence exhausted");
        #endif
        for (Size i=0; i<factors_; ++i)
            output[i] = bridgedVariates_[i][lastStep_];
        ++lastStep_;
        return 1.0;
    }

    inline Size SobolBrownianGenerator::numberOfFactors() const { return factors_; }

    inline Size SobolBrownianGenerator::numberOfSteps() const { return steps_; }



    inline SobolBrownianGeneratorFactory::SobolBrownianGeneratorFactory(
                                    SobolBrownianGenerator::Ordering ordering,
                                    unsigned long seed,
                                    SobolRsg::DirectionIntegers integers)
    : ordering_(ordering), seed_(seed), integers_(integers) {}

    inline boost::shared_ptr<BrownianGenerator>
    SobolBrownianGeneratorFactory::create(Size factors, Size steps) const {
        return boost::shared_ptr<BrownianGenerator>(
                         new SobolBrownianGenerator(factors, steps, ordering_,
                                                    seed_, integers_));
    }


}


#endif
