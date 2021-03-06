/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2013 Peter Caspers

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

/*! \file projection.hpp
    \brief Parameter projection
*/

#ifndef quantlib_math_projection_h
#define quantlib_math_projection_h

#include <ql/math/array.hpp>

namespace QuantLib {

    class Projection {
      public:
        Projection(const Array &parameterValues,
                   const std::vector<bool> &fixParameters = std::vector<bool>());

        //! returns the subset of free parameters corresponding
        // to set of parameters
        virtual Disposable<Array> project(const Array &parameters) const;

        //! returns whole set of parameters corresponding to the set
        // of projected parameters
        virtual Disposable<Array>
        include(const Array &projectedParameters) const;
        virtual ~Projection() {}
      protected:
        void mapFreeParameters(const Array &parameterValues) const;
        Size numberOfFreeParameters_;
        const Array fixedParameters_;
        mutable Array actualParameters_;
        std::vector<bool> fixParameters_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2013 Peter Caspers

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

    inline Projection::Projection(const Array &parameterValues,
                           const std::vector<bool> &fixParameters)
        : numberOfFreeParameters_(0), fixedParameters_(parameterValues),
          actualParameters_(parameterValues),
          fixParameters_(fixParameters) {

        if (fixParameters_.size() == 0)
            fixParameters_ =
                std::vector<bool>(actualParameters_.size(), false);

        QL_REQUIRE(fixedParameters_.size() == fixParameters_.size(),
                   "fixedParameters_.size()!=parametersFreedoms_.size()");
        for (Size i = 0; i < fixParameters_.size(); i++)
            if (!fixParameters_[i])
                numberOfFreeParameters_++;
        QL_REQUIRE(numberOfFreeParameters_ > 0, "numberOfFreeParameters==0");

    }

    inline void Projection::mapFreeParameters(const Array &parameterValues) const {

        QL_REQUIRE(parameterValues.size() == numberOfFreeParameters_,
                   "parameterValues.size()!=numberOfFreeParameters");
        Size i = 0;
        for (Size j = 0; j < actualParameters_.size(); j++)
            if (!fixParameters_[j])
                actualParameters_[j] = parameterValues[i++];

    }

    inline Disposable<Array> Projection::project(const Array &parameters) const {

        QL_REQUIRE(parameters.size() == fixParameters_.size(),
                   "parameters.size()!=parametersFreedoms_.size()");
        Array projectedParameters(numberOfFreeParameters_);
        Size i = 0;
        for (Size j = 0; j < fixParameters_.size(); j++)
            if (!fixParameters_[j])
                projectedParameters[i++] = parameters[j];
        return projectedParameters;

    }

    inline Disposable<Array>
    Projection::include(const Array &projectedParameters) const {

        QL_REQUIRE(projectedParameters.size() == numberOfFreeParameters_,
                   "projectedParameters.size()!=numberOfFreeParameters");
        Array y(fixedParameters_);
        Size i = 0;
        for (Size j = 0; j < y.size(); j++)
            if (!fixParameters_[j])
                y[j] = projectedParameters[i++];
        return y;

    }
}

#endif
