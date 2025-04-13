#include <tuple>
#include <utility>
#include "Addressing.h"
#include "Instruction.h"

using Mul = TwoAddressInstruction<
    0x0,2,
    RegisterAddressing<3>,
    RegisterAddressing<3>
>;


int main() {
    auto mem = AdvancedMemory<4,32,8>();

    std::vector<bool> cmd = {0,0,0,1,0,1,0,0,0,0};

    return 0;
}
