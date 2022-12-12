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

using opencmw::Annotated;
using opencmw::NoUnit;

// context for Dashboard
struct NilmContext {
    opencmw::TimingCtx      ctx;
    opencmw::MIME::MimeType contentType = opencmw::MIME::JSON;
};

ENABLE_REFLECTION_FOR(NilmContext, ctx, contentType)

// data for Dashboard
struct NilmPredictData {
    std::vector<double>  values={0.0,25.7,55.5,74.1, 89.4, 34.5,23.4,1.0, 45.4, 56.5,76.4};
};

// current structers fro sinks
struct PQSPhiDataSink{
    int64_t            timestamp;
    float              p;
    float              q;
    float              s;
    float              phi;
};

struct SUIDataSink{
    int64_t            timestamp;
    std::vector<float> s;
    std::vector<float> u;
    std::vector<float> i;
};



// TODO - remove
// data from time sink
struct PQSPHiData{
    int64_t            timestamp;
    std::vector<float> pqsphiValues; 

    // float p;
    // float q;
    // float s;
    // float phi;
};

// data from frequency sink
struct SUIData{
    int64_t                         timestamp;
    std::vector<std::vector<float>> suiValues; // three vectors - S U I - len 65538
};


struct RingBufferNilmData {
    std::vector<float> chunk;
    int64_t            timestamp;
};

// input data for model
struct ModelData {
    int64_t            timestamp;
    std::vector<float> data_point; //(196612) = 4 + 2^16 
};

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
 
    // remove - Test only
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
        std::string         _signalName;
        std::string         _signalUnit;
        float               _sampleRate;
        PQSPHiData          _pqsphiData;
    };

    struct SignalFrequencyData{
        std::string              _signalName;
        std::string              _signalUnit;
        float                    _sampleRate;
        SUIData                  _suiData;
    };

    std::unordered_map<std::string, SignalData>      _signalsMap; // remove - test only
    // std::unordered_map<std::string, SignalTimeData>  _signalsTimeMap; // map keys: P, Q,  S, Phi
    //std::unordered_map<std::string, SignalFrequency> _signalsFrequencyMap; // map S U I

    // TODO -remove
    std::shared_ptr<SignalFrequencyData> _suiSignalData = std::make_shared<SignalFrequencyData>(); // P, Q,  S, Phi data
    std::shared_ptr<SignalTimeData>   _pqsphiSignalData = std::make_shared<SignalTimeData>();      // S U I data

    
    std::shared_ptr<PQSPhiDataSink>   _pqsphiDataSink = std::make_shared<PQSPhiDataSink>();
    std::shared_ptr<SUIDataSink>         _suiDataSink = std::make_shared<SUIDataSink>();


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
        : super_t(broker, {}){
    
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
               
                NilmContext context;

                //TODO merge values

                // predict
                auto input = cppflow::fill({196612, 1}, 2.0f); // replace with merge - test only
                auto output = (*_model)(input);
                
                auto values = output.get_data<float>();

                nilmData.values.clear();


                // fill data for REST
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

                // TODO - check this - warning
                auto willSleepFor = updateInterval - pollingDuration;
                // if (willSleepFor>0){
                //     std::this_thread::sleep_for(willSleepFor);
                // } else {
                //     fmt::print("Data prediction too slow\n");
                // }  

                fmt::print("Sample data sieze: {}\n", _signalsMap.size());
                std::this_thread::sleep_for(1000ms); 

                std::this_thread::sleep_for(willSleepFor);        
            }

        });

        // // Test PQST
        // (*_pqsphiSignalData)._pqsphiData.pqsphiValues = std::vector<float>({.1f,.2f,.3f,.4f});
        // (*_pqsphiSignalData)._pqsphiData.timestamp = 111;

        // fmt::print("P Q S Phi: {}\n", (*_pqsphiSignalData)._pqsphiData.pqsphiValues);

        // // Test S U I
        // (*_suiSignalData)._suiData.suiValues.push_back(generateVector(.01)); 
        // (*_suiSignalData)._suiData.suiValues.push_back(generateVector(.2));
        // (*_suiSignalData)._suiData.suiValues.push_back(generateVector(.3));
        // fmt::print("S U I size: {}\n", (*_suiSignalData)._suiData.suiValues.size());
        // //fmt::print("S U I size: {}\n", (*_suiSignalData)._suiData.suiValues.at(0));

        std::scoped_lock lock(gr::pulsed_power::globalTimeSinksRegistryMutex);  // in call back machen 
        fmt::print("GR: number of time-domain sinks found: {}\n", gr::pulsed_power::globalTimeSinksRegistry.size());


        // // Test Merge TODO - modify
        // std::vector<float> data_point = mergeValues();
        // fmt::print("Test merge {}\n", data_point);

        std::vector<std::string> pqsohi_str{"P", "Q", "S", "Phi"};
        // from sink take only P Q S Phi Signal 
        for (auto sink : gr::pulsed_power::globalTimeSinksRegistry) {

            const auto signal_names = sink->get_signal_names();
            const auto signal_units = sink->get_signal_units();
            const auto sample_rate = sink->get_sample_rate();

            fmt::print("Name {}, unit {}, rate {}", signal_names, signal_units, sample_rate);

            
            if (signal_names == pqsohi_str){
                                        
                sink->set_callback(std::bind(&NilmPredictWorker::callbackCopySinkTimeData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));           
            }
        }

        std::vector<std::string> sui_str{"P", "Q", "S", "Phi"};
        for (auto sink : gr::pulsed_power::globalFrequencySinksRegistry) {
            const auto signal_names = sink->get_signal_names();
            const auto signal_units = sink->get_signal_units();
            const auto sample_rate = sink->get_sample_rate();

            if (signal_names == sui_str){

                fmt::print("Name {}, unit {}, rate {}", signal_names, signal_units, sample_rate);
                //_signalsFrequencyMap.insert({signal_name, SignalFrequencyData(signal_name, sample_rate)});
                sink->set_callback(std::bind(&NilmPredictWorker::callbackCopySinkFrequencyData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));                 
            }
        }

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
    
    // copy P Q S Phi data
    void callbackCopySinkTimeData(std::vector<const void *> &input_items, int &noutput_items, const std::vector<std::string> &signal_names, float /* sample_rate */, int64_t timestamp_ns) { 
        // data save into SignalTimeData
        std::vector<std::string> pqsphi_str{"P", "Q", "S", "Phi"};
        if (signal_names == pqsphi_str){
            _pqsphiDataSink->timestamp = timestamp_ns;
            //TODO write values
            _pqsphiDataSink->p = 0.5; // dummy value
            _pqsphiDataSink->q = 0.7;  // dummy value
            _pqsphiDataSink->s = 0.25; // dummy value
        }
    }

    // copy S U I data
    void callbackCopySinkFrequencyData(std::vector<const void *> &input_items, int &nitems, size_t vector_size, const std::vector<std::string> &signal_name, float sample_rate, int64_t timestamp) {

        const float *in                 = static_cast<const float *>(input_items[0]);

        std::vector<std::string> sui_str{"P", "Q", "S", "Phi"};
        if (signal_name == sui_str){
            _suiDataSink->timestamp = timestamp;
            // dummy velues TODO - assign 
            _suiDataSink->s = generateVector(.01);
            _suiDataSink->u = generateVector(.02);
            _suiDataSink->i = generateVector(.03);

        }
    }

private:

    // TODO - use simple structures
    // return data_point - input for model
    std::vector<float> mergeValues(){

        // TODO lock
        PQSPHiData &pqsphiData = (*_pqsphiSignalData)._pqsphiData;
        SUIData    &suiData    = (*_suiSignalData)._suiData;

        // TODOcheck 
        std::vector<float> output;
        if(pqsphiData.pqsphiValues.size() != 4){

            fmt::print("Error: incolmplete P Q S Phi signals {}\n", pqsphiData.pqsphiValues.size());
            
            return output;
        }

        if(suiData.suiValues.size()!= 3)
        {
            fmt::print("Error: incomplate S U I vetors {}\n", suiData.suiValues.size());
            return output;
        }

        int counter = 0;
        for(auto suiVector: suiData.suiValues){
            if(suiVector.size()!= 65536){
                fmt::print("Error: incorrect sieze {} of vector {} ", suiVector.size(), counter);
                output.clear();
                return output;
            }

            output.insert(output.end(),suiVector.begin(), suiVector.end());
            counter++;
        }

        output.insert(output.end(), pqsphiData.pqsphiValues.begin(), pqsphiData.pqsphiValues.end());
        return output;
    }

    // Test - remove
    std::vector<float> generateVector(float init_f){

        std::vector<float> v;
        for(int i=0; i< 65536;i++){
            v.push_back(i*init_f);
        }

        return v;
    }

};

#endif /* NILM_PREDICT_WORKER_H */