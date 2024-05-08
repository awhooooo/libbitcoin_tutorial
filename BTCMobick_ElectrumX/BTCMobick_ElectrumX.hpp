#ifndef BTCMOBICK_ELECTRUM_H
#define BTCMOBICK_ELECTRUM_H

#include <exception>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace ELECTRUMX 
{
    class ElectrumX_Error : public std::exception 
    {
        private:
            std::string message;
            std::string errorType;

        public:
            // Constructor to initialize error details
            ElectrumX_Error(const std::string& msg, const std::string& type);
            ~ElectrumX_Error();

            // Override what() to provide error message
            const char* what() const noexcept override {
                return message.c_str();
            }

            // Method to log the error, potentially with different treatments based on error type
            void logError() const;

            // Accessor method for error type
            std::string getErrorType() const;
    };

    class ElectrumX
    {
        private:
            boost::asio::io_context& electrum_io_context;
            boost::asio::ip::tcp::resolver resolver;
            boost::asio::ip::tcp::socket socket;
            boost::asio::ip::tcp::endpoint endpoint;
            std::thread io_thread;
            int id;
            bool is_connected;

        public:
            // ElectrumX();
            ElectrumX(boost::asio::io_context& io_context, const std::string& host, int port);
            ~ElectrumX();

            void connect();
            void disconnect();
            nlohmann::json send_requests_receive(const std::string& method, const std::vector< std::variant<std::string, uint64_t, bool> >& params);

            nlohmann::json block_header(const uint64_t height, const uint64_t cp_height = 0);
            nlohmann::json block_headers(const uint64_t start_height, const uint64_t count, const uint64_t cp_height);
            nlohmann::json estimate_fee(const uint64_t number);
            nlohmann::json headers_subscribe();
            nlohmann::json get_balance(const std::string& address);
            nlohmann::json get_history(const std::string& address);
            nlohmann::json get_mempool(const std::string& address);
            nlohmann::json list_unspent(const std::string& address);
            nlohmann::json broadcast_transaction(const std::string& tx_hexstring);
            nlohmann::json get_transaction(const std::string& txid, bool verbose = true);
            nlohmann::json get_merkle(const std::string& txid, const uint64_t height);
            // nlohmann::json get_tsc_merkle(const std::string& txid, const uint64_t height, std::string txid_or_tx = "txid", std::string target_type = "block_hash");
            nlohmann::json id_from_pos(const uint64_t height, const uint64_t tx_pos, bool merkle = true);
            nlohmann::json get_fee_histogram();
            nlohmann::json server_banner();
            nlohmann::json server_donation();
            nlohmann::json server_features();
            nlohmann::json server_peers_subscribe();
            nlohmann::json server_ping();
            nlohmann::json server_version(const std::string& client_name = "", const std::string& protocol_version = "1.4");

            std::string address_to_electrum_scripthash(const std::string& address);
            static nlohmann::json deserialize_headers(const nlohmann::json& headers);

            enum methods
            {blockchain_block_header,
             blockchain_block_headers,
             blockchain_estimatefee,
             blockchain_headers_subscribe,
             blockchain_scripthash_get_balance,
             blockchain_scripthash_get_history,
             blockchain_scripthash_get_mempool,
             blockchain_scripthash_listunspent,
             blockchain_transaction_broadcast, 
             blockchain_transaction_get,
             blockchain_transaction_get_merkle,
             blockchain_transaction_get_tsc_merkle,
             blockchain_transaction_id_from_pos,
             mempool_get_fee_histogram,
             electrum_server_banner,
             electrum_server_donation_address,
             electrum_server_features,
             electrum_server_peers_subscribe,
             electrum_server_ping,
             electrum_server_version};
            
            static const std::string& methodName(const methods method);
    };
};

#endif // BTCMOBICK_ELECTRUM_H
