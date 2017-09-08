// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#pragma once

#include <aliceVision/config.hpp>

#if ALICEVISION_IS_DEFINED(ALICEVISION_HAVE_BOOST)
#include <boost/container/flat_set.hpp>
#endif

namespace stl
{
#if ALICEVISION_IS_DEFINED(ALICEVISION_HAVE_BOOST)
  template <class Key
         ,class Compare  = std::less<Key>
         ,class Allocator = std::allocator<Key> >
  using flat_set = boost::container::flat_set<Key, Compare, Allocator>;
#else
  // Fallback to non-flat implementation
  template <class Key
         ,class Compare  = std::less<Key>
         ,class Allocator = std::allocator<Key> >
  using flat_set = std::set<Key, Compare, Allocator>;
#endif
}
