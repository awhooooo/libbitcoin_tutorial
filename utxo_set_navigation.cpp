#include "utxo_set_navigation.hpp"

// #include <spdlog/spdlog.h>

// XOR decryption function
void decrypt(std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key[i % key.size()];
    }
}

// Varint decoding
std::pair<uint64_t, size_t> decode_varint(const std::vector<uint8_t>& data, size_t offset = 0) {
    uint64_t n = 0;
    size_t i = 0;
    for (; i < data.size(); ++i) {
        uint8_t c = data[offset + i];
        n = (n << 7) | (c & 0x7f);
        if (c & 0x80) {
            n += 1;
        } else {
            break;
        }
    }
    return {n, i + 1};
}

// Amount decompression
uint64_t decompress_amount(uint64_t x) {
    if (x == 0) return 0;
    x -= 1;
    int e = x % 10;
    x /= 10;
    uint64_t n = 0;
    if (e < 9) {
        int d = (x % 9) + 1;
        x /= 9;
        n = x * 10 + d;
    } else {
        n = x + 1;
    }
    while (e--) {
        n *= 10;
    }
    return n;
}

// Decode key to (txid, vout)
std::pair<std::string, uint64_t> decode_key(const std::string& key) {
    assert(key[0] == COIN);
    std::string txid = key.substr(1, 32);
    std::reverse(txid.begin(), txid.end());
    std::stringstream ss;
    for (unsigned char c : txid) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    auto [vout, declen] = decode_varint(std::vector<uint8_t>(key.begin() + 33, key.end()));
    return {ss.str(), vout};
}

// Decode value to (height, coinbase, amount, scriptsize)
std::tuple<uint64_t, bool, uint64_t, size_t> decode_val(const std::vector<uint8_t>& val) {
    auto [code, consumed] = decode_varint(val);
    bool coinbase = code & 1;
    uint64_t height = code >> 1;
    auto [amount, rem] = decode_varint(val, consumed);
    return {height, coinbase, decompress_amount(amount), val.size() - rem};
}

UTXO_DB_READER::UTXO_DB_READER(const std::string& db_path)
{
    this->db_path = db_path;
}

UTXO_DB_READER::~UTXO_DB_READER()
{
    delete this->pdb;
}

void UTXO_DB_READER::Init()
{
    this->options.create_if_missing = false;
    leveldb::Status status = leveldb::DB::Open(this->options, this->db_path, &(this->pdb));
    if (!status.ok()) {
        // spdlog::error("Unable to open LevelDB database: {}", status.ToString());
        std::cerr << "Unable to open LevelDB database: " << status.ToString() << std::endl;
        return;
    }
    // spdlog::info("Successfully opened LevelDB database: {}", status.ToString());
    std::cout << "Successfully opened LevelDB database: " << status.ToString() << std::endl;
}

// Function to get the obfuscation key
void UTXO_DB_READER::get_obfuscate_key()
{
    std::string secret;
    leveldb::Status status = this->pdb->Get(leveldb::ReadOptions(), leveldb::Slice(reinterpret_cast<const char*>(OBFUSCATE_KEY.data()), OBFUSCATE_KEY.size()), &secret);
    assert(status.ok());
    assert(secret[0] == 8 && secret.size() == 9);
    this->obf_key = std::vector<uint8_t>(secret.begin() + 1, secret.end());
}

void UTXO_DB_READER::dump_chainstate_csv(const std::string& output_file)
{
    std::ofstream csv_file(output_file);
    csv_file << "txid, vout, height, coinbase, amount, scriptsize\n";

    leveldb::Iterator* it = this->pdb->NewIterator(leveldb::ReadOptions());
    for (it->Seek("C"); it->Valid() && it->key().ToString()[0] == 'C'; it->Next()) {
        std::string key = it->key().ToString();
        std::vector<uint8_t> value(it->value().data(), it->value().data() + it->value().size());
        
        decrypt(value, this->obf_key);

        auto [txid, vout] = decode_key(key);
        auto [height, coinbase, amount, scriptsize] = decode_val(value);

        csv_file << txid << " ," << vout << " ," << height << " ," << coinbase << " ," << amount << " ," << scriptsize << "\n";
    }
    delete it;
    csv_file.close();
}

uint64_t UTXO_DB_READER::add_utxo_amount(const uint64_t block_height)
{
    uint64_t total_amount = 0;
    leveldb::Iterator* it = this->pdb->NewIterator(leveldb::ReadOptions());
    for (it->Seek("C"); it->Valid() && it->key().ToString()[0] == 'C'; it->Next()) {
        std::string key = it->key().ToString();
        std::vector<uint8_t> value(it->value().data(), it->value().data() + it->value().size());
        
        decrypt(value, this->obf_key);

        auto [txid, vout] = decode_key(key);
        auto [height, coinbase, amount, sz] = decode_val(value);
        if (height < block_height) {
            total_amount += amount;
        }
    }
    
    if (!it->status().ok()) {
        std::cerr << "An error occurred during iteration: " << it->status().ToString() << std::endl;
    }

    delete it; // Clean up the iterator
    return total_amount;
}

int main() {

    const std::string db_path = "/Users/legacy/BitcoinCore/bitcoin-testing/testing3 copy 13/chainstate";
    std::string output_file = "chainstate.csv";
    
    UTXO_DB_READER database(db_path);
    database.Init();
    database.get_obfuscate_key();
    database.dump_chainstate_csv(output_file);
    std::cout << database.add_utxo_amount(37) << std::endl;;
    
    return EXIT_SUCCESS;
}