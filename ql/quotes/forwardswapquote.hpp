/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

/*! \file forwardswapquote.hpp
    \brief quote for a forward starting swap
*/

#ifndef quantlib_forward_swap_quote_hpp
#define quantlib_forward_swap_quote_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Quote for a forward starting swap
    class ForwardSwapQuote : public Quote,
                             public LazyObject {
      public:
        ForwardSwapQuote(const boost::shared_ptr<SwapIndex>& swapIndex,
                         const Handle<Quote>& spread,
                         const Period& fwdStart);
        //! \name Quote interface
        //@{
        Real value() const;
        bool isValid() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        const Date& valueDate() const;
        const Date& startDate() const;
        const Date& fixingDate() const;
      protected:
        void initializeDates();
        void performCalculations() const;

        boost::shared_ptr<SwapIndex> swapIndex_;
        Handle<Quote> spread_;
        Period fwdStart_;

        Date evaluationDate_, valueDate_, startDate_, fixingDate_;
        boost::shared_ptr<VanillaSwap> swap_;

        mutable Rate result_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/settings.hpp>

namespace QuantLib {

    inline ForwardSwapQuote::ForwardSwapQuote(
                    const boost::shared_ptr<SwapIndex>& swapIndex,
                    const Handle<Quote>& spread,
                    const Period& fwdStart)
    : swapIndex_(swapIndex), spread_(spread), fwdStart_(fwdStart) {
        registerWith(swapIndex_);
        registerWith(spread_);
        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
        initializeDates();
    }

    inline void ForwardSwapQuote::initializeDates() {
        valueDate_ = swapIndex_->fixingCalendar().advance(
                                                evaluationDate_,
                                                swapIndex_->fixingDays()*Days,
                                                Following);
        startDate_ = swapIndex_->fixingCalendar().advance(valueDate_,
                                                          fwdStart_,
                                                          Following);
        fixingDate_ = swapIndex_->fixingDate(startDate_);
        swap_ = swapIndex_->underlyingSwap(fixingDate_);
    }

    inline void ForwardSwapQuote::update() {
        if (evaluationDate_ != Settings::instance().evaluationDate()) {
            evaluationDate_ = Settings::instance().evaluationDate();
            initializeDates();
        }
        LazyObject::update();
    }

    inline const Date& ForwardSwapQuote::valueDate() const {
        calculate();
        return valueDate_;
    }

    inline const Date& ForwardSwapQuote::startDate() const {
        calculate();
        return startDate_;
    }

    inline const Date& ForwardSwapQuote::fixingDate() const {
        calculate();
        return fixingDate_;
    }

    inline Real ForwardSwapQuote::value() const {
        calculate();
        return result_;
    }

    inline bool ForwardSwapQuote::isValid() const {
        bool swapIndexIsValid = true;
        try {
            swap_->recalculate();
        } catch (...) {
            swapIndexIsValid = false;
        }
        bool spreadIsValid = spread_.empty() ? true : spread_->isValid();
        return swapIndexIsValid && spreadIsValid;
    }

    inline void ForwardSwapQuote::performCalculations() const {
        // we didn't register as observers - force calculation
        swap_->recalculate();
        // weak implementation... to be improved
        static const Spread basisPoint = 1.0e-4;
        Real floatingLegNPV = swap_->floatingLegNPV();
        Spread spread = spread_.empty() ? 0.0 : spread_->value();
        Real spreadNPV = swap_->floatingLegBPS()/basisPoint*spread;
        Real totNPV = - (floatingLegNPV+spreadNPV);
        result_ = totNPV/(swap_->fixedLegBPS()/basisPoint);
    }
}


#endif
