#pragma once
#ifndef IKUTILS_HPP
#define IKUTILS_HPP

#include <functional>

namespace ikE {

	//needs explanation
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};









}  // namespace lve
#endif