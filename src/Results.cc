/**
 * Results container Interface
 *
 * Created by Samvel Khalatyan on Mar 13, 2011
 * Copyright 2011, All rights reserved
 */

#include "Results.h"

Results::Results(const Type &type):
    _type(type)
{
}

Results::~Results()
{
}

Results::Type Results::type() const
{
    return _type;
}
