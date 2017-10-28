/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Jose Aparicio

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

#ifndef quantlib_cva_swap_engine_hpp
#define quantlib_cva_swap_engine_hpp

#include <ql/handle.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class Quote;

  /*! Bilateral (CVA and DVA) default adjusted vanilla swap pricing
    engine. Collateral is not considered. No wrong way risk is 
    considered (rates and counterparty default are uncorrelated).
    Based on:
    Sorensen,  E.H.  and  Bollier,  T.F.,  Pricing  swap  default 
    risk. Financial Analysts Journal, 1994, 50, 23–33
    Also see sect. II-5 in: Risk Neutral Pricing of Counterparty Risk
    D. Brigo, M. Masetti, 2004
    or in sections 3 and 4 of "A Formula for Interest Rate Swaps 
      Valuation under Counterparty Risk in presence of Netting Agreements"
    D. Brigo and M. Masetti; May 4, 2005

    to do: Compute fair rate through iteration instead of the 
    current approximation .
    to do: write Issuer based constructors (event type)
    to do: Check consistency between option engine discount and the one given
   */
  class CounterpartyAdjSwapEngine : public VanillaSwap::engine {
    public:
      //! \name Constructors
      //@{
      //! 
      /*! Creates the engine from an arbitrary swaption engine.
        If the investor default model is not given a default 
        free one is assumed.
        @param discountCurve Used in pricing.
        @param swaptionEngine Determines the volatility and thus the 
        exposure model.
        @param ctptyDTS Counterparty default curve.
        @param ctptyRecoveryRate Counterparty recovey rate.
        @param invstDTS Investor (swap holder) default curve.
        @param invstRecoveryRate Investor recovery rate.
       */
      CounterpartyAdjSwapEngine(
          const Handle<YieldTermStructure>& discountCurve,
          const Handle<PricingEngine>& swaptionEngine,
          const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
          Real ctptyRecoveryRate,
          const Handle<DefaultProbabilityTermStructure>& invstDTS =
              Handle<DefaultProbabilityTermStructure>(),
          Real invstRecoveryRate = 0.999);
      /*! Creates an engine with a black volatility model for the 
        exposure.
        If the investor default model is not given a default 
        free one is assumed.
        @param discountCurve Used in pricing.
        @param blackVol Black volatility used in the exposure model.
        @param ctptyDTS Counterparty default curve.
        @param ctptyRecoveryRate Counterparty recovey rate.
        @param invstDTS Investor (swap holder) default curve.
        @param invstRecoveryRate Investor recovery rate.
       */
      CounterpartyAdjSwapEngine(
          const Handle<YieldTermStructure>& discountCurve,
          const Volatility blackVol,
          const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
          Real ctptyRecoveryRate,
          const Handle<DefaultProbabilityTermStructure>& invstDTS =
              Handle<DefaultProbabilityTermStructure>(),
          Real invstRecoveryRate = 0.999);
      /*! Creates an engine with a black volatility model for the 
        exposure. The volatility is given as a quote.
        If the investor default model is not given a default 
        free one is assumed.
        @param discountCurve Used in pricing.
        @param blackVol Black volatility used in the exposure model.
        @param ctptyDTS Counterparty default curve.
        @param ctptyRecoveryRate Counterparty recovey rate.
        @param invstDTS Investor (swap holder) default curve.
        @param invstRecoveryRate Investor recovery rate.
      */
      CounterpartyAdjSwapEngine(
          const Handle<YieldTermStructure>& discountCurve,
          const Handle<Quote>& blackVol,
          const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
          Real ctptyRecoveryRate,
          const Handle<DefaultProbabilityTermStructure>& invstDTS =
              Handle<DefaultProbabilityTermStructure>(),
          Real invstRecoveryRate = 0.999);
      //@}
      void calculate() const;
    private:
      Handle<PricingEngine> baseSwapEngine_;
      Handle<PricingEngine> swaptionletEngine_;
      Handle<YieldTermStructure> discountCurve_;
      Handle<DefaultProbabilityTermStructure> defaultTS_;	  
      Real ctptyRecoveryRate_;
      Handle<DefaultProbabilityTermStructure> invstDTS_;	  
      Real invstRecoveryRate_;
  };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Jose Aparicio

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/exercise.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <boost/make_shared.hpp>

namespace QuantLib {
  
  inline CounterpartyAdjSwapEngine::CounterpartyAdjSwapEngine(
      const Handle<YieldTermStructure>& discountCurve,
      const Handle<PricingEngine>& swaptionEngine,
      const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
      Real ctptyRecoveryRate,
      const Handle<DefaultProbabilityTermStructure>& invstDTS,
      Real invstRecoveryRate)
  : baseSwapEngine_(Handle<PricingEngine>(
      boost::make_shared<DiscountingSwapEngine>(discountCurve))),
    swaptionletEngine_(swaptionEngine),
    discountCurve_(discountCurve),
    defaultTS_(ctptyDTS), 
    ctptyRecoveryRate_(ctptyRecoveryRate),
    invstDTS_(invstDTS.empty() ? Handle<DefaultProbabilityTermStructure>(
        boost::make_shared<FlatHazardRate>(0, ctptyDTS->calendar(), 1.e-12, 
        ctptyDTS->dayCounter()) ) : invstDTS ),
    invstRecoveryRate_(invstRecoveryRate)
  {
      registerWith(discountCurve);
      registerWith(ctptyDTS);
      registerWith(invstDTS_);
      registerWith(swaptionEngine);
  }

    inline CounterpartyAdjSwapEngine::CounterpartyAdjSwapEngine(
        const Handle<YieldTermStructure>& discountCurve,
        const Volatility blackVol,
        const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
        Real ctptyRecoveryRate,
        const Handle<DefaultProbabilityTermStructure>& invstDTS,
        Real invstRecoveryRate)
  : baseSwapEngine_(Handle<PricingEngine>(
      boost::make_shared<DiscountingSwapEngine>(discountCurve))),
    swaptionletEngine_(Handle<PricingEngine>(
      boost::make_shared<BlackSwaptionEngine>(discountCurve,
        blackVol))),
    discountCurve_(discountCurve),
    defaultTS_(ctptyDTS), 
    ctptyRecoveryRate_(ctptyRecoveryRate),
    invstDTS_(invstDTS.empty() ? Handle<DefaultProbabilityTermStructure>(
        boost::make_shared<FlatHazardRate>(0, ctptyDTS->calendar(), 1.e-12, 
        ctptyDTS->dayCounter()) ) : invstDTS ),
    invstRecoveryRate_(invstRecoveryRate)
  {
      registerWith(discountCurve);
      registerWith(ctptyDTS);
      registerWith(invstDTS_);
  }

  inline CounterpartyAdjSwapEngine::CounterpartyAdjSwapEngine(
        const Handle<YieldTermStructure>& discountCurve,
        const Handle<Quote>& blackVol,
        const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
        Real ctptyRecoveryRate,
        const Handle<DefaultProbabilityTermStructure>& invstDTS,
        Real invstRecoveryRate)
  : baseSwapEngine_(Handle<PricingEngine>(
      boost::make_shared<DiscountingSwapEngine>(discountCurve))),
    swaptionletEngine_(Handle<PricingEngine>(
      boost::make_shared<BlackSwaptionEngine>(discountCurve,
        blackVol))),
    discountCurve_(discountCurve),
    defaultTS_(ctptyDTS), 
    ctptyRecoveryRate_(ctptyRecoveryRate),
    invstDTS_(invstDTS.empty() ? Handle<DefaultProbabilityTermStructure>(
        boost::make_shared<FlatHazardRate>(0, ctptyDTS->calendar(), 1.e-12, 
        ctptyDTS->dayCounter()) ) : invstDTS ),
    invstRecoveryRate_(invstRecoveryRate)
  {
      registerWith(discountCurve);
      registerWith(ctptyDTS);
      registerWith(invstDTS_);
      registerWith(blackVol);
  }

  inline void CounterpartyAdjSwapEngine::calculate() const {
      /* both DTS, YTS ref dates and pricing date consistency 
         checks? settlement... */
    QL_REQUIRE(!discountCurve_.empty(),
                 "no discount term structure set");
    QL_REQUIRE(!defaultTS_.empty(),
                 "no ctpty default term structure set");
    QL_REQUIRE(!swaptionletEngine_.empty(),
                 "no swap option engine set");

    Date priceDate = defaultTS_->referenceDate();

    Real cumOptVal = 0., 
        cumPutVal = 0.;
    // Vanilla swap so 0 leg is floater

    std::vector<Date>::const_iterator nextFD = 
      arguments_.fixedPayDates.begin();
    Date swapletStart = priceDate;
    while (*nextFD < priceDate) ++nextFD;

    // Compute fair spread for strike value:
    // copy args into the non risky engine
    Swap::arguments * noCVAArgs = dynamic_cast<Swap::arguments*>(
      baseSwapEngine_->getArguments());
    QL_REQUIRE(noCVAArgs != 0, "wrong argument type");

    noCVAArgs->legs = this->arguments_.legs;
    noCVAArgs->payer = this->arguments_.payer;

    baseSwapEngine_->calculate();

    boost::shared_ptr<FixedRateCoupon> coupon = boost::dynamic_pointer_cast<FixedRateCoupon>(arguments_.legs[0][0]);
    QL_REQUIRE(coupon,"dynamic cast of fixed leg coupon failed.");
    Rate baseSwapRate = coupon->rate();

    const Swap::results * vSResults =  
        dynamic_cast<const Swap::results *>(baseSwapEngine_->getResults());
    QL_REQUIRE(vSResults != 0, "wrong result type");

    Rate baseSwapFairRate = -baseSwapRate * vSResults->legNPV[1] / 
        vSResults->legNPV[0];
    Real baseSwapNPV = vSResults->value;

    VanillaSwap::Type reversedType = arguments_.type == VanillaSwap::Payer ? 
        VanillaSwap::Receiver : VanillaSwap::Payer;

    // Swaplet options summatory:
    while(nextFD != arguments_.fixedPayDates.end()) {
      // iFD coupon not fixed, create swaptionlet:
      boost::shared_ptr<FloatingRateCoupon> floatCoupon = boost::dynamic_pointer_cast<FloatingRateCoupon>(arguments_.legs[1][0]);
      QL_REQUIRE(floatCoupon,"dynamic cast of floating leg coupon failed.");
      boost::shared_ptr<IborIndex> swapIndex = boost::dynamic_pointer_cast<IborIndex>(floatCoupon->index());
      QL_REQUIRE(swapIndex,"dynamic cast of floating leg index failed.");

      // Alternatively one could cap this period to, say, 1M 
      // Period swapPeriod = boost::dynamic_pointer_cast<FloatingRateCoupon>(
      //   arguments_.legs[1][0])->index()->tenor();

      Period baseSwapsTenor(arguments_.fixedPayDates.back().serialNumber() 
	    - swapletStart.serialNumber(), Days);
      boost::shared_ptr<VanillaSwap> swaplet = MakeVanillaSwap(
        baseSwapsTenor,
        swapIndex, 
        baseSwapFairRate // strike
        )
	    .withType(arguments_.type)
	    .withNominal(arguments_.nominal)
          ////////	    .withSettlementDays(2)
        .withEffectiveDate(swapletStart)
        .withTerminationDate(arguments_.fixedPayDates.back());
      boost::shared_ptr<VanillaSwap> revSwaplet = MakeVanillaSwap(
        baseSwapsTenor,
        swapIndex, 
        baseSwapFairRate // strike
        )
	    .withType(reversedType)
	    .withNominal(arguments_.nominal)
          /////////	    .withSettlementDays(2)
        .withEffectiveDate(swapletStart)
        .withTerminationDate(arguments_.fixedPayDates.back());

      Swaption swaptionlet(swaplet, 
        boost::make_shared<EuropeanExercise>(swapletStart));
      Swaption putSwaplet(revSwaplet, 
        boost::make_shared<EuropeanExercise>(swapletStart));
      swaptionlet.setPricingEngine(swaptionletEngine_.currentLink());
      putSwaplet.setPricingEngine(swaptionletEngine_.currentLink());

      // atm underlying swap means that the value of put = value
      // call so this double pricing is not needed
      cumOptVal += swaptionlet.NPV() * defaultTS_->defaultProbability(
          swapletStart, *nextFD);
      cumPutVal += putSwaplet.NPV()  * invstDTS_->defaultProbability(
	      swapletStart, *nextFD);

      swapletStart = *nextFD;
      ++nextFD;
    }
  
    results_.value = baseSwapNPV - (1.-ctptyRecoveryRate_) * cumOptVal
        + (1.-invstRecoveryRate_) * cumPutVal;

    results_.fairRate =  -baseSwapRate * (vSResults->legNPV[1] 
        - (1.-ctptyRecoveryRate_) * cumOptVal + 
          (1.-invstRecoveryRate_) * cumPutVal )
      / vSResults->legNPV[0];

  }


}

#endif
