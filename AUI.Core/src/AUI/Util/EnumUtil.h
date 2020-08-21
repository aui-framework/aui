#pragma once


#define ENUM_FLAG(name) enum name: int; \
				   constexpr inline name operator|(name a, name b) {return static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
				   constexpr inline name operator&(name a, name b) {return static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
				   constexpr inline name operator^(name a, name b) {return static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
				   constexpr inline name operator|=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
				   constexpr inline name operator&=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
				   constexpr inline name operator^=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
				   enum name: int
