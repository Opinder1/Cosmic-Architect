#pragma once

constexpr static const size_t k_worker_thread_max = 16;

template<class DataT>
struct alignas(std::hardware_destructive_interference_size) AlignedData : DataT {};

template<class DataT>
using PerThread = std::array<AlignedData<DataT>, k_worker_thread_max>;