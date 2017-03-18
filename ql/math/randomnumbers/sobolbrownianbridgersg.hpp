/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file sobolbrownianbridgersg.hpp
    \brief interface class to map the functionality of SobolBrownianGenerator
           to the "conventional" sequence generator interface
*/

#ifndef quantlib_sobol_brownian_bridge_rsg_hpp
#define quantlib_sobol_brownian_bridge_rsg_hpp

#include <ql/models/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>

namespace QuantLib {

    class SobolBrownianBridgeRsg {
      public:
        typedef Sample<std::vector<Real> > sample_type;

        SobolBrownianBridgeRsg(Size factors, Size steps,
                               SobolBrownianGenerator::Ordering ordering
                                   = SobolBrownianGenerator::Diagonal,
                               unsigned long seed = 0,
                               SobolRsg::DirectionIntegers directionIntegers
                                   = SobolRsg::JoeKuoD7);

        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const;
        Size dimension() const;

      private:
        const Size factors_, steps_, dim_;
        mutable sample_type seq_;
        mutable SobolBrownianGenerator gen_;
    };

    // implementation

    inline SobolBrownianBridgeRsg::SobolBrownianBridgeRsg(
        Size factors, Size steps,
        SobolBrownianGenerator::Ordering ordering,
        unsigned long seed,
        SobolRsg::DirectionIntegers directionIntegers)
    : factors_(factors), steps_(steps), dim_(factors*steps),
      seq_(sample_type::value_type(factors*steps), 1.0),
      gen_(factors, steps, ordering, seed, directionIntegers) {
    }

    inline const SobolBrownianBridgeRsg::sample_type&
    SobolBrownianBridgeRsg::nextSequence() const {
        gen_.nextPath();
        std::vector<Real> output(factors_);
        for (Size i=0; i < steps_; ++i) {
            gen_.nextStep(output);
            std::copy(output.begin(), output.end(),
                      seq_.value.begin()+i*factors_);
        }

        return seq_;
    }

    inline const SobolBrownianBridgeRsg::sample_type&
    SobolBrownianBridgeRsg::lastSequence() const {
        return seq_;
    }

    inline Size SobolBrownianBridgeRsg::dimension() const {
        return dim_;
    }

}

#endif
