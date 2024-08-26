#include "utxo_set_navigation.hpp"

#include <thread>
#include <future>

int main() {

    const std::string db_path = "/Users/legacy/BitcoinCore/bitcoin-testing/testing3 copy 25/chainstate";
    std::string output_file = "chainstate.csv";
    
    UTXO_DB_READER database(db_path);
    database.Init();
    database.get_obfuscate_key();
    // database.dump_chainstate_csv(output_file);
    // database.check();

    std::vector<std::pair<unsigned char, unsigned char>> ranges = {
        {0x00, 0x3F},  // First range: 0x00 to 0x3F
        {0x40, 0x7F},  // Second range: 0x40 to 0x7F
        {0x80, 0xBF},  // Third range: 0x80 to 0xBF
        {0xC0, 0xFF}   // Fourth range: 0xC0 to 0xFF
    };

    // Use std::future to capture the results from threads
    std::vector<std::future<uint64_t>> futures;
    futures.reserve(4);

    for (int i = 0; i < 4; i++) {
        // Use std::async to launch the task asynchronously
        futures.emplace_back(std::async(std::launch::async, &UTXO_DB_READER::add_utxo_amount, &database, ranges[i].first, ranges[i].second, 43));
    }

    // Aggregate results from all threads
    uint64_t total_utxo_amount = 0;
    for (auto& fut : futures) {
        total_utxo_amount += fut.get();
    }

    std::cout << "Total UTXO Amount: " << total_utxo_amount << std::endl;

    return EXIT_SUCCESS;
}
