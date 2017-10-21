/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file knuthuniformrng.hpp
    \brief Knuth uniform random number generator
*/

#ifndef quantlib_knuth_uniform_rng_h
#define quantlib_knuth_uniform_rng_h

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    //! Uniform random number generator
    /*! Random number generator by Knuth.
        For more details see Knuth, Seminumerical Algorithms,
        3rd edition, Section 3.6.
        \note This is <b>not</b> Knuth's original implementation which
              is available at
              http://www-cs-faculty.stanford.edu/~knuth/programs.html,
              but rather a slightly modified version wrapped in a C++ class.
              Such modifications did not affect the code but only the data
              structures used, which were converted to their standard C++
              equivalents.
    */
    class KnuthUniformRng {
      public:
        typedef Sample<Real> sample_type;
        /*! if the given seed is 0, a random seed will be chosen
            based on clock() */
        explicit KnuthUniformRng(long seed = 0);
        /*! returns a sample with weight 1.0 containing a random number
          uniformly chosen from (0.0,1.0) */
        sample_type next() const;
      private:
        static const int KK, LL, TT, QUALITY;
        mutable std::vector<double> ranf_arr_buf;
        mutable size_t ranf_arr_ptr, ranf_arr_sentinel;
        mutable std::vector<double> ran_u;
        double mod_sum(double x, double y) const;
        bool is_odd(int s) const;
        void ranf_start(long seed);
        void ranf_array(std::vector<double>& aa, int n) const;
        double ranf_arr_cycle() const;
    };


    // inline definitions

    inline KnuthUniformRng::sample_type KnuthUniformRng::next() const {
        double result = (ranf_arr_ptr != ranf_arr_sentinel ?
                         ranf_arr_buf[ranf_arr_ptr++] :
                         ranf_arr_cycle());
        return sample_type(result,1.0);
    }

    inline double KnuthUniformRng::mod_sum(double x, double y) const {
        return (x+y)-int(x+y);
    }

    inline bool KnuthUniformRng::is_odd(int s) const {
        return (s&1) != 0;
    }

}





/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/math/randomnumbers/seedgenerator.hpp>

namespace QuantLib {

    const int KnuthUniformRng::KK = 100;
    const int KnuthUniformRng::LL = 37;
    const int KnuthUniformRng::TT = 70;
    const int KnuthUniformRng::QUALITY = 1009;

    KnuthUniformRng::KnuthUniformRng(long seed)
    : ranf_arr_buf(QUALITY), ran_u(QUALITY) {
        ranf_arr_ptr = ranf_arr_sentinel = ranf_arr_buf.size();
        ranf_start(seed != 0 ? seed : SeedGenerator::instance().get());
    }

    void KnuthUniformRng::ranf_start(long seed) {
        int t,s,j;
        std::vector<double> u(KK+KK-1),ul(KK+KK-1);
        double ulp=(1.0/(1L<<30))/(1L<<22);                // 2 to the -52
        double ss=2.0*ulp*((seed&0x3fffffff)+2);

        for (j=0;j<KK;j++) {
            u[j]=ss; ul[j]=0.0;                    // bootstrap the buffer
            ss+=ss; if (ss>=1.0) ss-=1.0-2*ulp; // cyclic shift of 51 bits
        }
        for (;j<KK+KK-1;j++) u[j]=ul[j]=0.0;
        u[1]+=ulp;ul[1]=ulp;            // make u[1] (and only u[1]) "odd"
        s=seed&0x3fffffff;
        t=TT-1; while (t) {
            for (j=KK-1;j>0;--j) ul[j+j]=ul[j],u[j+j]=u[j];    // "square"
            for (j=KK+KK-2;j>KK-LL;j-=2)
                ul[KK+KK-1-j]=0.0,u[KK+KK-1-j]=u[j]-ul[j];
            for (j=KK+KK-2;j>=KK;--j) if(ul[j]) {
                ul[j-(KK-LL)]=ulp-ul[j-(KK-LL)],
                    u[j-(KK-LL)]=mod_sum(u[j-(KK-LL)],u[j]);
                ul[j-KK]=ulp-ul[j-KK],u[j-KK]=mod_sum(u[j-KK],u[j]);
            }
            if (is_odd(s)) {                            // "multiply by z"
                for (j=KK;j>0;--j)  ul[j]=ul[j-1],u[j]=u[j-1];
                ul[0]=ul[KK],u[0]=u[KK];    // shift the buffer cyclically
                if (ul[KK]) ul[LL]=ulp-ul[LL],u[LL]=mod_sum(u[LL],u[KK]);
            }
            if (s) s>>=1; else t--;
        }
        for (j=0;j<LL;j++) ran_u[j+KK-LL]=u[j];
        for (;j<KK;j++) ran_u[j-LL]=u[j];
    }

    void KnuthUniformRng::ranf_array(std::vector<double>& aa,
                                     int n) const {
        int i,j;
        for (j=0;j<KK;j++) aa[j]=ran_u[j];
        for (;j<n;j++) aa[j]=mod_sum(aa[j-KK],aa[j-LL]);
        for (i=0;i<LL;i++,j++) ran_u[i]=mod_sum(aa[j-KK],aa[j-LL]);
        for (;i<KK;i++,j++) ran_u[i]=mod_sum(aa[j-KK],ran_u[i-LL]);
    }

    double KnuthUniformRng::ranf_arr_cycle() const {
        ranf_array(ranf_arr_buf,QUALITY);
        ranf_arr_ptr = 1;
        ranf_arr_sentinel = 100;
        return ranf_arr_buf[0];
    }

}

#endif