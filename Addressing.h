#ifndef ADDRESSINGMODE_H
#define ADDRESSINGMODE_H

#include "Memory.h"

template <typename Derived, size_t Size>
struct  AddressingMode {
    using address_type = Word<Size>;
    static constexpr size_t size = Size;

    template <BasicMemory MemType>
    typename MemType::word_type read_by_address(const address_type& address, MemType& memory) const {
        return static_cast<const Derived*>(this)->read_by_address_impl(address, memory);
    }
};

template <size_t Size>
struct ImmediateAddressing : public AddressingMode<ImmediateAddressing<Size>, Size> {
    template <BasicMemory MemType>
    typename MemType::word_type read_by_address_impl(const typename ImmediateAddressing::address_type& address,MemType& memory) const {
            return typename MemType::word_type();
    };
};

template <size_t Size>
struct RegisterAddressing : public AddressingMode<RegisterAddressing<Size>, Size> {
    template <MemoryWithRegisters MemType>
    typename MemType::word_type read_by_address_impl(const typename RegisterAddressing::address_type& address, MemType& memory) const {
        return typename MemType::word_type();
    };
};

#endif //ADDRESSINGMODE_H
