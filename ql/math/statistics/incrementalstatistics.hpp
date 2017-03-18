/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2015 Peter Caspers

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

/*! \file incrementalstatistics.hpp
    \brief statistics tool based on incremental accumulation
           in the meantime, this is just a wrapper to the boost
           accumulator library, kept for backward compatibility
*/

#ifndef quantlib_incremental_statistics_hpp
#define quantlib_incremental_statistics_hpp

#include <ql/utilities/null.hpp>
#include <ql/errors.hpp>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-extensions"
#endif
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/weighted_mean.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/accumulators/statistics/weighted_skewness.hpp>
#include <boost/accumulators/statistics/weighted_kurtosis.hpp>
#include <boost/accumulators/statistics/weighted_moment.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <iomanip>

namespace QuantLib {

    //! Statistics tool based on incremental accumulation
    /*! It can accumulate a set of data and return statistics (e.g: mean,
        variance, skewness, kurtosis, error estimation, etc.).
        This class is a wrapper to the boost accumulator library.
    */

    class IncrementalStatistics {
      public:
        typedef Real value_type;
        IncrementalStatistics();
        //! \name Inspectors
        //@{
        //! number of samples collected
        Size samples() const;

        //! sum of data weights
        Real weightSum() const;

        /*! returns the mean, defined as
            \f[ \langle x \rangle = \frac{\sum w_i x_i}{\sum w_i}. \f]
        */
        Real mean() const;

        /*! returns the variance, defined as
            \f[ \frac{N}{N-1} \left\langle \left(
                x-\langle x \rangle \right)^2 \right\rangle. \f]
        */
        Real variance() const;

        /*! returns the standard deviation \f$ \sigma \f$, defined as the
            square root of the variance.
        */
        Real standardDeviation() const;

        /*! returns the error estimate \f$ \epsilon \f$, defined as the
            square root of the ratio of the variance to the number of
            samples.
        */
        Real errorEstimate() const;

        /*! returns the skewness, defined as
            \f[ \frac{N^2}{(N-1)(N-2)} \frac{\left\langle \left(
                x-\langle x \rangle \right)^3 \right\rangle}{\sigma^3}. \f]
            The above evaluates to 0 for a Gaussian distribution.
        */
        Real skewness() const;

        /*! returns the excess kurtosis, defined as
            \f[ \frac{N^2(N+1)}{(N-1)(N-2)(N-3)}
                \frac{\left\langle \left(x-\langle x \rangle \right)^4
                \right\rangle}{\sigma^4} - \frac{3(N-1)^2}{(N-2)(N-3)}. \f]
            The above evaluates to 0 for a Gaussian distribution.
        */
        Real kurtosis() const;

        /*! returns the minimum sample value */
        Real min() const;

        /*! returns the maximum sample value */
        Real max() const;

        //! number of negative samples collected
        Size downsideSamples() const;

        //! sum of data weights for negative samples
        Real downsideWeightSum() const;

        /*! returns the downside variance, defined as
            \f[ \frac{N}{N-1} \times \frac{ \sum_{i=1}^{N}
                \theta \times x_i^{2}}{ \sum_{i=1}^{N} w_i} \f],
            where \f$ \theta \f$ = 0 if x > 0 and
            \f$ \theta \f$ =1 if x <0
        */
        Real downsideVariance() const;

        /*! returns the downside deviation, defined as the
            square root of the downside variance.
        */
        Real downsideDeviation() const;

        //@}

        //! \name Modifiers
        //@{
        //! adds a datum to the set, possibly with a weight
        /*! \pre weight must be positive or null */
        void add(Real value, Real weight = 1.0);
        //! adds a sequence of data to the set, with default weight
        template <class DataIterator>
        void addSequence(DataIterator begin, DataIterator end) {
            for (;begin!=end;++begin)
                add(*begin);
        }
        //! adds a sequence of data to the set, each with its weight
        /*! \pre weights must be positive or null */
        template <class DataIterator, class WeightIterator>
        void addSequence(DataIterator begin, DataIterator end,
                         WeightIterator wbegin) {
            for (;begin!=end;++begin,++wbegin)
                add(*begin, *wbegin);
        }
        //! resets the data to a null set
        void reset();
        //@}
     private:
       typedef boost::accumulators::accumulator_set<
           Real,
           boost::accumulators::stats<
               boost::accumulators::tag::count, boost::accumulators::tag::min,
               boost::accumulators::tag::max,
               boost::accumulators::tag::weighted_mean,
               boost::accumulators::tag::weighted_variance,
               boost::accumulators::tag::weighted_skewness,
               boost::accumulators::tag::weighted_kurtosis,
               boost::accumulators::tag::sum_of_weights>,
           Real> accumulator_set;
        accumulator_set acc_;
        typedef boost::accumulators::accumulator_set<
            Real, boost::accumulators::stats<
                      boost::accumulators::tag::count,
                      boost::accumulators::tag::weighted_moment<2>,
                      boost::accumulators::tag::sum_of_weights>,
            Real> downside_accumulator_set;
        downside_accumulator_set downsideAcc_;
    };

    // implementation

    inline IncrementalStatistics::IncrementalStatistics() {
        reset();
    }

    inline Size IncrementalStatistics::samples() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::count>(acc_);
    }

    inline Real IncrementalStatistics::weightSum() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::sum_of_weights>(acc_);
    }

    inline Real IncrementalStatistics::mean() const {
        QL_REQUIRE(weightSum() > 0.0, "sampleWeight_= 0, unsufficient");
        return boost::accumulators::extract_result<
            boost::accumulators::tag::weighted_mean>(acc_);
    }

    inline Real IncrementalStatistics::variance() const {
        QL_REQUIRE(weightSum() > 0.0, "sampleWeight_= 0, unsufficient");
        QL_REQUIRE(samples() > 1, "sample number <= 1, unsufficient");
        Real n = static_cast<Real>(samples());
        return n / (n - 1.0) *
               boost::accumulators::extract_result<
                   boost::accumulators::tag::weighted_variance>(acc_);
    }

    inline Real IncrementalStatistics::standardDeviation() const {
        return std::sqrt(variance());
    }

    inline Real IncrementalStatistics::errorEstimate() const {
        return std::sqrt(variance() / (samples()));
    }

    inline Real IncrementalStatistics::skewness() const {
        QL_REQUIRE(samples() > 2, "sample number <= 2, unsufficient");
        Real n = static_cast<Real>(samples());
        Real r1 = n / (n - 2.0);
        Real r2 = (n - 1.0) / (n - 2.0);
        return std::sqrt(r1 * r2) * 
               boost::accumulators::extract_result<
                   boost::accumulators::tag::weighted_skewness>(acc_);
    }

    inline Real IncrementalStatistics::kurtosis() const {
        QL_REQUIRE(samples() > 3,
                   "sample number <= 3, unsufficient");
        boost::accumulators::extract_result<
            boost::accumulators::tag::weighted_kurtosis>(acc_);
        Real n = static_cast<Real>(samples());
        Real r1 = (n - 1.0) / (n - 2.0);
        Real r2 = (n + 1.0) / (n - 3.0);
        Real r3 = (n - 1.0) / (n - 3.0);
        return ((3.0 + boost::accumulators::extract_result<
                           boost::accumulators::tag::weighted_kurtosis>(acc_)) *
                    r2 -
                3.0 * r3) *
               r1;
    }

    inline Real IncrementalStatistics::min() const {
        QL_REQUIRE(samples() > 0, "empty sample set");
        return boost::accumulators::extract_result<
            boost::accumulators::tag::min>(acc_);
    }

    inline Real IncrementalStatistics::max() const {
        QL_REQUIRE(samples() > 0, "empty sample set");
        return boost::accumulators::extract_result<
            boost::accumulators::tag::max>(acc_);
    }

    inline Size IncrementalStatistics::downsideSamples() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::count>(downsideAcc_);
    }

    inline Real IncrementalStatistics::downsideWeightSum() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::sum_of_weights>(downsideAcc_);
    }

    inline Real IncrementalStatistics::downsideVariance() const {
        QL_REQUIRE(downsideWeightSum() > 0.0, "sampleWeight_= 0, unsufficient");
        QL_REQUIRE(downsideSamples() > 1, "sample number <= 1, unsufficient");
        Real n = static_cast<Real>(downsideSamples());
        Real r1 = n / (n - 1.0);
        return r1 *
               boost::accumulators::extract_result<
                   boost::accumulators::tag::moment<2> >(downsideAcc_);
    }

    inline Real IncrementalStatistics::downsideDeviation() const {
        return std::sqrt(downsideVariance());
    }

    inline void IncrementalStatistics::add(Real value, Real valueWeight) {
        QL_REQUIRE(valueWeight >= 0.0, "negative weight (" << valueWeight
                                                           << ") not allowed");
        acc_(value, boost::accumulators::weight = valueWeight);
        if(value < 0.0)
            downsideAcc_(value, boost::accumulators::weight = valueWeight);
    }

    inline void IncrementalStatistics::reset() {
        acc_ = accumulator_set();
        downsideAcc_ = downside_accumulator_set();
    }


}


#endif
