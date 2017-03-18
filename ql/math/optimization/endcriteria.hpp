/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

/*! \file endcriteria.hpp
    \brief Optimization criteria class
*/

#ifndef quantlib_optimization_criteria_hpp
#define quantlib_optimization_criteria_hpp

#include <ql/utilities/null.hpp>
#include <ql/errors.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Criteria to end optimization process:
    /*! - maximum number of iterations AND minimum number of iterations around stationary point
        - x (independent variable) stationary point
        - y=f(x) (dependent variable) stationary point
        - stationary gradient
    */
    class EndCriteria {
      public:
        enum Type {None,
                   MaxIterations,
                   StationaryPoint,
                   StationaryFunctionValue,
                   StationaryFunctionAccuracy,
                   ZeroGradientNorm,
                   Unknown};

        //! Initialization constructor
        EndCriteria(Size maxIterations,
                    Size maxStationaryStateIterations,
                    Real rootEpsilon,
                    Real functionEpsilon,
                    Real gradientNormEpsilon);

        // Inspectors
        Size maxIterations() const;
        Size maxStationaryStateIterations() const;
        Real rootEpsilon() const;
        Real functionEpsilon() const;
        Real gradientNormEpsilon() const;

        /*! Test if the number of iterations is not too big 
            and if a minimum point is not reached */
        bool operator()(const Size iteration,
                        Size& statState,
                        const bool positiveOptimization,
                        const Real fold,
                        const Real normgold,
                        const Real fnew,
                        const Real normgnew,
                        EndCriteria::Type& ecType) const;

        /*! Test if the number of iteration is below MaxIterations */
        bool checkMaxIterations(const Size iteration,
                                  EndCriteria::Type& ecType) const;
        /*! Test if the root variation is below rootEpsilon */
        bool checkStationaryPoint(const Real xOld,
                                  const Real xNew,
                                  Size& statStateIterations,
                                  EndCriteria::Type& ecType) const;
        /*! Test if the function variation is below functionEpsilon */
        bool checkStationaryFunctionValue(const Real fxOld,
                                          const Real fxNew,
                                          Size& statStateIterations,
                                          EndCriteria::Type& ecType) const;
        /*! Test if the function value is below functionEpsilon */
        bool checkStationaryFunctionAccuracy(const Real f,
                                             const bool positiveOptimization,
                                             EndCriteria::Type& ecType) const;
        /*! Test if the gradient norm variation is below gradientNormEpsilon */
        //bool checkZerGradientNormValue(const Real gNormOld, 
        //                               const Real gNormNew,
        //                               EndCriteria::Type& ecType) const;
        /*! Test if the gradient norm value is below gradientNormEpsilon */
        bool checkZeroGradientNorm(const Real gNorm,
                                           EndCriteria::Type& ecType) const;

      protected:
        //! Maximum number of iterations
        Size maxIterations_;
        //! Maximun number of iterations in stationary state
        mutable Size maxStationaryStateIterations_;
        //! root, function and gradient epsilons
        Real rootEpsilon_, functionEpsilon_, gradientNormEpsilon_;

    };

    std::ostream& operator<<(std::ostream& out, EndCriteria::Type ecType);

    // implementation

    inline EndCriteria::EndCriteria(Size maxIterations,
                             Size maxStationaryStateIterations,
                             Real rootEpsilon,
                             Real functionEpsilon,
                             Real gradientNormEpsilon)
    : maxIterations_(maxIterations),
      maxStationaryStateIterations_(maxStationaryStateIterations),
      rootEpsilon_(rootEpsilon),
      functionEpsilon_(functionEpsilon),
      gradientNormEpsilon_(gradientNormEpsilon) {

        if (maxStationaryStateIterations_ == Null<Size>())
            maxStationaryStateIterations_ = std::min(static_cast<Size>(maxIterations/2),
                                                     static_cast<Size>(100));
        QL_REQUIRE(maxStationaryStateIterations_>1,
                   "maxStationaryStateIterations_ (" <<
                   maxStationaryStateIterations_ <<
                   ") must be greater than one");
        QL_REQUIRE(maxStationaryStateIterations_<maxIterations_,
                   "maxStationaryStateIterations_ (" <<
                   maxStationaryStateIterations_ <<
                   ") must be less than maxIterations_ (" <<
                   maxIterations_ << ")");
        if (gradientNormEpsilon_ == Null<Real>())
            gradientNormEpsilon_ = functionEpsilon_;
    }

    inline bool EndCriteria::checkMaxIterations(const Size iteration,
                                         EndCriteria::Type& ecType) const{
        if (iteration < maxIterations_)
            return false;
        ecType = MaxIterations;
        return true;
    }

    inline bool EndCriteria::checkStationaryPoint(const Real xOld,
                                           const Real xNew,
                                           Size& statStateIterations,
                                           EndCriteria::Type& ecType) const {
        if (std::fabs(xNew-xOld) >= rootEpsilon_) {
            statStateIterations = 0;
            return false;
        }
        ++statStateIterations;
        if (statStateIterations <= maxStationaryStateIterations_)
            return false;
        ecType = StationaryPoint;
        return true;
    }

    inline bool EndCriteria::checkStationaryFunctionValue(
                                            const Real fxOld,
                                            const Real fxNew,
                                            Size& statStateIterations,
                                            EndCriteria::Type& ecType) const {
        if (std::fabs(fxNew-fxOld) >= functionEpsilon_) {
            statStateIterations = 0;
            return false;
        }
        ++statStateIterations;
        if (statStateIterations <= maxStationaryStateIterations_)
            return false;
        ecType = StationaryFunctionValue;
        return true;
    }

    inline bool EndCriteria::checkStationaryFunctionAccuracy(
                                            const Real f,
                                            const bool positiveOptimization,
                                            EndCriteria::Type& ecType) const {
        if (!positiveOptimization)
            return false;
        if (f >= functionEpsilon_)
            return false;
        ecType = StationaryFunctionAccuracy;
        return true;
    }

    //bool EndCriteria::checkZerGradientNormValue(
    //                                        const Real gNormOld,
    //                                        const Real gNormNew,
    //                                        EndCriteria::Type& ecType) const {
    //    if (std::fabs(gNormNew-gNormOld) >= gradientNormEpsilon_)
    //        return false;
    //    ecType = StationaryGradient;
    //    return true;
    //}

    inline bool EndCriteria::checkZeroGradientNorm(const Real gradientNorm,
                                            EndCriteria::Type& ecType) const {
        if (gradientNorm >= gradientNormEpsilon_)
            return false;
        ecType = ZeroGradientNorm;
        return true;
    }

    inline bool EndCriteria::operator()(const Size iteration,
                                 Size& statStateIterations,
                                 const bool positiveOptimization,
                                 const Real fold,
                                 const Real, //normgold,
                                 const Real fnew,
                                 const Real normgnew,
                                 EndCriteria::Type& ecType) const {
        return
            checkMaxIterations(iteration, ecType) ||
            checkStationaryFunctionValue(fold, fnew, statStateIterations, ecType) ||
            checkStationaryFunctionAccuracy(fnew, positiveOptimization, ecType) ||
            checkZeroGradientNorm(normgnew, ecType);
    }

    // Inspectors
    inline Size EndCriteria::maxIterations() const {
        return maxIterations_;
    }

    inline Size EndCriteria::maxStationaryStateIterations() const {
        return maxStationaryStateIterations_;
    }

    inline Real EndCriteria::rootEpsilon() const {
        return rootEpsilon_;
    }

    inline Real EndCriteria::functionEpsilon() const {
        return functionEpsilon_;
    }

    inline Real EndCriteria::gradientNormEpsilon() const {
        return gradientNormEpsilon_;
    }

    inline std::ostream& operator<<(std::ostream& out, EndCriteria::Type ec) {
        switch (ec) {
        case QuantLib::EndCriteria::None:
            return out << "None";
        case QuantLib::EndCriteria::MaxIterations:
            return out << "MaxIterations";
        case QuantLib::EndCriteria::StationaryPoint:
            return out << "StationaryPoint";
        case QuantLib::EndCriteria::StationaryFunctionValue:
            return out << "StationaryFunctionValue";
        case QuantLib::EndCriteria::StationaryFunctionAccuracy:
            return out << "StationaryFunctionAccuracy";
        case QuantLib::EndCriteria::ZeroGradientNorm:
            return out << "ZeroGradientNorm";
        case QuantLib::EndCriteria::Unknown:
            return out << "Unknown";
        default:
            QL_FAIL("unknown EndCriteria::Type (" << Integer(ec) << ")");
        }
    }
    

}

#endif
