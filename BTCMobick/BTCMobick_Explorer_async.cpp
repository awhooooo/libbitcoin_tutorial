#include "BTCMobick_Explorer_async.hpp"
#include <iostream>

namespace EXPLORER_API_ASYNC
{
    Explorer_API_Client::Explorer_API_Client(net::io_context& explorer_ioc)
    : resolver_(net::make_strand(explorer_ioc)), stream_(net::make_strand(explorer_ioc)), buffer_(8192)
    {
        this->host = "blockchain.mobick.info";
        this->port = "80";
    }

    Explorer_API_Client::~Explorer_API_Client()
    {

    }

    // Start the asynchronous operation
    std::future<nlohmann::json> Explorer_API_Client::run(const std::string& api_type, int version)
    {
        this->json_promise = std::promise<nlohmann::json>();

        // Set up an HTTP GET request message
        this->req_.version(version);
        this->req_.method(http::verb::get);
        this->req_.target(api_type);
        this->req_.set(http::field::host, this->host);
        this->req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        this->resolver_.async_resolve(this->host, this->port,
            beast::bind_front_handler(&Explorer_API_Client::on_resolve, shared_from_this()));
        
        this->res_ = {};
        return this->json_promise.get_future();
    }

    void Explorer_API_Client::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if (ec) {
            std::cerr << "Resolve Error" << ": " << ec.message() << "\n";
            this->json_promise.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            return;
        }

        // Set a timeout on the operation
        this->stream_.expires_after(std::chrono::seconds(30));
        
        // Make the connection on the IP address we get from a lookup
        this->stream_.async_connect(results,
            beast::bind_front_handler(&Explorer_API_Client::on_connect, shared_from_this()));
    }

    void Explorer_API_Client::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if (ec) {
            std::cerr << "Connect Error" << ": " << ec.message() << "\n";
            this->json_promise.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            return;
        }

        // Set a timeout on the operation
        this->stream_.expires_after(std::chrono::seconds(30));
        
        // Send the HTTP request to the remote host
        http::async_write(this->stream_, this->req_,
            beast::bind_front_handler(&Explorer_API_Client::on_write, shared_from_this()));
    }

    void Explorer_API_Client::on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            std::cerr << "Write Error" << ": " << ec.message() << "\n";
            this->json_promise.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            return;
        }
    
        // Receive the HTTP response
        http::async_read(this->stream_, this->buffer_, this->res_,
            beast::bind_front_handler(&Explorer_API_Client::on_read, shared_from_this()));
    }

    void Explorer_API_Client::on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            std::cerr << "Read Error" << ": " << ec.message() << "\n";
            this->json_promise.set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
            return;
        }
        
        try {
            nlohmann::json json_response = nlohmann::json::parse(res_.body());
            this->json_promise.set_value(json_response);
        } 
        catch(const std::exception& e) {
            this->json_promise.set_exception(std::make_exception_ptr(e));
        }

        // Gracefully close the socket
        this->stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes so don't bother reporting it.
        if (ec && ec != beast::errc::not_connected)
            std::cerr << "Shutdown" << ": " << ec.message() << "\n";

        // If we get here then the connection is closed gracefully
    }

    std::future<nlohmann::json> Explorer_API_Client::block(int block_number)
    {
        std::string api_type = "/api/block/" + std::to_string(block_number);
        return this->run(api_type, 11);
    }

    std::future<nlohmann::json> Explorer_API_Client::block_header(int block_number)
    {
        std::string api_type = "/api/block/header/" + std::to_string(block_number);
        return this->run(api_type, 11);
    }

    std::future<nlohmann::json> Explorer_API_Client::current_height()
    {
        std::string api_type = "/api/blocks/tip";
        return this->run(api_type, 11);
    }

    std::future<nlohmann::json> Explorer_API_Client::transaction(const std::string& txid)
    {
        std::string api_type = "/api/tx/" + txid;
        return this->run(api_type, 11);
    }

    std::future<nlohmann::json> Explorer_API_Client::utxo_set()
    {
        std::string api_type = "/api/blockchain/utxo-set";
        return this->run(api_type, 11);
    }

    std::future<nlohmann::json> Explorer_API_Client::address(const std::string& addr)
    {
        std::string api_type = "/api/address/" + addr;
        return this->run(api_type, 11);
    }
};
