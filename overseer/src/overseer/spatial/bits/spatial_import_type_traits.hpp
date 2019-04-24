// -*- C++ -*-
//
// Copyright Sylvain Bougerel 2009 - 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file COPYING or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 *  \file spatial_import_type_traits.hpp Contains the macro to pull certain
 *  "type_traits" dependancies into the spatial::import namespace.
 *
 */

#ifndef SPATIAL_IMPORT_TYPE_TRAITS
#define SPATIAL_IMPORT_TYPE_TRAITS

#ifdef SPATIAL_TYPE_TRAITS_NAMESPACE
#  undef SPATIAL_TYPE_TRAITS_NAMESPACE
#endif

#include <type_traits>
#define SPATIAL_TYPE_TRAITS_NAMESPACE std

namespace spatial
{
  namespace import
  {
    using SPATIAL_TYPE_TRAITS_NAMESPACE::is_arithmetic;
    using SPATIAL_TYPE_TRAITS_NAMESPACE::is_empty;
    using SPATIAL_TYPE_TRAITS_NAMESPACE::is_floating_point;
    using SPATIAL_TYPE_TRAITS_NAMESPACE::true_type;
    using SPATIAL_TYPE_TRAITS_NAMESPACE::false_type;
  }
}

#endif // SPATIAL_IMPORT_TYPE_TRAITS
