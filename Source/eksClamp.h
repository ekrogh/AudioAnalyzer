#pragma once
template<typename T1, typename T2>
constexpr auto eks_min(T1 a, T2  b) { return (((a) < (b)) ? (a) : (b)); }
template<typename T1, typename T2>
constexpr auto eks_max(T1 a, T2  b) { return (((a) > (b)) ? (a) : (b)); }
template<typename T1, typename T2, typename T3>
constexpr auto eks_clamp(T1 x, T2  lo, T3  hi) { return (eks_max((lo), eks_min((hi), (x)))); }

