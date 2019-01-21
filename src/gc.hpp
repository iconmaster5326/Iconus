/*
 * gc.hpp
 *
 *  Created on: Jan 20, 2019
 *      Author: iconmaster
 */

#ifndef SRC_GC_HPP_
#define SRC_GC_HPP_


#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <unordered_set>

namespace iconus {
	template<typename T> using Vector = std::vector<T, traceable_allocator<T>>;
	template<typename T> using Deque = std::deque<T, traceable_allocator<T>>;
	template<typename T> using List = std::list<T, traceable_allocator<T>>;
	template<typename T> using Set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, traceable_allocator<T>>;
	template<typename K, typename V> using Map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, traceable_allocator<std::pair<const K,V>>>;
	
	template<typename T, typename... Args> T* gcAlloc(Args... args) {
		traceable_allocator<T> allocator;
		T* gcMem = allocator.allocate(1);
	    return new(gcMem) T(args...);
	}
}

#endif /* SRC_GC_HPP_ */
