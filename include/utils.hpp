#pragma once

#include "Eigen/Eigen"
#include "array"
#include "component.hpp"
#include "coords.hpp"
#include "vec2.hpp"
#include <variant>

namespace Utils {
	Coords screenToGridRounded(const squi::vec2 &screen, const squi::vec2 &offset, const squi::vec2 &boardPos);
	Coords screenToGridFloored(const squi::vec2 &screen, const squi::vec2 &offset, const squi::vec2 &boardPos);
	Coords componentSizeWithRotation(const Component &component, uint32_t rotation);
	struct RotatedElementData {
		squi::vec2 newTopLeft{};
		squi::vec2 newTopRight{};
		squi::vec2 newBottomRight{};
		squi::vec2 newBottomLeft{};
		std::vector<Coords> newNodes{};
	};
	RotatedElementData rotateElement(uint32_t rotation, const Component &comp);
	std::tuple<Eigen::MatrixXf, std::vector<int64_t>> calculateNonzeroPivots(Eigen::MatrixXf &input);

	template<class T, class TupleType, size_t... I>
	int64_t consteval impl_getIndexFromTuple(std::index_sequence<I...> /*Indexes*/) {
		constexpr auto size = std::tuple_size_v<TupleType>;
		std::array<bool, size> solution{
			std::is_same_v<T, std::tuple_element_t<I, TupleType>>...
		};

		for (int64_t i = 0; i < size; i++) {
			if (solution.at(i)) return i;
		}
		return -1;
	}

	template<class T, class TupleType>
	int64_t consteval getIndexFromTuple() {
		constexpr auto val = impl_getIndexFromTuple<T, TupleType>(std::make_index_sequence<std::tuple_size_v<TupleType>>{});
		static_assert(val != -1, "Type not found in the Tuple");
		return val;
	}

	template<template<class...> class, class>
	struct TransferParams;

	template<template<class...> class To, class... Args>
	struct TransferParams<To, std::tuple<Args...>> {
		using type = To<Args...>;
	};

	template<template<class...> class, template<class...> class, class>
	struct TransferWrapParams;

	template<template<class...> class To, template<class...> class Wrapper, class... Args>
	struct TransferWrapParams<To, Wrapper, std::tuple<Args...>> {
		using type = To<Wrapper<Args>...>;
	};

	template<class T>
	struct WrapperClass {
		using type = T;
	};

	template<class T, class Ret, size_t... I>
	std::vector<Ret> impl_FromIndex(std::index_sequence<I...> /*indx*/) {
		return std::vector<Ret>{(Ret{WrapperClass<std::tuple_element_t<I, T>>{}})...};
	}

	template<class T>
	TransferWrapParams<std::variant, WrapperClass, T>::type FromIndex(size_t index) {
		return impl_FromIndex<T, typename TransferWrapParams<std::variant, WrapperClass, T>::type>(std::make_index_sequence<std::tuple_size_v<T>>{}).at(index);
	}

	template<class T>
	void addBytes(std::vector<std::byte> &to, const T &from) {
		const auto &bytes = *std::bit_cast<std::array<std::byte, sizeof(T)> *>(&from);
		for (const auto &byte: bytes) {
			to.emplace_back(byte);
		}
	}

	template<class T>
	T &bytesTo(std::span<const std::byte>::const_iterator &it) {
		T &ret = *std::bit_cast<T *>(std::bit_cast<std::array<std::byte, sizeof(T)> *>(&*it));
		it += sizeof(T);
		return ret;
	}

	template<class T>
	T &staticBytesTo(const std::span<const std::byte>::const_iterator &it) {
		T &ret = *std::bit_cast<T *>(std::bit_cast<std::array<std::byte, sizeof(T)> *>(&*it));
		return ret;
	}
}// namespace Utils