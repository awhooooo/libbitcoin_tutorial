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

        // calculates the entire amount of coin recorded below the parameter height
        uint64_t add_utxo_amount(const uint64_t block_height);
};