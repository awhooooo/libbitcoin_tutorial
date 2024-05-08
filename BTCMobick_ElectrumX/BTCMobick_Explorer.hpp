#ifndef BTCMOBICK_EXPLORER_H
#define BTCMOBICK_EXPLORER_H

#include <iostream>
#include <string>
#include <exception>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

namespace EXPLORER_API
{
    class Explorer_API_Error : public std::exception
    {
        private:
            std::string message;
            std::string errorType;

        public:
            // Constructor to initialize error details
            Explorer_API_Error(const std::string& msg, const std::string& type);
            ~Explorer_API_Error();

            // Override what() to provide error message
            const char* what() const noexcept override {
                return message.c_str();
            }

            // Method to log the error, potentially with different treatments based on error type
            void logError() const;

            // Accessor method for error type
            std::string getErrorType() const;
    };

    class Explorer_API
    {
        private:
            std::string base_url;
            CURL *curl;
            static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);
            nlohmann::json request_and_jsonize(const std::string& url);

        public:
            Explorer_API();
            ~Explorer_API();

            nlohmann::json get_utxo_set();
            nlohmann::json get_address(const std::string& address);
            nlohmann::json get_next_block();
            nlohmann::json get_unconfirmed_tx();
            nlohmann::json get_transaction(const std::string& txid);
            nlohmann::json get_block(const int& block);
            nlohmann::json get_block_header(const int& block);
            nlohmann::json get_height();

            enum api_list {
                utxo_set,
                address,
                mining,
                unconfirmed_txs,
                transaction,
                block,
                block_header,
                height,
            };

            static const std::string& methodName(const api_list method);
    };
};

#endif
