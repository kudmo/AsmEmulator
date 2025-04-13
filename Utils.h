#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <concepts>
#include <stdexcept>

using bits_t = std::vector<bool>;
using nums_t = long long int;

template <typename T>
concept BitArray =
    requires {
    typename T::iterator;
    typename T::const_iterator;
    } &&

    std::default_initializable<T> &&
    std::copy_constructible<T> &&
    std::move_constructible<T> &&
    std::constructible_from<T, typename T::const_iterator, typename T::const_iterator> &&
    std::constructible_from<bits_t> &&

    std::ranges::input_range<T> &&

    requires(const T& arr, size_t index) {
    { arr.size() } -> std::same_as<size_t>;
    { arr[index] } -> std::convertible_to<bool>;
    };

/**
 * @brief A fixed-size binary word template class.
 *
 * @tparam Size The compile-time specified size of the Word (number of bits).
 *
 * This class represents a binary word of fixed size, stored as a sequence of bits.
 * It provides iterator access, element-wise operations, and size queries.
 * The underlying storage is a bits_t, but access is constrained to the first `Size` bits.
 */
template <size_t Size>
struct Word {
    /// Internal storage for bits
    bits_t bits = bits_t(Size);
    /// Iterator type for mutable bit access
    using iterator = bits_t::iterator;
    /// Iterator type for read-only bit access
    using const_iterator = bits_t::const_iterator;

    /**
     * @brief Default constructor.
     * @post Initializes all bits to false (0).
     */
    explicit Word() = default;
    /**
    * @brief Copy constructor.
    * @post Copy word value from the other.
    */
    Word(const Word&) = default;
    /**
    * @brief Move constructor.
    * @post Move word value from the other.
    */
    Word(Word&&) = default;
    /**
    * @brief Copy operator.
    * @post Copy word value from the other.
    */
    Word& operator=(const Word&) = default;
    /**
    * @brief Move operator.
    * @post Move word value from the other.
    */
    Word& operator=(Word&&) = default;
    /**
     * @brief Constructs a Word from a vector of booleans.
     * @param bits Source bits vector.
     * @throws std::invalid_argument if bits.size() not equal to Size
     */
    explicit Word(const bits_t& bits) {
        if (bits.size() != Size)
            throw std::invalid_argument("Word size must be equal to the size of vector");
        std::copy(bits.begin(), bits.end(), bits.begin());
    }

    /**
     * @brief Constructs a Word from an iterator range.
     * @param begin Start iterator of the bit sequence.
     * @param end End iterator of the bit sequence.
     * @throws std::invalid_argument if distance between begin and end not equal to Size
     */
    explicit Word(const_iterator begin, const_iterator end) {
        if (std::distance(begin, end) != Size)
            throw std::invalid_argument("Word size must be equal to the size of vector");
        std::copy(begin, end, bits.begin());
    }

    /**
     * @brief Accesses the bit at the specified position.
     * @param index Zero-based bit position (0 <= index < Size).
     * @return bool Value of the bit at `index`.
     * @throws std::invalid_argument if index out of bounds
     */
    bool operator[](const size_t index) const {
        if (index >= Size)
            throw std::invalid_argument("Word index out of bounds");
        return bits[index];
    }

    /**
     * @brief Returns the compile-time size of the Word.
     * @return constexpr size_t Always equal to the template parameter `Size`.
     */
    constexpr size_t size() const { return Size; }

    /**
     * @brief Returns an iterator to the first bit.
     */
    iterator begin() { return bits.begin(); }

    /**
     * @brief Returns an iterator past the last valid bit (position `Size`).
     */
    iterator end() { return bits.begin() + Size; }

    /**
     * @brief Returns a const iterator to the first bit.
     */
    const_iterator begin() const { return bits.begin(); }

    /**
     * @brief Returns a const iterator past the last valid bit (position `Size`).
     */
    const_iterator end() const { return bits.begin() + Size; }

    friend std::ostream& operator<< (std::ostream& out, const Word<Size>& value) {
        for (auto i : value) out << i;
        return out;
    }
};

enum class NumberEncoding {
    Unsigned,
    DirectCode,
    ReverseCode,
    AdditionCode,
};

template <BitArray bitarray_t>
long long int decode(const bitarray_t& bitarray, NumberEncoding encoding) {
    long long int result = 0;
    switch (encoding) {
        case NumberEncoding::Unsigned: {
            for (auto it = bitarray.begin(); it != bitarray.end(); ++it) {
                result = (result << 1) + static_cast<size_t>(*it);
            }
            break;
        }
        case NumberEncoding::DirectCode: {
            const bool is_negative = bitarray[0];
            for (auto it = bitarray.begin()+1; it != bitarray.end(); ++it) {
                result = (result << 1) + static_cast<size_t>(*it);
            }
            if (is_negative) result *= -1;
            break;
        }
        case NumberEncoding::ReverseCode: {
            const bool is_negative = bitarray[0];

            for (auto it = bitarray.begin()+1; it != bitarray.end(); ++it) {
                result = (result << 1) + static_cast<int>(is_negative ? !*it : *it);
            }

            result = is_negative ? -result : result;
            break;
        }
        case NumberEncoding::AdditionCode: {
            const bool is_negative = bitarray[0];

            for (auto it = bitarray.begin()+1; it != bitarray.end(); ++it) {
                result = (result << 1) + static_cast<int>(is_negative ? !*it : *it);
            }

            if (is_negative) {
                result += 1;
                result = -result;
            }
            break;
        }
        default:
            throw std::invalid_argument("Invalid encoding value");
    }

    return result;
}

//!@todo Добавить encode

#endif //UTILS_H
