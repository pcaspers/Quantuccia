/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file makecapfloor.hpp
    \brief Helper class to instantiate standard market cap/floor.
*/

#ifndef quantlib_instruments_makecapfloor_hpp
#define quantlib_instruments_makecapfloor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/instruments/makevanillaswap.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market cap and floor.
    */
    class MakeCapFloor {
      public:
        MakeCapFloor(CapFloor::Type capFloorType,
                     const Period& capFloorTenor,
                     const boost::shared_ptr<IborIndex>& iborIndex,
                     Rate strike = Null<Rate>(),
                     const Period& forwardStart = 0*Days);

        operator CapFloor() const;
        operator boost::shared_ptr<CapFloor>() const;

        MakeCapFloor& withNominal(Real n);
        MakeCapFloor& withEffectiveDate(const Date& effectiveDate,
                                        bool firstCapletExcluded);
        MakeCapFloor& withTenor(const Period& t);
        MakeCapFloor& withCalendar(const Calendar& cal);
        MakeCapFloor& withConvention(BusinessDayConvention bdc);
        MakeCapFloor& withTerminationDateConvention(BusinessDayConvention bdc);
        MakeCapFloor& withRule(DateGeneration::Rule r);
        MakeCapFloor& withEndOfMonth(bool flag = true);
        MakeCapFloor& withFirstDate(const Date& d);
        MakeCapFloor& withNextToLastDate(const Date& d);
        MakeCapFloor& withDayCount(const DayCounter& dc);

        //! only get last coupon
        MakeCapFloor& asOptionlet(bool b = true);

        MakeCapFloor& withPricingEngine(
                              const boost::shared_ptr<PricingEngine>& engine);
      private:
        CapFloor::Type capFloorType_;
        Rate strike_;
        bool firstCapletExcluded_, asOptionlet_;

        MakeVanillaSwap makeVanillaSwap_;

        boost::shared_ptr<PricingEngine> engine_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    inline MakeCapFloor::MakeCapFloor(CapFloor::Type capFloorType,
                               const Period& tenor,
                               const shared_ptr<IborIndex>& iborIndex,
                               Rate strike,
                               const Period& forwardStart)
    : capFloorType_(capFloorType), strike_(strike),
      firstCapletExcluded_(forwardStart==0*Days), asOptionlet_(false),
      // setting the fixed leg tenor avoids that MakeVanillaSwap throws
      // because of an unknown fixed leg default tenor for a currency,
      // notice that only the floating leg of the swap is used anyway
      makeVanillaSwap_(MakeVanillaSwap(tenor, iborIndex, 0.0, forwardStart)
                       .withFixedLegTenor(1*Years)) {}

    inline MakeCapFloor::operator CapFloor() const {
        shared_ptr<CapFloor> capfloor = *this;
        return *capfloor;
    }

    inline MakeCapFloor::operator shared_ptr<CapFloor>() const {

        VanillaSwap swap = makeVanillaSwap_;

        Leg leg = swap.floatingLeg();
        if (firstCapletExcluded_)
            leg.erase(leg.begin());

        // only leaves the last coupon
        if (asOptionlet_ && leg.size() > 1) {
            Leg::iterator end = leg.end();  // Sun Studio needs an lvalue
            leg.erase(leg.begin(), --end);
        }

        std::vector<Rate> strikeVector(1, strike_);
        if (strike_ == Null<Rate>()) {

            // temporary patch...
            // should be fixed for every CapFloor::Engine
            shared_ptr<BlackCapFloorEngine> temp = 
                dynamic_pointer_cast<BlackCapFloorEngine>(engine_);
            QL_REQUIRE(temp,
                       "cannot calculate ATM without a BlackCapFloorEngine");
            Handle<YieldTermStructure> discountCurve = temp->termStructure();
            strikeVector[0] = CashFlows::atmRate(leg,
                                                 **discountCurve,
                                                 false,
                                                 discountCurve->referenceDate());
        }

        shared_ptr<CapFloor> capFloor(new
            CapFloor(capFloorType_, leg, strikeVector));
        capFloor->setPricingEngine(engine_);
        return capFloor;
    }

    inline MakeCapFloor& MakeCapFloor::withNominal(Real n) {
        makeVanillaSwap_.withNominal(n);
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::withEffectiveDate(const Date& effectiveDate,
                                                  bool firstCapletExcluded) {
        makeVanillaSwap_.withEffectiveDate(effectiveDate);
        firstCapletExcluded_ = firstCapletExcluded;
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::withTenor(const Period& t) {
        makeVanillaSwap_.withFloatingLegTenor(t);
        return *this;
    }


    inline MakeCapFloor& MakeCapFloor::withCalendar(const Calendar& cal) {
        makeVanillaSwap_.withFloatingLegCalendar(cal);
        return *this;
    }


    inline MakeCapFloor& MakeCapFloor::withConvention(BusinessDayConvention bdc) {
        makeVanillaSwap_.withFloatingLegConvention(bdc);
        return *this;
    }


    inline MakeCapFloor&
    MakeCapFloor::withTerminationDateConvention(BusinessDayConvention bdc) {
        makeVanillaSwap_.withFloatingLegTerminationDateConvention(bdc);
        return *this;
    }


    inline MakeCapFloor& MakeCapFloor::withRule(DateGeneration::Rule r) {
        makeVanillaSwap_.withFloatingLegRule(r);
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::withEndOfMonth(bool flag) {
        makeVanillaSwap_.withFloatingLegEndOfMonth(flag);
        return *this;
    }


    inline MakeCapFloor& MakeCapFloor::withFirstDate(const Date& d) {
        makeVanillaSwap_.withFloatingLegFirstDate(d);
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::withNextToLastDate(const Date& d) {
        makeVanillaSwap_.withFloatingLegNextToLastDate(d);
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::withDayCount(const DayCounter& dc) {
        makeVanillaSwap_.withFloatingLegDayCount(dc);
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::asOptionlet(bool b) {
        asOptionlet_ = b;
        return *this;
    }

    inline MakeCapFloor& MakeCapFloor::withPricingEngine(
                             const shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}


#endif
