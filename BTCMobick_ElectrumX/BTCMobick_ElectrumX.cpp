#include <bitcoin/system.hpp>
#include <exception>
#include <future>
#include "../bech32/bech32.h"
#include "../bech32/segwit_addr.h"
#include "BTCMobick_ElectrumX.hpp"
#include <cassert>

using namespace libbitcoin;
using namespace libbitcoin::chain;
using namespace libbitcoin::wallet;
using namespace libbitcoin::machine;

namespace ELECTRUMX
{
    ElectrumX_Error::ElectrumX_Error(const std::string& msg, const std::string& type)
    : message(msg), errorType(type) {}

    ElectrumX_Error::~ElectrumX_Error() {}

    void ElectrumX_Error::logError() const {
        std::cerr << errorType << " Error: " << what() << std::endl;
        // Additional logic for logging to file, sending notifications, etc.
    }

    // const char* ElectrumX_Error::what() const noexcept override {
    //     return message.c_str();
    // }

    std::string ElectrumX_Error::getErrorType() const {
        return errorType;
    }


    ElectrumX::ElectrumX(boost::asio::io_context& io_context, const std::string& host, int port)
    : electrum_io_context(io_context), resolver(io_context), socket(io_context), id(0), is_connected(false)
    {
        try {
            this->endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host), port);
            ElectrumX::connect();
        } catch (const boost::system::system_error& error) {
            std::cerr << "Error creating endpoint: " << error.what() << std::endl;
        }
    }

    ElectrumX::~ElectrumX()
    {
        if (this->is_connected == true)
            disconnect();
            electrum_io_context.stop();
    }

    void ElectrumX::connect() 
    {
        bool is_timeout = false;

        this->socket.async_connect(this->endpoint, [this, &is_timeout](const boost::system::error_code& error) {
            if (!error && !is_timeout) {
                std::cout << "Connected!" << std::endl;
                this->is_connected = true;
            } else if (!error) {
                std::cerr << "Connection was established but too late." << std::endl;
                this->socket.close();
            } else {
                std::cerr << "Connection Error: " << error.message() << std::endl;
                if (is_timeout) {
                    std::cerr << "Connection failed due to timeout." << std::endl;
                }
            }
        });

        // Create a timer on the io_context
        boost::asio::deadline_timer timer(this->electrum_io_context, boost::posix_time::seconds(1));
        timer.async_wait([&](const boost::system::error_code& error_code) {
            if (!error_code && !this->is_connected) {
                std::cerr << "Connection timeout, operation will be aborted." << std::endl;
                this->socket.close();
                is_timeout = true;  // Set timeout flag to indicate the cause of connection failure
            }
        });

        // Run the io_context object to handle asynchronous operations
        this->electrum_io_context.reset();
        this->electrum_io_context.run();
    }


    void ElectrumX::disconnect()
    {
        if (this->socket.is_open()) {
            boost::system::error_code error;  // For capturing any error during socket close
            this->socket.close(error);  // Close the socket and capture any errors
            this->is_connected = false;
            if (error) {
                std::cerr << "Failed to close socket: " << error.message() << std::endl;
                throw ElectrumX_Error(error.message(), "Termination Error");
            }
        }
    }

   nlohmann::json ElectrumX::send_requests_receive(const std::string& method, const std::vector<std::variant<std::string, uint64_t, bool> >& params) 
    {
        assert(this->is_connected == true);

        nlohmann::json j;
        j["jsonrpc"] = "2.0";
        j["method"] = method;
        j["params"] = nlohmann::json::array();
        j["id"] = ++id;

        for (const auto& param : params) {
            std::visit([&j](auto&& arg) {
                j["params"].push_back(arg);
            }, param);
        }

        // Convert the JSON object to a string
        std::string request = j.dump() + "\n";
        boost::asio::write(socket, boost::asio::buffer(request));

        boost::asio::streambuf response_buffer;
        boost::system::error_code error;
        std::string response_data;
        
        // Asynchronous read operation
        boost::asio::async_read_until(socket, response_buffer, '\n', 
        [this, &response_buffer, &response_data](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream response_stream(&response_buffer);
                std::string response_part;
                std::getline(response_stream, response_part);
                response_data += response_part;
            } else {
                // Handle errors or log them as needed
                std::cerr << "Error reading: " << ec.message() << std::endl;
            }
        });

        // Since this is an async operation, we need to run the io_context
        this->electrum_io_context.reset();
        this->electrum_io_context.run();

        if (response_data.empty()) {
            throw ElectrumX_Error("No response received", "");
        }
       
        // Process the response
        auto response_json = nlohmann::json::parse(response_data);
        if (response_json.contains("error") && !response_json["error"].is_null()) {
            return response_json["error"]["message"];
        } else if (response_json.contains("result")) {
            return response_json["result"];
            // return response_json;
        }

        throw ElectrumX_Error("Unexpected response structure", response_data);
    }

    nlohmann::json ElectrumX::block_header(const uint64_t height, const uint64_t cp_height)
    {
        nlohmann::json result = ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_block_header), {height, cp_height});
        return ElectrumX::deserialize_headers(result);
    }

    nlohmann::json ElectrumX::block_headers(const uint64_t start_height, const uint64_t count, const uint64_t cp_height)
    {
        nlohmann::json result = ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_block_headers), {start_height, count, cp_height});
        return ElectrumX::deserialize_headers(result);
    }

    nlohmann::json ElectrumX::estimate_fee(const uint64_t number)
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_estimatefee), {number});
    }

    nlohmann::json ElectrumX::headers_subscribe()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_headers_subscribe), {});
    }

    nlohmann::json ElectrumX::get_balance(const std::string& address)
    {
        std::string electrum_scripthash = ElectrumX::address_to_electrum_scripthash(address);
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_scripthash_get_balance), {electrum_scripthash});
    }

    nlohmann::json ElectrumX::get_history(const std::string& address)
    {
        std::string electrum_scripthash = ElectrumX::address_to_electrum_scripthash(address);
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_scripthash_get_history), {electrum_scripthash});
    }

    nlohmann::json ElectrumX::get_mempool(const std::string& address)
    {
        std::string electrum_scripthash = ElectrumX::address_to_electrum_scripthash(address);
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_scripthash_get_mempool), {electrum_scripthash});
    }

    nlohmann::json ElectrumX::list_unspent(const std::string& address)
    {
        std::string electrum_scripthash = ElectrumX::address_to_electrum_scripthash(address);
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_scripthash_listunspent), {electrum_scripthash});
    }

    nlohmann::json ElectrumX::broadcast_transaction(const std::string& tx_hexstring)
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_transaction_broadcast), {tx_hexstring});
    }

    nlohmann::json ElectrumX::get_transaction(const std::string& txid, bool verbose)
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_transaction_get), {txid, verbose});
    }

    nlohmann::json ElectrumX::get_merkle(const std::string& txid, const uint64_t height)
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_transaction_get_merkle), {txid, height});
    }

    // nlohmann::json ElectrumX::get_tsc_merkle(const std::string& txid, uint64_t height, std::string txid_or_tx = "txid", std::string target_type = "block_hash")
    // {
    //     return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_transaction_get_tsc_merkle), {txid, height, txid_or_tx, target_type});
    // }   

    nlohmann::json ElectrumX::id_from_pos(const uint64_t height, const uint64_t tx_pos, bool merkle)
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::blockchain_transaction_id_from_pos), {height, tx_pos, merkle});
    }

    nlohmann::json ElectrumX::get_fee_histogram()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::mempool_get_fee_histogram), {});
    }

    nlohmann::json ElectrumX::server_banner()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::electrum_server_banner), {});
    }

    nlohmann::json ElectrumX::server_donation()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::electrum_server_donation_address), {});
    }

    nlohmann::json ElectrumX::server_features()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::electrum_server_features), {});
    }

    nlohmann::json ElectrumX::server_peers_subscribe()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::electrum_server_peers_subscribe), {});
    }

    nlohmann::json ElectrumX::server_ping()
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::electrum_server_ping), {});
    }

    nlohmann::json ElectrumX::server_version(const std::string& client_name, const std::string& protocol_version)
    {
        return ElectrumX::send_requests_receive(ElectrumX::methodName(ElectrumX::electrum_server_version), {client_name, protocol_version});
    }

    // latest libbitcoin release version (version 3) doesn't support witness addresses (i.e native segwit and taproot addresses)
    // the derivation of scripthash for witness addresses rely on bech32.h and segwit_addr.h files (written by Pieter Wuille)
    std::string ElectrumX::address_to_electrum_scripthash(const std::string& address)
    {
        std::string electrum_scriptHash;
        script scriptPubKey;

        std::pair<int, std::vector<uint8_t> > decoded_result = segwit_addr::decode("bc", address);
        int witness_version = decoded_result.first;  // Native Segwit => (0) Taproot => (1) Decode Failed => (-1)
        data_chunk witness_program = decoded_result.second;

        if (witness_version == 0) {
            // P2WPKH or P2WSH address
            scriptPubKey = script({operation(opcode(0)), operation(witness_program)});
        }
        else if (witness_version == 1) {
            // P2TR address
            scriptPubKey = script({operation(opcode(81)), operation(witness_program)});
        }
        else if (witness_version == -1) {
            wallet::payment_address bitcoin_address(address);
            if (bitcoin_address.version() == wallet::payment_address::mainnet_p2kh) {
                // Legacy address
                scriptPubKey = script().to_pay_key_hash_pattern(bitcoin_address.hash());
            }
            else if (bitcoin_address.version() == wallet::payment_address::mainnet_p2sh) {
                // P2SH address
                scriptPubKey = script().to_pay_script_hash_pattern(bitcoin_address.hash());
            }
        }

        hash_digest scriptHash = sha256_hash(scriptPubKey.to_data(0));
        std::reverse(scriptHash.begin(), scriptHash.end());
        electrum_scriptHash = encode_base16(scriptHash);

        return electrum_scriptHash;
    }

    nlohmann::json ElectrumX::deserialize_headers(const nlohmann::json& headers)
    {
        nlohmann::json serialized_headers;
        std::string headers_string;
        int count = 1;

        if (headers.contains("hex")) {
            headers_string = headers.at("hex").get<std::string>();
            count = headers.at("count").get<int>();
        }
        else {
            headers_string = headers.get<std::string>();
        }

        assert(headers_string.length() % 160 == 0);
        assert(headers_string.length() / 160 == count);

        libbitcoin::data_chunk data;
        libbitcoin::decode_base16(data, headers_string);
        std::reverse(data.begin(), data.end());
        std::string headers_string_big = libbitcoin::encode_base16(data);

        for (int i = count; i > 0; i--) {
            nlohmann::json j;
            int baseIndex = 160 * (i - 1);
            j["version"] = headers_string_big.substr(baseIndex + 152, 8);
            j["prevhash"] = headers_string_big.substr(baseIndex + 88, 64);
            j["merkle_root"] = headers_string_big.substr(baseIndex + 24, 64);
            j["timestamp"] = headers_string_big.substr(baseIndex + 16, 8);
            j["bits"] = headers_string_big.substr(baseIndex + 8, 8);
            j["nonce"] = headers_string_big.substr(baseIndex, 8);
            serialized_headers.push_back(j);
        }
        return serialized_headers;
    }

    const std::string& ElectrumX::methodName(const ElectrumX::methods method)
    {
        static const std::string names[] = {
            "blockchain.block.header",
            "blockchain.block.headers",
            "blockchain.estimatefee",
            "blockchain.headers.subscribe",
            "blockchain.scripthash.get_balance",
            "blockchain.scripthash.get_history",
            "blockchain.scripthash.get_mempool",
            "blockchain.scripthash.listunspent",
            "blockchain.transaction.broadcast", 
            "blockchain.transaction.get",
            "blockchain.transaction.get_merkle",
            "blockchain.transaction.get_tsc_merkle",
            "blockchain.transaction.id_from_pos",
            "mempool.get_fee_histogram",
            "server.banner",
            "server.donation_address",
            "server.features",
            "server.peers.subscribe",
            "server.ping",
            "server.version"};

        return names[method];
    }
;}
