#include <iostream>
#include <string>
#include <curl/curl.h>
using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string fetchStockData(const std::string& symbol) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = "https://api.twelvedata.com/time_series"
                      "?apikey=3bae178ca6d3415cbd5cf805c6a8750f"
                      "&interval=1min"
                      "&symbol=" + symbol +
                      "&timezone=exchange"
                      "&format=JSON";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code != 200) {
                std::cerr << "HTTP error: " << http_code << std::endl;
            } else {
                std::cout << "API Response for " << symbol << ": " << readBuffer << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize CURL" << std::endl;
    }

    curl_global_cleanup();
    return readBuffer;
}