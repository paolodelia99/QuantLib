/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
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
#include <ql/termstructures/volatility/equityfx/fxvannavolgasmilesection.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;


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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
