#include "BTCMobick_Explorer.hpp"
#include <future>


namespace EXPLORER_API
{
    Explorer_API_Error::Explorer_API_Error(const std::string& msg, const std::string& type)
    : message(msg), errorType(type) {}

    Explorer_API_Error::~Explorer_API_Error() {}

    void Explorer_API_Error::logError() const {
        std::cerr << errorType << " Error: " << what() << std::endl;
        // Additional logic for logging to file, sending notifications, etc.
    }

    std::string Explorer_API_Error::getErrorType() const {
        return errorType;
    }


    Explorer_API::Explorer_API()
    {
        this->base_url = "http://blockchain.mobick.info";
    }

    Explorer_API::~Explorer_API()
    {

    }

    size_t Explorer_API::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    nlohmann::json Explorer_API::request_and_jsonize(const std::string& url) {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;

        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        return nlohmann::json::parse(readBuffer);
    }

    nlohmann::json Explorer_API::get_utxo_set()
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::utxo_set);
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_address(const std::string& address)
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::address) + address;
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_next_block()
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::mining);
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_unconfirmed_tx()
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::unconfirmed_txs);
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_transaction(const std::string& txid)
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::transaction) + txid;
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_block(const int& block)
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::block) + std::to_string(block);
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_block_header(const int& block)
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::block) + std::to_string(block);
        return Explorer_API::request_and_jsonize(url);
    }

    nlohmann::json Explorer_API::get_height()
    {
        std::string url = this->base_url + Explorer_API::methodName(Explorer_API::height);
        return Explorer_API::request_and_jsonize(url);
    }

    const std::string& Explorer_API::methodName(const Explorer_API::api_list method)
    {
        static const std::string names[] = {
            "/api/blockchain/utxo-set/",
            "/api/address/",
            "/api/mining/next-block/",
            "/api/mining/next-block/txids/",
            "/api/tx/",
            "/api/block/",
            "/api/block/header/",
            "/api/blocks/tip/"
        };

        return names[method];
    }
};