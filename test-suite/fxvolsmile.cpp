/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fxvolsmile.cpp
    \brief fx vol smile
*/

#include "toplevelfixture.hpp"
#include <boost/make_shared.hpp>
#include <boost/test/unit_test.hpp>
#include <ql/math/matrix.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <ql/termstructures/volatility/equityfx/fxblackvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/fxvannavolgasmilesection.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
    struct VolData {
        const char* tenor;
        Volatility atm;
        Volatility rr;
        Volatility bf;
        Time time;
        Real df_d;
        Real df_f;
    };
}

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FxVolSmileTest)

BOOST_AUTO_TEST_CASE(testVannaVolgaFxSmileSection) {

    BOOST_TEST_MESSAGE("Testing fx vanna volga smile");

    SavedSettings backup;

    // test numbers from Castagna & Mercurio (2006)
    // http://papers.ssrn.com/sol3/papers.cfm?abstract_id=873788
    // page 5
    Date today = Date(1, July, 2005);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();
    Calendar cal = TARGET();
    Time t = dc.yearFraction(today, cal.advance(today, Period(3, Months)));//94 / (double)365;
    Real S0 = 1.205;
    Volatility atmVol = 0.0905;
    Volatility rrVol = -0.005;
    Volatility bfVol = 0.0013;
    // page 11
    DiscountFactor dfUsd = 0.9902752;
    DiscountFactor dfEur = 0.9945049;

    // Rates
    Real rd = -std::log(dfUsd) / t;
    Real rf = -std::log(dfEur) / t;

    VannaVolgaSmileSection vvss(S0, rd, rf, t, atmVol, rrVol, bfVol);

    // Check the Strike and Vol values from the paper
    Real tolerance = 0.0001; // 4 decimal places
    if (fabs(vvss.k_atm() - 1.2114) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to calculte ATM strike, got " << vvss.k_atm());
    if (fabs(vvss.k_p() - 1.1733) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to calculate 25P strike, got " << vvss.k_p());
    if (fabs(vvss.k_c() - 1.2487) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to calculate 25C strike, got " << vvss.k_c());
    if (fabs(vvss.vol_atm() - 0.0905) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to calculate ATM vol, got " << vvss.vol_atm());
    if (fabs(vvss.vol_p() - 0.0943) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to calculate 25P vol, got " << vvss.vol_p());
    if (fabs(vvss.vol_c() - 0.0893) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to calculate 25C vol, got " << vvss.vol_c());

    // Now check that our smile returns these
    if (fabs(vvss.volatility(vvss.k_atm()) - vvss.vol_atm()) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to recover ATM vol, got " << vvss.volatility(vvss.k_atm()));
    if (fabs(vvss.volatility(vvss.k_p()) - vvss.vol_p()) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to recover 25P vol, got " << vvss.volatility(vvss.k_p()));
    if (fabs(vvss.volatility(vvss.k_c()) - vvss.vol_c()) > tolerance)
        BOOST_FAIL("VannaVolgaSmileSection failed to recover 25C vol, got " << vvss.volatility(vvss.k_c()));

    // To graph the smile, uncomment this code
    /*
    cout << "strike,vol" << endl;
    //for (Real k = 1.1; k < 1.35; k += 0.002) // normal (as per paper)
    //for (Real k = 0.9; k < 1.5; k += 0.01) // large
    for (Real k = 0.1; k < 3; k += 0.05) // extreme
        cout << k << "," << vvss.volatility(k) << endl;
    */
}

BOOST_AUTO_TEST_CASE(testVannaVolgaFxVolSurface) {

    BOOST_TEST_MESSAGE("Testing fx vanna volga surface");

    SavedSettings backup;

    // Data from
    // "Consistent pricing and hedging of an FX options book" (2005)
    // L. Bisesti, A. Castagna and F. Mercurio
    // http://www.fabiomercurio.it/fxbook.pdf
    Date asof(12, Feb, 2004);
    Settings::instance().evaluationDate() = asof;

    Handle<Quote> fxSpot = makeQuoteHandle(1.2832);

    // vols are % here
    // tenor, atm, rr, bf, T, p_d, p_f
    VolData volData[] = { { "1W", 11.75, 0.50, 0.190, 0.0192, 0.999804, 0.999606 },
                          { "2W", 11.60, 0.50, 0.190, 0.0384, 0.999595, 0.999208 },
                          { "1M", 11.50, 0.60, 0.190, 0.0877, 0.999044, 0.998179 },
                          { "2M", 11.25, 0.60, 0.210, 0.1726, 0.998083, 0.996404 },
                          { "3M", 11.00, 0.60, 0.220, 0.2493, 0.997187, 0.994803 },
                          { "6M", 10.87, 0.65, 0.235, 0.5014, 0.993959, 0.989548 },
                          { "9M", 10.83, 0.69, 0.235, 0.7589, 0.990101, 0.984040 },
                          { "1Y", 10.80, 0.70, 0.240, 1.0110, 0.985469, 0.978479 },
                          { "2Y", 10.70, 0.65, 0.255, 2.0110, 0.960102, 0.951092 } };

    // Assume act/act
    DayCounter dc = ActualActual(ActualActual::ISDA);
    Calendar cal = TARGET();

    // set up vectors
    Size len = sizeof(volData) / sizeof(volData[0]);
    std::vector<Date> dates(len);
    std::vector<Volatility> atm(len);
    std::vector<Volatility> rr(len);
    std::vector<Volatility> bf(len);
    // For DiscountCurve we need the T=0 points.
    std::vector<Date> discountDates(len + 1);
    std::vector<DiscountFactor> dfDom(len + 1);
    std::vector<DiscountFactor> dfFor(len + 1);
    discountDates[0] = asof;
    dfDom[0] = 1.0;
    dfFor[0] = 1.0;

    for (Size i = 0; i < sizeof(volData) / sizeof(volData[0]); i++) {
        dates[i] = asof + PeriodParser::parse(volData[i].tenor);
        // check time == volData[i].time
        /*
        if (fabs(dc.yearFraction(asof, dates[i]) - volData[i].time) > 0.001)
            BOOST_FAIL("Did not match vol data time (" << volData[i].time <<
                       ") with aosf " << asof << " and maturity " << dates[i] <<
                       " got year fraction of " << dc.yearFraction(asof, dates[i]));
         */

        atm[i] = volData[i].atm / 100;
        rr[i] = volData[i].rr / 100;
        bf[i] = volData[i].bf / 100;

        discountDates[i + 1] = dates[i];
        dfDom[i + 1] = volData[i].df_d;
        dfFor[i + 1] = volData[i].df_f;
    }

    // Now build discount curves
    Handle<YieldTermStructure> domYTS(
        QuantLib::ext::shared_ptr<YieldTermStructure>(new DiscountCurve(discountDates, dfDom, dc)));
    Handle<YieldTermStructure> forYTS(
        QuantLib::ext::shared_ptr<YieldTermStructure>(new DiscountCurve(discountDates, dfFor, dc)));

    // build surface
    FxBlackVannaVolgaVolatilitySurface volSurface(asof, dates, atm, rr, bf, dc, cal, fxSpot, domYTS, forYTS);

    // 1.55,1.75,0.121507
    Real vol = volSurface.blackVol(1.75, 1.55);
    Real expected = 0.121507;
    if (fabs(vol - expected) > 0.00001)
        BOOST_FAIL("Failed to get expected vol from surface " << vol);
    /*
    cout << "strike,time,vol" << endl;
    for (Real k = 1.0; k < 1.6; k += 0.01) // extreme
        for (Time tt = 0.1; tt < 2; tt+= 0.05)
            cout << k << "," << tt << "," << volSurface.blackVol(tt, k) << endl;
     */

    // Test if Null<Real>() or 0 strike returns atm vol
    for (Size i = 0; i < len; i++) {
        Real vol = volSurface.blackVol(dates[i], Null<Real>());
        Real vol2 = volSurface.blackVol(dates[i], 0);
        if (fabs(vol - atm[i]) > 0.00001)
            BOOST_FAIL("Failed to get expected atm vol from surface: " << vol);
        if (fabs(vol2 - atm[i]) > 0.00001)
            BOOST_FAIL("Failed to get expected atm vol from surface: " << vol);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
