// C++ implementation of https://github.com/eklitzke/utxodump

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <leveldb/db.h>
#include <leveldb/options.h>
#include <leveldb/iterator.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <cstdint>

// Constants
const uint8_t COIN = 67;
const std::vector<uint8_t> OBFUSCATE_KEY = {0x0e, 0x00, 'o', 'b', 'f', 'u', 's', 'c', 'a', 't', 'e', '_', 'k', 'e', 'y'};
const int BATCH_SIZE = 1000;

class UTXO_DB_READER
{
    private:
        std::string db_path;
        leveldb::DB* pdb;
        leveldb::Options options;
        std::vector<uint8_t> obf_key;

    public:
        UTXO_DB_READER(const std::string& db_path);
        ~UTXO_DB_READER();

        void Init();
        void get_obfuscate_key();
        void dump_chainstate_csv(const std::string& output_file);

        void check();
        uint64_t process_batch_amount(std::vector<std::pair<std::string, std::vector<uint8_t>>>& batch, const std::vector<uint8_t>& obf_key, const uint64_t block_height);
        uint64_t add_utxo_amount(const unsigned char start_byte, const unsigned char end_byte, const uint64_t block_height);
};
