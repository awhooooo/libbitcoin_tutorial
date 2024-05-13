// => Use command "brew list" and check whether gmp and pkg-config is installed
// => assuming libbitcoin is installed with command "./install.sh --prefix=/home/me/myprefix --build-boost --disable-shared"
//    compile this cpp file with command "g++ -std=c++11 -o genesis genesis.cpp \
//     -I/home/me/myprefix/include $(pkg-config --cflags --libs libbitcoin-system libsecp256k1 gmp) -Wno-deprecated"

#include <bitcoin/system.hpp>
using namespace bc;

int main()
{
        // Extracting Satoshi's words from genesis block.
        const auto block = bc::chain::block::genesis_mainnet();
        const auto& coinbase = block.transactions().front();
        const auto& input = coinbase.inputs().front();
        BITCOIN_ASSERT_MSG(input.script().size() > 2u, "unexpected genesis");
        const auto headline = input.script()[2].data();
        std::string message(headline.begin(), headline.end());
        std::cout << message << std::endl;
        return EXIT_SUCCESS;
}
