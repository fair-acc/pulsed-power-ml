#ifndef NILM_PREDICT_WORKER_H
#define NILM_PREDICT_WORKER_H

#include <disruptor/RingBuffer.hpp>
#include <majordomo/Worker.hpp>

#include <gnuradio/pulsed_power/opencmw_time_sink.h>

#include <chrono>
#include <unordered_map>

#include <cppflow/cppflow.h>
#include <cppflow/ops.h>
#include <cppflow/model.h>
#include <cppflow/tensor.h>

//#define MODEL_PATH "src/model/model_example"

//#include <iostream>

using opencmw::Annotated;
using opencmw::NoUnit;


struct NilmContext {
    opencmw::TimingCtx      ctx;
    //std::string             testFilter;
    opencmw::MIME::MimeType contentType = opencmw::MIME::JSON;
};

//ENABLE_REFLECTION_FOR(NilmContext, ctx, testFilter, contentType)
ENABLE_REFLECTION_FOR(NilmContext, ctx, contentType)


struct RingBufferNilmData {
    std::vector<float> chunk;
    int64_t            timestamp;
};

struct SingalNilmTimeData{
    int64_t            timestamp;
    float              value;
};

struct SignalNilmFrequencyData{
    int64_t             timestamp;
    std::vector<float>  frequencyValues;
};

struct NilmPredictData {
    std::vector<double>  values={0.0,25.7,55.5,74.1, 89.4, 34.5,23.4,1.0, 45.4, 56.5,76.4};
   // opencmw::MultiArray<float, 2> channelValues;
};

struct ModelData {
    int64_t            timestamp;
    std::vector<float> data_point; //(196612) = 4 + 2^16 
};

// ENABLE_REFLECTION_FOR(NilmPredictData, values, channelValues)
ENABLE_REFLECTION_FOR(NilmPredictData, values)

using namespace opencmw::disruptor;
using namespace opencmw::majordomo;
template<units::basic_fixed_string serviceName, typename... Meta>
class NilmPredictWorker   
    : public Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...> {
private:
    static const size_t RING_BUFFER_NILM_SIZE = 4096;

    //const std::string   _deviceName;

    //const std::string  MODEL_PATH = "src/model/model_example";
    const std::string  MODEL_PATH = "src/model/dummy_model";

    using ringbuffer_t  = std::shared_ptr<RingBuffer<RingBufferNilmData, RING_BUFFER_NILM_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;

    using eventpoller_t = std::shared_ptr<EventPoller<RingBufferNilmData, RING_BUFFER_NILM_SIZE, BusySpinWaitStrategy, SingleThreadedStrategy>>;

    std::shared_ptr<cppflow::model> _model = std::make_shared<cppflow::model>(MODEL_PATH);
 
    struct SignalData {
        std::string   _signalName;
        std::string   _signalUnit;
        float         _sampleRate;
        ringbuffer_t  _ringBuffer;
        eventpoller_t _eventPoller;
        RingBufferNilmData _nilmData;

        SignalData(std::string signalName, float sampleRate):_signalName(signalName),_sampleRate(sampleRate),
                _ringBuffer(newRingBuffer<RingBufferNilmData, RING_BUFFER_NILM_SIZE, BusySpinWaitStrategy, ProducerType::Single>()),
                _eventPoller(_ringBuffer->newPoller()){

            _ringBuffer->addGatingSequences({ _eventPoller->sequence() });
        }
    };

    struct SignalTimeData{
        std::string     _signalName;
        std::string     _signalUnit;
        float           _sampleRate;
        SingalNilmTimeData  _signalValue;
    };

    struct SignalFrequency{
        std::string              _signalName;
        std::string              _signalUnit;
        float                    _sampleRate;
        SignalNilmFrequencyData  _frequncyValues;
        SignalFrequencyData(std::string signal_name, _sampleRate sample_rate):_signalName(_signalName),_sampleRate(sample_rate){

        }
    }

    std::unordered_map<std::string, SignalData>      _signalsMap; 
    std::unordered_map<std::string, SignalTimeData>  _signalsTimeMap; // map keys: P, Q,  S, Phi
    std::unordered_map<std::string, SignalFrequency> _signalsFrequencyMap; // map P, Q, S

    bool                  init;
    int64_t               timestamp_frq;


public:
    std::atomic<bool>     shutdownRequested;
    std::jthread          notifyThread;
    NilmPredictData       nilmData;
    std::time_t           timestamp;
   

    static constexpr auto PROPERTY_NAME = std::string_view("NilmPredicData");

    using super_t = Worker<serviceName, NilmContext, Empty, NilmPredictData, Meta...>;
    

    template<typename BrokerType>
    explicit NilmPredictWorker(const BrokerType &broker,
            std::chrono::milliseconds        updateInterval)
        : super_t(broker, {}),init(false) {
    
        notifyThread = std::jthread([this, updateInterval] {
            std::chrono::duration<double, std::milli> pollingDuration;
            while (!shutdownRequested) {

                std::chrono::time_point time_start = std::chrono::system_clock::now();

                timestamp = std::time(nullptr);

                // static size_t           subs       = 0;
                // if (subs != super_t::activeSubscriptions().size()) {
                //     subs = super_t::activeSubscriptions().size();
                //     fmt::print("NilmPredicWorker: activeSubs: {}\n", subs);
                // }


                // TODO - read values

                for(const auto &kv : _signalsMap ){
                    fmt::print("Sginal name from map {}\n", kv.first);
                    fmt::print("Sginal timestamp {}\n", kv.second._nilmData.timestamp);
                    fmt::print("Sitnal chunk {}\n", kv.second._nilmData.chunk);

                    fmt::print("Sginal rate {}\n", kv.second._sampleRate);
                }
               
                NilmContext context;

                context.contentType = opencmw::MIME::JSON;

                auto input = cppflow::fill({196612, 1}, 2.0f);
                auto output = (*_model)(input);
                
                auto values = output.get_data<float>();

                nilmData.values.clear();

                for (auto v : values) {
                    nilmData.values.push_back(static_cast<double>(v));
                }

                fmt::print("Output: {}\n", output);

                // TODO - fill nilmData.values
              
                // // generierte dummy Werte
                // if (counter == 5){
                //     counter = 0;
                //     for (std::size_t i=0;i<11;i++){
                //         double value_new = nilmData.values.at(i) + 25.5;
                //         nilmData.values.at(i) = value_new<=100?value_new :  (value_new -100.0);
                //     }
                //     nilmData.values.at(zero_index) = 0.0;
                //     zero_index = zero_index == 10? 0: (zero_index +1);
                // }
                // counter++;

                super_t::notify("/nilmPredictData", context, nilmData);
            
                pollingDuration   = std::chrono::system_clock::now() - time_start;

                auto willSleepFor = updateInterval - pollingDuration;
                if (willSleepFor>0){
                    std::this_thread::sleep_for(willSleepFor);
                } else {
                    fmt::print("Data prediction too slow\n");
                }  

                fmt::print("Sample data sieze: {}\n", _signalsMap.size());
                std::this_thread::sleep_for(1000ms); 

                std::this_thread::sleep_for(willSleepFor);        
            }

        });

        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);  // in call back machen 
        fmt::print("GR: number of time-domain sinks found: {}\n", gr::pulsed_power::globalTimeSinksRegistry.size());

        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {

            // nur für bestimmte Singal 
            const auto signal_name = sink->get_signal_name();
            const auto signal_unit = sink->get_signal_unit();
            const auto sample_rate = sink->get_sample_rate();

            fmt::print("Name {}, unit {}, rate {}", signal_name, signal_unit, sample_rate);
            _signalsMap.insert({signal_name, SignalData(signal_name, sample_rate)});
                                        
            // fmt::print("GR: OpenCMW Time Sink '{}' added\n", completeSignalName);
            sink->set_callback(std::bind(&NilmPredictWorker::callbackCopySinkData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

        }

        // TODO  -Im Progress
        // for (auto sink : gr::pulsed_power::frequencySink) {
            // const auto signal_name = sink->get_signal_name();
            // const auto signal_unit = sink->get_signal_unit();
            // const auto sample_rate = sink->get_sample_rate();

            // fmt::print("Name {}, unit {}, rate {}", signal_name, signal_unit, sample_rate);
            // _signalsFrequencyMap.insert({signal_name, SignalFrequencyData(signal_name, sample_rate)});
            // sink->set_callback(std::bind(&NilmPredictWorker::callbackCopySinkFrequencyData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));                 
        // }

        super_t::setCallback([this](RequestContext &rawCtx, const NilmContext &,
                                     const Empty &, NilmContext &,
                                     NilmPredictData &out) {
            using namespace opencmw;
            const auto topicPath = URI<RELAXED>(std::string(rawCtx.request.topic()))
                                           .path()
                                           .value_or("");
            out     = nilmData;

        });

       
    }

    ~NilmPredictWorker() {
        shutdownRequested = true;
        notifyThread.join();
    }


    // data - zeiger auf erste , data_size - große - index operator /// data von gnu radio
     void callbackCopySinkData(const float *data, int data_size, const std::string &signal_name, float sample_rate, int64_t timestamp_ns) {
        if (sample_rate > 0) {
            // realign timestamp
            timestamp_ns -= static_cast<int64_t>(1 / sample_rate * 1e9F) * data_size;
        }

        // write into RingBuffer
        if (_signalsMap.contains(signal_name)) {
            //const 
            SignalData &signalData = _signalsMap.at(signal_name);

            //fmt::print("Signal {}\n", signal_name);
            //fmt::print("data {}\n", data[0]);

            /// schrieben in Fiel in daten
            // std::vector<float> chunk;
            // int64_t            timestamp;

            // RingBufferNilmData nilmData;
            // nilmData.timestamp = timestamp_ns;
            // nilmData.chunk.assign(data, data + data_size); 

            // fmt::print("NilmData: {},  {}\n", nilmData.timestamp, nilmData.chunk);
            // signalData._nilmData=nilmData;


            signalData._nilmData.timestamp = timestamp_ns;
            signalData._nilmData.chunk.assign(data, data + data_size);



            // publish data // data speicher - kann es in 
            // TODO anpassen - für PQSPHI

            bool result = signalData._ringBuffer->tryPublishEvent([&data, data_size, timestamp_ns](RingBufferNilmData &&bufferData, std::int64_t /*sequence*/) noexcept {
                bufferData.timestamp = timestamp_ns;
                bufferData.chunk.assign(data, data + data_size);
            });

            if (!result) {
                // fmt::print("error writing into RingBuffer, signal_name: {}\n", signal_name);
            }
        }
    }

    void callbackCopySinkFrequencyData(const float *data, int data_size, const std::string &signal_name, float sample_rate, int64_t timestamp_ns){
        // TODO
        // copy data to S 
        if (sample_rate > 0) {
            // realign timestamp
            timestamp_ns -= static_cast<int64_t>(1 / sample_rate * 1e9F) * data_size;
        }

        if (_signalsFrequencyMap.contains(signal_name)) {
            // TODO lock
            SignalFreuncyData &signalData = _signalsFrequencyMap.at(signal_name);

            signalData._frequncyValues.timestamp=timestamp_ns;
            signalData._frequncyValues.frequencyValues.clear();
            signalData._frequncyValues.frequencyValues.assing(data, data+data_size);
        }        
    }

private:

    

    // std::vector<double> mergeValues(std::vector<double> &timeData, std:: vector<double> &frequencyData){
       
    //     std::vector<double> output;
   
    //     if (timeData.size()!=3 || frequencyData.size() != 3){
    //         return output;
    //     } else {
    //         output.insert(output.end(),timeData.begin(), timeData.end());
    //         output.insert(output.end(), frequencyData.begin(), frequencyData.end());
    //         return output;
    //     }
    // }

    // const std::string getCompleteSignalName(const std::string &signalName, float sampleRate) const {
    //     return fmt::format("{}{}{}@{}Hz", _deviceName, _deviceName.empty() ? "" : ":", signalName, sampleRate);
       
    // }

    //  void respondWithEmptyResponse(const TimeDomainContext &filter, const std::string_view errorText) {
    //     fmt::print("{}\n", errorText);
    //     super_t::notify("/nilmPredictData", filter, nilmPredictData());
    // }

};

#endif /* NILM_PREDICT_WORKER_H */