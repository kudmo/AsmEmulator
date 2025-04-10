#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <tuple>
#include <utility>
#include <type_traits>
#include <array>
#include "Memory.h"
#include "Addressing.h"

template <size_t N>
struct Skip {};

template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset, typename... Args>
class InstructionImpl;

template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset>
class InstructionImpl<CodeVal, CodeSize, CurrentOffset> {
public:
    static constexpr auto fields = std::make_tuple();
    static constexpr size_t total_size = CurrentOffset;
};

template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset,
          template <size_t> class Addressing, size_t Size, typename... Rest>
class InstructionImpl<CodeVal, CodeSize, CurrentOffset, Addressing<Size>, Rest...> {
private:
    using Next = InstructionImpl<CodeVal, CodeSize, CurrentOffset + Size, Rest...>;
public:
    static constexpr auto fields = std::tuple_cat(
        std::make_tuple(std::make_pair(CurrentOffset, Addressing<Size>())),
        Next::fields
    );
    static constexpr size_t total_size = Next::total_size;
};

template <size_t CodeVal, size_t CodeSize, size_t CurrentOffset, size_t N, typename... Rest>
class InstructionImpl<CodeVal, CodeSize, CurrentOffset, Skip<N>, Rest...> {
private:
    using Next = InstructionImpl<CodeVal, CodeSize, CurrentOffset + N, Rest...>;
public:
    static constexpr auto fields = Next::fields;
    static constexpr size_t total_size = Next::total_size;
};

template <size_t CodeVal, size_t CodeSize, typename... Args>
class Instruction {
    using Impl = InstructionImpl<CodeVal, CodeSize, CodeSize, Args...>;

public:
    static constexpr size_t code        = CodeVal;
    static constexpr size_t code_size   = CodeSize;
    static constexpr size_t total_size  = Impl::total_size;
    static constexpr auto   fields      = Impl::fields;
    static constexpr size_t fields_count= std::tuple_size_v<decltype(fields)>;

    template <size_t I>
    static constexpr auto get_field() {
        static_assert(I < fields_count, "Index out of range");
        return std::get<I>(fields);
    }

    template <BasicMemory MemType>
    static void execute(const Word<total_size>& word, MemType& memory) {
        execute_impl<MemType>(word, memory, std::make_index_sequence<fields_count>{});
    }

private:
    template <BasicMemory MemType, size_t... Is>
    static void execute_impl(const Word<total_size>& bits,
                             MemType& memory,
                             std::index_sequence<Is...>)
    {
        ( get_field<Is>().second.read_by_address(
              extract_field<Is>(bits),
              memory
          ), ... );
    }

    template <size_t I>
    static auto extract_field(const Word<total_size>& bits) {
        constexpr auto field  = get_field<I>();
        constexpr auto offset = field.first;
        constexpr auto addr   = field.second;
        constexpr auto size   = decltype(addr)::size;

        Word<size> result(bits.begin() + offset, bits.begin() + offset + size);
        return result;
    }
};

#endif //INSTRUCTION_H
