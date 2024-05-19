#ifndef BTCMOBICK_EXPLORER_ASYNC_HPP
#define BTCMOBICK_EXPLORER_ASYNC_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <future>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace EXPLORER_API_ASYNC
{
    class Explorer_API_Client : public std::enable_shared_from_this<Explorer_API_Client>
    {
        private:
            tcp::resolver resolver_;
            beast::tcp_stream stream_;
            beast::flat_buffer buffer_; // (Must persist between reads)
            http::request<http::empty_body> req_;
            http::response<http::string_body> res_;
            std::promise<nlohmann::json> json_promise;

            std::string host;
            std::string port;
        
        public:
            Explorer_API_Client(net::io_context& explorer_ioc);
            ~Explorer_API_Client();
            
            std::future<nlohmann::json> run(const std::string& api_type, int version);
            void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
            void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
            void on_write(beast::error_code ec, std::size_t bytes_transferred);
            void on_read(beast::error_code ec, std::size_t bytes_transferred);
            
            std::future<nlohmann::json> block(int block_number);
            std::future<nlohmann::json> block_header(int block_number);
            std::future<nlohmann::json> current_height();
            std::future<nlohmann::json> transaction(const std::string& txid);
            std::future<nlohmann::json> utxo_set();
            std::future<nlohmann::json> address(const std::string& addr);
    };
};

#endif // BTCMOBICK_EXPLORER_ASYNC_HPP