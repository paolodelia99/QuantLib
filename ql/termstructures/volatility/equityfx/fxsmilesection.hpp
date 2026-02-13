/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
 Copyright (C) 2026 Paolo D'Elia

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

/*! \file fxsmilesection.hpp
    \brief FX smile section assuming a strike/volatility space
*/

#ifndef quantlib_fx_smile_section_hpp
#define quantlib_fx_smile_section_hpp

#include <ql/types.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

//! FX SmileSection
class FxSmileSection {
public:
    FxSmileSection() : spot_(0.0), rd_(0.0), rf_(0.0), t_(0.0) {}
    FxSmileSection(Real spot, Real rd, Real rf, Time t) : spot_(spot), rd_(rd), rf_(rf), t_(t) {}
    virtual ~FxSmileSection(){};

    virtual Volatility volatility(Real strike) const = 0;

    DiscountFactor domesticDiscount() const { return std::exp(-rd_ * t_); }
    DiscountFactor foreignDiscount() const { return std::exp(-rf_ * t_); }

protected:
    Real spot_;
    Real rd_;
    Real rf_;
    Time t_;
};

}

#endif
