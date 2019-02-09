/*
 * types.hpp
 *
 *  Created on: Jan 20, 2019
 *      Author: iconmaster
 */

#ifndef SRC_TYPES_HPP_
#define SRC_TYPES_HPP_

#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

namespace iconus {
	template<typename T> using Vector = std::vector<T>;
	template<typename T> using Deque = std::deque<T>;
	template<typename T> using List = std::list<T>;
	template<typename T> using Set = std::unordered_set<T>;
	template<typename K, typename V> using Map = std::unordered_map<K, V>;
	
	using Mutex = std::recursive_mutex;
	using Lock = std::lock_guard<Mutex>;
}

#endif /* SRC_TYPES_HPP_ */
