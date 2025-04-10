#include <tuple>
#include <utility>
#include "Addressing.h"
#include "Instruction.h"

using Mul = Instruction<
    0x0,2,
    RegisterAddressing<3>,
    Skip<3>,
    ImmediateAddressing<5>,
    ImmediateAddressing<4>,
    Skip<8>
>;



int main() {
    auto mem = AdvancedMemory<4,32,8>();

    auto a = Word<Mul::total_size>();
    Mul::execute(a, mem);
    return 0;
}
