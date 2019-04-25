// -*- C++ -*-
//
// Copyright Sylvain Bougerel 2009 - 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file COPYING or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 *  \file spatial_import_tuple.hpp Contains the macro to pull certain
 *  "tuple" dependancies into the spatial::import namespace.
 *
 */

#ifndef SPATIAL_IMPORT_TUPLE
#define SPATIAL_IMPORT_TUPLE

#ifdef SPATIAL_TUPLE_NAMESPACE
#  undef SPATIAL_TUPLE_NAMESPACE
#endif

#include <tuple>
#define SPATIAL_TUPLE_NAMESPACE std

namespace spatial
{
  namespace import
  {
    using SPATIAL_TUPLE_NAMESPACE::tuple;
    using SPATIAL_TUPLE_NAMESPACE::tie;
    using SPATIAL_TUPLE_NAMESPACE::get;
    using SPATIAL_TUPLE_NAMESPACE::make_tuple;
  }
}

#endif // SPATIAL_IMPORT_TUPLE
