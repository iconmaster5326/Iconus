/*
 * base64.hpp
 *
 *  Created on: Jan 16, 2019
 *      Author: iconmaster
 */

#ifndef SRC_BASE64_HPP_
#define SRC_BASE64_HPP_

#include <string>
#include <cstring>
#include <iostream>

namespace iconus {
	class Base64Buffer {
	public:
		const char* b;
		std::size_t n;
		
		template<typename T> static Base64Buffer fromStream(T& stream) {
			size_t n = 0;
			size_t maxN = 1024;
			char* b = new char[maxN];
			char c;
			
			while (stream) {
				if (n == maxN) {
					char* newB = new char[maxN*2];
					memcpy(newB, b, maxN);
					
					maxN *= 2;
					
					delete[] b;
					b = newB;
				}
				
				stream.get(c);
				b[n] = c;
				n++;
			}
			
			return Base64Buffer{b,n};
		}
	};
	
	std::string base64encode(const Base64Buffer& buf);
	
	inline std::string base64encode(const char* b, std::size_t n) {
		const Base64Buffer buf{b,n};
		return base64encode(buf);
	}
	
	inline std::string base64encode(const std::string& s) {
		const Base64Buffer buf{s.c_str(),s.size()};
		return base64encode(buf);
	}
	
	// bool base64decode(const std::string& val, Base64Buffer& buf);
}

#endif /* SRC_BASE64_HPP_ */
