/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

/*! \file eurodollarfuturesquote.hpp
    \brief quote for the Eurodollar-future implied standard deviation
*/

#ifndef quantlib_eurodollar_futures_quote_hpp
#define quantlib_eurodollar_futures_quote_hpp

#include <ql/quote.hpp>
#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>

namespace QuantLib {

    //! %quote for the Eurodollar-future implied standard deviation
    class EurodollarFuturesImpliedStdDevQuote : public Quote,
                                                public LazyObject {
      public:
        EurodollarFuturesImpliedStdDevQuote(const Handle<Quote>& forward,
                                            const Handle<Quote>& callPrice,
                                            const Handle<Quote>& putPrice,
                                            Real strike,
                                            Real guess = .15,
                                            Real accuracy = 1.0e-6,
                                            Natural maxIter = 100);
        //! \name Quote interface
        //@{
        Real value() const;
        bool isValid() const;
        //@}
      protected:
        void performCalculations() const;
        mutable Real impliedStdev_;
        Real strike_;
        Real accuracy_;
        Natural maxIter_;
        Handle<Quote> forward_;
        Handle<Quote> callPrice_;
        Handle<Quote> putPrice_;
    };

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

#include <ql/pricingengines/blackformula.hpp>

namespace QuantLib {

    inline EurodollarFuturesImpliedStdDevQuote::EurodollarFuturesImpliedStdDevQuote(
                                const Handle<Quote>& forward,
                                const Handle<Quote>& callPrice,
                                const Handle<Quote>& putPrice,
                                Real strike,
                                Real guess,
                                Real accuracy,
                                Natural maxIter)
    : impliedStdev_(guess), strike_(100.0-strike),
      accuracy_(accuracy), maxIter_(maxIter), forward_(forward),
      callPrice_(callPrice), putPrice_(putPrice) {
        registerWith(forward_);
        registerWith(callPrice_);
        registerWith(putPrice_);
    }

    inline Real EurodollarFuturesImpliedStdDevQuote::value() const {
        calculate();
        return impliedStdev_;
    }

    inline bool EurodollarFuturesImpliedStdDevQuote::isValid() const {
        if (forward_.empty() || !forward_->isValid())
            return false;
        Real forwardValue = 100.0-forward_->value();
        if (strike_>forwardValue)
            return !putPrice_.empty() && putPrice_->isValid();
        else
            return !callPrice_.empty() && callPrice_->isValid();
    }

    inline void EurodollarFuturesImpliedStdDevQuote::performCalculations() const {
        static const Real discount = 1.0;
        static const Real displacement = 0.0;
        Real forwardValue = 100.0-forward_->value();
        if (strike_>forwardValue) {
            impliedStdev_ =
                blackFormulaImpliedStdDev(Option::Call, strike_,
                                          forwardValue, putPrice_->value(),
                                          discount, displacement,
                                          impliedStdev_, accuracy_, maxIter_);
        } else {
            impliedStdev_ =
                blackFormulaImpliedStdDev(Option::Put, strike_,
                                          forwardValue, callPrice_->value(),
                                          discount, displacement,
                                          impliedStdev_, accuracy_, maxIter_);
        }
    }
}


#endif
