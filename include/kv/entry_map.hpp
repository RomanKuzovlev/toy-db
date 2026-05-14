#pragma once

#ifdef KV_USE_CUSTOM_HASH_MAP
#include "kv/hash_map.hpp"
#else
#include "kv/std_hash_map.hpp"
#endif

namespace kv
{

#ifdef KV_USE_CUSTOM_HASH_MAP
using EntryMap = HashMap;
#else
using EntryMap = StdHashMap;
#endif

} // namespace kv
