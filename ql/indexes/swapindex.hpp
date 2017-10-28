/*
 Copyright (C) 2006, 2009 Ferdinando Ametrano
 Copyright (C) 2006, 2007, 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file swapindex.hpp
    \brief swap-rate indexes
*/

#ifndef quantlib_swapindex_hpp
#define quantlib_swapindex_hpp

#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    class Schedule;

    class IborIndex;
    class VanillaSwap;

    class OvernightIndex;
    class OvernightIndexedSwap;

    //! base class for swap-rate indexes
    class SwapIndex : public InterestRateIndex {
      public:
        SwapIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  Currency currency,
                  const Calendar& fixingCalendar,
                  const Period& fixedLegTenor,
                  BusinessDayConvention fixedLegConvention,
                  const DayCounter& fixedLegDayCounter,
                  const boost::shared_ptr<IborIndex>& iborIndex);
        SwapIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  Currency currency,
                  const Calendar& fixingCalendar,
                  const Period& fixedLegTenor,
                  BusinessDayConvention fixedLegConvention,
                  const DayCounter& fixedLegDayCounter,
                  const boost::shared_ptr<IborIndex>& iborIndex,
                  const Handle<YieldTermStructure>& discountingTermStructure);
        //! \name InterestRateIndex interface
        //@{
        Date maturityDate(const Date& valueDate) const;
        //@}
        //! \name Inspectors
        //@{
        Period fixedLegTenor() const { return fixedLegTenor_; }
        BusinessDayConvention fixedLegConvention() const;
        boost::shared_ptr<IborIndex> iborIndex() const { return iborIndex_; }
        Handle<YieldTermStructure> forwardingTermStructure() const;
        Handle<YieldTermStructure> discountingTermStructure() const;
        bool exogenousDiscount() const;
        /*! \warning Relinking the term structure underlying the index will
                     not have effect on the returned swap.
        */
        boost::shared_ptr<VanillaSwap> underlyingSwap(
                                                const Date& fixingDate) const;
        //@}
        //! \name Other methods
        //@{
        //! returns a copy of itself linked to a different forwarding curve
        virtual boost::shared_ptr<SwapIndex> clone(
                        const Handle<YieldTermStructure>& forwarding) const;
        //! returns a copy of itself linked to different curves
        virtual boost::shared_ptr<SwapIndex> clone(
                        const Handle<YieldTermStructure>& forwarding,
                        const Handle<YieldTermStructure>& discounting) const;
        //! returns a copy of itself with different tenor
        virtual boost::shared_ptr<SwapIndex> clone(
                        const Period& tenor) const;
        // @}
      protected:
        Rate forecastFixing(const Date& fixingDate) const;
        Period tenor_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Period fixedLegTenor_;
        BusinessDayConvention fixedLegConvention_;
        bool exogenousDiscount_;
        Handle<YieldTermStructure> discount_;
        // cache data to avoid swap recreation when the same fixing date
        // is used multiple time to forecast changing fixing
        mutable boost::shared_ptr<VanillaSwap> lastSwap_;
        mutable Date lastFixingDate_;
    };


    //! base class for overnight indexed swap indexes
    class OvernightIndexedSwapIndex : public SwapIndex {
      public:
        OvernightIndexedSwapIndex(
                  const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  Currency currency,
                  const boost::shared_ptr<OvernightIndex>& overnightIndex,
                  bool telescopicValueDates = false);
        //! \name Inspectors
        //@{
        boost::shared_ptr<OvernightIndex> overnightIndex() const;
        /*! \warning Relinking the term structure underlying the index will
                     not have effect on the returned swap.
        */
        boost::shared_ptr<OvernightIndexedSwap> underlyingSwap(
                                                const Date& fixingDate) const;
        //@}
      protected:
        boost::shared_ptr<OvernightIndex> overnightIndex_;
        bool telescopicValueDates_;
        // cache data to avoid swap recreation when the same fixing date
        // is used multiple time to forecast changing fixing
        mutable boost::shared_ptr<OvernightIndexedSwap> lastSwap_;
        mutable Date lastFixingDate_;
    };

    // inline definitions

    inline BusinessDayConvention SwapIndex::fixedLegConvention() const {
        return fixedLegConvention_;
    }

    inline bool SwapIndex::exogenousDiscount() const {
        return exogenousDiscount_;
    }

    inline boost::shared_ptr<OvernightIndex>
    OvernightIndexedSwapIndex::overnightIndex() const {
        return overnightIndex_;
    }

}


/*
 Copyright (C) 2006, 2009 Ferdinando Ametrano
 Copyright (C) 2006, 2007, 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/instruments/makevanillaswap.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

#include <sstream>

using boost::shared_ptr;

namespace QuantLib {

    inline SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         Currency currency,
                         const Calendar& fixingCalendar,
                         const Period& fixedLegTenor,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         const shared_ptr<IborIndex>& iborIndex)
    : InterestRateIndex(familyName, tenor, settlementDays,
                        currency, fixingCalendar, fixedLegDayCounter),
      tenor_(tenor), iborIndex_(iborIndex),
      fixedLegTenor_(fixedLegTenor),
      fixedLegConvention_(fixedLegConvention),
      exogenousDiscount_(false),
      discount_(Handle<YieldTermStructure>()) {
        registerWith(iborIndex_);
    }

    inline SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         Currency currency,
                         const Calendar& fixingCalendar,
                         const Period& fixedLegTenor,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         const shared_ptr<IborIndex>& iborIndex,
                         const Handle<YieldTermStructure>& discount)
    : InterestRateIndex(familyName, tenor, settlementDays,
                        currency, fixingCalendar, fixedLegDayCounter),
      tenor_(tenor), iborIndex_(iborIndex),
      fixedLegTenor_(fixedLegTenor),
      fixedLegConvention_(fixedLegConvention),
      exogenousDiscount_(true),
      discount_(discount) {
        registerWith(iborIndex_);
        registerWith(discount_);
    }

    inline Handle<YieldTermStructure> SwapIndex::forwardingTermStructure() const {
        return iborIndex_->forwardingTermStructure();
    }

    inline Handle<YieldTermStructure> SwapIndex::discountingTermStructure() const {
        return discount_;  // empty if not exogenous
    }

    inline Rate SwapIndex::forecastFixing(const Date& fixingDate) const {
        return underlyingSwap(fixingDate)->fairRate();
    }

    inline shared_ptr<VanillaSwap>
    SwapIndex::underlyingSwap(const Date& fixingDate) const {

        QL_REQUIRE(fixingDate!=Date(), "null fixing date");

        // caching mechanism
        if (lastFixingDate_!=fixingDate) {
            Rate fixedRate = 0.0;
            if (exogenousDiscount_)
                lastSwap_ = MakeVanillaSwap(tenor_, iborIndex_, fixedRate)
                    .withEffectiveDate(valueDate(fixingDate))
                    .withFixedLegCalendar(fixingCalendar())
                    .withFixedLegDayCount(dayCounter_)
                    .withFixedLegTenor(fixedLegTenor_)
                    .withFixedLegConvention(fixedLegConvention_)
                    .withFixedLegTerminationDateConvention(fixedLegConvention_)
                    .withDiscountingTermStructure(discount_);
            else
                lastSwap_ = MakeVanillaSwap(tenor_, iborIndex_, fixedRate)
                    .withEffectiveDate(valueDate(fixingDate))
                    .withFixedLegCalendar(fixingCalendar())
                    .withFixedLegDayCount(dayCounter_)
                    .withFixedLegTenor(fixedLegTenor_)
                    .withFixedLegConvention(fixedLegConvention_)
                    .withFixedLegTerminationDateConvention(fixedLegConvention_);
            lastFixingDate_ = fixingDate;
        }
        return lastSwap_;
    }

    inline Date SwapIndex::maturityDate(const Date& valueDate) const {
        Date fixDate = fixingDate(valueDate);
        return underlyingSwap(fixDate)->maturityDate();
    }

    inline shared_ptr<SwapIndex>
    SwapIndex::clone(const Handle<YieldTermStructure>& forwarding) const {

        if (exogenousDiscount_)
            return shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor(),
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex_->clone(forwarding),
                          discount_));
        else
            return shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor(),
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex_->clone(forwarding)));
    }

    inline shared_ptr<SwapIndex>
    SwapIndex::clone(const Handle<YieldTermStructure>& forwarding,
                     const Handle<YieldTermStructure>& discounting) const {
        return shared_ptr<SwapIndex>(new
             SwapIndex(familyName(),
                       tenor(),
                       fixingDays(),
                       currency(),
                       fixingCalendar(),
                       fixedLegTenor(),
                       fixedLegConvention(),
                       dayCounter(),
                       iborIndex_->clone(forwarding),
                       discounting));
    }

    inline shared_ptr<SwapIndex>
    SwapIndex::clone(const Period& tenor) const {

        if (exogenousDiscount_)
            return shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor,
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex(),
                          discountingTermStructure()));
        else
            return shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor,
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex()));

    }

    inline OvernightIndexedSwapIndex::OvernightIndexedSwapIndex(
                            const std::string& familyName,
                            const Period& tenor,
                            Natural settlementDays,
                            Currency currency,
                            const shared_ptr<OvernightIndex>& overnightIndex,
                            bool telescopicValueDates)
    : SwapIndex(familyName, tenor, settlementDays,
                currency, overnightIndex->fixingCalendar(),
                1*Years, ModifiedFollowing, overnightIndex->dayCounter(),
                overnightIndex),
      overnightIndex_(overnightIndex), telescopicValueDates_(telescopicValueDates) {}


    inline boost::shared_ptr<OvernightIndexedSwap>
    OvernightIndexedSwapIndex::underlyingSwap(const Date& fixingDate) const {

        QL_REQUIRE(fixingDate!=Date(), "null fixing date");

        // caching mechanism
        if (lastFixingDate_!=fixingDate) {
            Rate fixedRate = 0.0;
            lastSwap_ = MakeOIS(tenor_, overnightIndex_, fixedRate)
                .withEffectiveDate(valueDate(fixingDate))
                .withFixedLegDayCount(dayCounter_)
                .withTelescopicValueDates(telescopicValueDates_);
            lastFixingDate_ = fixingDate;
        }
        return lastSwap_;
    }

}


#endif
