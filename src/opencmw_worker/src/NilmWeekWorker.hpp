#ifndef NILM_WEEK_WORKER_H
#define NILM_WEEK_WORKER_H

#include <mutex>

#include <majordomo/Worker.hpp>

#include "DataFetcher.hpp"
#include "TimeDomainWorker.hpp"

// context for Dashboard
struct NilmWeekContext {
    opencmw::TimingCtx      ctx;
    opencmw::MIME::MimeType contentType = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(NilmWeekContext, ctx, contentType)

// data for Dashboard
struct NilmWeekData {
    int64_t            timestamp;
    std::vector<float> values;
};

ENABLE_REFLECTION_FOR(NilmWeekData, timestamp, values)

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmWeekWorker : public Worker<serviceName, NilmWeekContext, Empty, NilmWeekData, Meta...> {
public:
    using super_t = Worker<serviceName, NilmWeekContext, Empty, NilmWeekData, Meta...>;

    template<typename BrokerType>
    explicit NilmWeekWorker(const BrokerType &broker, std::chrono::milliseconds updateInterval = std::chrono::milliseconds(1000))
        : super_t(broker, {}) {
        for (unsigned int i = 0; i < NUMBER_OF_SAVED_VALUES; i++) {
            _values.push_back(0.0F);
        }
        if (!readValuesFromFile()) {
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            _lastSave                                              = now;
            if (!saveValuesToFile()) {
                std::cout << "Error writing file" << std::endl;
            }
        }
        _workThread = std::jthread([this, updateInterval] {
            std::chrono::duration<double, std::milli> predictDuration;
            while (!_shutdownRequested) {
                std::chrono::time_point timeStart = std::chrono::system_clock::now();

                try {
                    Acquisition  aq;
                    auto         response = _dataFetcherAcq.get(aq);
                    unsigned int numValues = 0;
                    if (checkResponse(aq, numValues)) {
                        processValues(aq, numValues);
                    }
                    updateSaveFile();

                } catch (const std::exception &ex) {
                    fmt::print("caught exception '{}'\n", ex.what());
                }

                predictDuration   = std::chrono::system_clock::now() - timeStart;
                auto willSleepFor = updateInterval - predictDuration;
                if (willSleepFor > 0ms) {
                    std::this_thread::sleep_for(willSleepFor);
                }
            }
        });

        super_t::setCallback([this](RequestContext &rawCtx, const NilmWeekContext &, const Empty &, NilmWeekContext &, NilmWeekData &out) {
            if (rawCtx.request.command() == Command::Get) {
                std::unique_lock<std::mutex> lock(_mutex);
                out.timestamp = _dataFetcherAcq.getLastTimestamp();
                out.values    = _values;
            }
        });
    }

    ~NilmWeekWorker() {
        _shutdownRequested = true;
        _workThread.join();
    }

private:
    void processValues(const Acquisition& aq, unsigned int numValues) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!aq.channelTimeSinceRefTrigger.empty()) {
            int64_t timeStamp = aq.refTriggerStamp + static_cast<int64_t>(aq.channelTimeSinceRefTrigger.back() * 1e9F);
            _dataFetcherAcq.setLastTimeStamp(timeStamp);
        }
        std::vector<float> vals = aq.channelValues.elements();
        float              val  = 0.0F;
        for (unsigned int i = numValues; i < 2 * numValues; i++) {
            val = vals[i] / 1000.0f; // Convert from Wh to kWh
            if (val - _values[0] < -1e-6F) {
                std::cout << "Value smaller - new val: " << val << std::endl;
                if (val < 2e-3F) { // value was resetted
                    // shift
                    _values[6] = _values[5];
                    _values[5] = _values[4];
                    _values[4] = _values[3];
                    _values[3] = _values[2];
                    _values[2] = _values[1];
                    _values[1] = _values[0];
                }
            }
            _values[0] = val;
        }
    }

    bool checkResponse(const Acquisition& aq, unsigned int &numValues) {
        if (_dataFetcherAcq.responseOk()) {
            unsigned int dim0 = aq.channelValues.dimensions()[0];
            unsigned int dim1 = aq.channelValues.dimensions()[1];
            if (!(dim0 == 0 && dim1 == 0) && dim0 != 2) { // dim0 != 2 and no empty response
                fmt::print("Invalid response - dims: {} {}", dim0, dim1);
                return false;
            }
            numValues = dim1;
            return true;
        }
        fmt::print("Invalid response - respone not ok");
        return false;
    }

    void updateSaveFile() {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        if (now - _lastSave > _saveInterval) {
            _lastSave = now;
            if (!saveValuesToFile()) {
                std::cout << "Error updating save file" << std::endl;
            }
        }
    }

    bool readValuesFromFile() {
        int64_t           lastSaveTimestamp = 0;
        const std::string filePath          = "nilm_week_values.txt";
        try {
            std::ifstream readStream;
            readStream.open(filePath, std::ifstream::in);
            if (!readStream.is_open()) {
                return false;
            }
            {
                std::unique_lock<std::mutex> lock(_mutex);
                readStream >> lastSaveTimestamp;
                _lastSave   = getTimePointFromUnixTimestamp(lastSaveTimestamp);

                float value = 0.0;
                for (unsigned int i = 0; i < NUMBER_OF_SAVED_VALUES; i++) {
                    readStream >> value;
                    _values[i] = value;
                }
            }
            readStream.close();
            return true;
        } catch (...) {
            return false;
        }
    }

    bool saveValuesToFile() {
        const std::string filePath = "nilm_week_values.txt";
        try {
            std::ofstream outputFile;
            outputFile.open(filePath, std::ofstream::out | std::ofstream::trunc);
            if (!outputFile.is_open()) {
                return false;
            }
            {
                std::unique_lock<std::mutex> lock(_mutex);
                outputFile << getUnixTimestampFromTimepoint(_lastSave) << '\n';
                for (float value : _values) {
                    outputFile << value << '\n';
                }
            }
            outputFile.close();
            return true;
        } catch (...) {
            return false;
        }
    }

    int64_t getUnixTimestampFromTimepoint(std::chrono::time_point<std::chrono::system_clock> timePoint) {
        auto timePointSeconds = std::chrono::time_point_cast<std::chrono::seconds>(timePoint);
        return timePointSeconds.time_since_epoch().count();
    }

    std::chrono::time_point<std::chrono::system_clock> getTimePointFromUnixTimestamp(int64_t timestamp) {
        std::chrono::seconds                               seconds(timestamp);
        std::chrono::time_point<std::chrono::system_clock> timePoint(
                seconds);
        return timePoint;
    }

    std::vector<float>                                 _values;
    std::mutex                                         _mutex;

    DataFetcher<Acquisition>                           _dataFetcherAcq = DataFetcher<Acquisition>("pulsed_power/Acquisition", "P_Int_Day@1Hz,S_Int_Day@1Hz");

    std::atomic<bool>                                  _shutdownRequested;
    std::jthread                                       _workThread;
    std::chrono::time_point<std::chrono::system_clock> _lastSave;
    const std::chrono::minutes                         _saveInterval = std::chrono::minutes(10);

    const unsigned int                                  NUMBER_OF_SAVED_VALUES = 7;
};

#endif /* NILM_WEEK_WORKER_H */
