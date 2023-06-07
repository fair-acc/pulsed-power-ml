#ifndef DATA_FETCHER_H
#define DATA_FETCHER_H

#include <httplib.h>
#include <IoSerialiser.hpp>
#include <IoBuffer.hpp>


template<typename Acq>
class DataFetcher {
    std::string     _endpoint;
    std::string     _signalNames;
    int64_t         _lastTimeStamp;
    httplib::Client _http;
    bool            _responseOk;

public:
    DataFetcher() = delete;
    explicit DataFetcher(const std::string &endPoint, const std::string &signalNames = "")
        : _endpoint(endPoint), _signalNames(signalNames), _lastTimeStamp(0), _http("localhost", DEFAULT_REST_PORT), _responseOk(false) {
        _http.set_keep_alive(true);
    }
    ~DataFetcher() = default;
    httplib::Result get(Acq &data) {
        const std::string getPath = fmt::format("{}?channelNameFilter={}&lastRefTrigger={}", _endpoint, _signalNames, _lastTimeStamp);
        httplib::Result response = _http.Get(getPath);
        if (response.error() == httplib::Error::Success && response->status == 200) {
            _responseOk = true;
            opencmw::IoBuffer buffer;
            buffer.put<opencmw::IoBuffer::MetaInfo::WITHOUT>(response->body);
            auto result = opencmw::deserialise<opencmw::Json, opencmw::ProtocolCheck::LENIENT>(buffer, data);
        } else {
            _responseOk = false;
        }

        return response;
    }

    bool responseOk() {
        return _responseOk;
    }

    void setLastTimeStamp(int64_t timestamp) {
        _lastTimeStamp = timestamp;
    }

    int64_t getLastTimestamp() {
        return _lastTimeStamp;
    }
};

#endif /* DATA_FETCHER_H */