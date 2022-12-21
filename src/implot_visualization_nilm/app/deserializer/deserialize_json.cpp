#include <cmath>
#include <deserialize_json.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

constexpr DataPoint::DataPoint()
    : x(0.0f), y(0.0f) {}
constexpr DataPoint::DataPoint(double _x, double _y)
    : x(_x), y(_y) {}

Buffer::Buffer(int max_size) {
    this->data.reserve(max_size);
    this->data.reserve(max_size);
}

void Buffer::assign(const std::vector<double> &x, const std::vector<double> &y) {
    if (this->data.size() > 0) {
        this->data.shrink(0);
    }
    for (int i = 0; i < x.size(); i++) {
        this->data.push_back(DataPoint(x[i], y[i]));
    }
}

ScrollingBuffer::ScrollingBuffer(int max_size) {
    this->maxSize = max_size;
    this->offset  = 0;
    this->data.reserve(this->maxSize);
}

void ScrollingBuffer::addPoint(double x, double y) {
    if (this->data.size() < this->maxSize)
        this->data.push_back(DataPoint(x, y));
    else {
        this->data[this->offset] = DataPoint(x, y);
        this->offset             = (this->offset + 1) % this->maxSize;
    }
}

void ScrollingBuffer::erase() {
    if (this->data.size() > 0) {
        this->data.shrink(0);
        this->offset = 0;
    }
}

// class BufferPower {
//     std::vector<double>    values;
//     double                 timestamp;

//     void addValues(vector<double> values);
// }


void BufferPower::updateValues(const std::vector<double> &_values){
    this->values = _values;
    auto   clock       = std::chrono::system_clock::now();
    //double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    this->timestamp = currentTime;
    init = true;
}


Acquisition::Acquisition() {}

Acquisition::Acquisition(int _numSignals) {
    std::vector<ScrollingBuffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

void Acquisition::addToBuffers() {
    double absoluteTimestamp = 0.0;
    double value             = 0.0;

    // Destride array
    for (int i = 0; i < signalNames.size(); i++) {
        this->buffers[i].signalName = this->signalNames[i];
        int stride                  = this->strideArray.dims[1];
        int offset                  = i * stride;

        for (int j = 0; j < stride; j++) {
            absoluteTimestamp = this->refTrigger_s + this->relativeTimestamps[j];
            value             = this->strideArray.values[offset + j];
            this->buffers[i].addPoint(absoluteTimestamp, value);
        }
    }
}

void Acquisition::deserialize() {

    std::string modifiedJsonString = this->jsonString;
    
    if (this->jsonString.substr(0, 14) == "\"Acquisition\":") {
        modifiedJsonString.erase(0, 14);
    }

    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            if (element.value() == 0) {
                return;
            }
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelTimeSinceRefTrigger") {
            this->relativeTimestamps.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelNames") {
            this->signalNames.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelValues") {
            this->strideArray.dims   = std::vector<int>(element.value()["dims"]);
            this->strideArray.values = std::vector<double>(element.value()["values"]);
        }
    }

    this->lastRefTrigger = this->refTrigger_ns;
    this->lastTimeStamp  = this->lastRefTrigger + relativeTimestamps.back() * 1e9;
    addToBuffers();
    this->init = true;
    this->success = true;

}

AcquisitionSpectra::AcquisitionSpectra() {}

AcquisitionSpectra::AcquisitionSpectra(int _numSignals) {
    std::vector<Buffer> _buffers(_numSignals);
    this->buffers = _buffers;
}

void AcquisitionSpectra::addToBuffers() {
    this->buffers[0].signalName = this->signalName;
    this->buffers[0].assign(this->channelFrequencyValues, this->channelMagnitudeValues);
}

void AcquisitionSpectra::deserialize() {
    std::string modifiedJsonString = this->jsonString;
    
    if (this->jsonString.substr(0, 21) != "\"AcquisitionSpectra\":") {
        modifiedJsonString.erase(0, 21);  

    }
      
    auto json_obj = json::parse(modifiedJsonString);
    for (auto &element : json_obj.items()) {
        if (element.key() == "refTriggerStamp") {
            this->refTrigger_ns = element.value();
            this->refTrigger_s  = this->refTrigger_ns / std::pow(10, 9);
        } else if (element.key() == "channelName") {
            this->signalName = element.value();
        } else if (element.key() == "channelMagnitudeValues") {
            this->channelMagnitudeValues.assign(element.value().begin(), element.value().end());
        } else if (element.key() == "channelFrequencyValues") {
            this->channelFrequencyValues.assign(element.value().begin(), element.value().end());
        }
    }
     
    //this->lastRefTrigger = this->refTrigger_ns;
    lastTimeStamp        = lastRefTrigger;
    this->lastRefTrigger = this->refTrigger_ns;
    addToBuffers();
}

// Power Usage class
PowerUsage::PowerUsage(){}
PowerUsage::PowerUsage(int _numSignals){
    std::vector<Buffer> _buffers(_numSignals);
    this->buffers = _buffers;
}


void PowerUsage::deserialize(){

   /*  if (this->jsonString.substr(0, 16) != "\"NilmPowerData\":" && this->jsonString.substr(0,18) != "\"NilmPredictData\":") { 
        // TODO thorw Exception
        return;
    } */

    std::string modifiedJsonString = this->jsonString;

    if (this->jsonString.substr(0, 16) == "\"NilmPowerData\":" ){
        modifiedJsonString.erase(0, 16); 
    } else if (this->jsonString.substr(0,18) == "\"NilmPredictData\":"){
        modifiedJsonString.erase(0, 18); 
    }

    auto json_obj = json::parse(modifiedJsonString);
    for( auto &element : json_obj.items()){
        if (element.key() == "values"){        
            this->powerUsages.assign(element.value().begin(), element.value().end());
        }
        if (element.key() == "names"){
            this->devices.clear();
            this->devices.assign(element.value().begin(), element.value().end());
        }

        if (element.key() == "timestamp")
        {
            timestamp = element.value();
        }

        if (element.key() == "day_usage"){
            this->powerUsagesDay.clear();
            this->powerUsagesDay.assign(element.value().begin(), element.value().end());
        }

        if (element.key() == "week_usage"){
            this->powerUsagesWeek.clear();
            this->powerUsagesWeek.assign(element.value().begin(), element.value().end());
        }

        if (element.key() == "month_usage"){
            this->powerUsagesMonth.clear();
            this->powerUsagesMonth.assign(element.value().begin(), element.value().end());
        }

       // TODO - Buffer if needed
       // addToBuffers();
    }

    // check values

    // dummy data TODO - deserialize in for , when structure is known
    // this->powerUsagesDay = {100.0,323.34,234.33, 500.55, 100.0,323.34,234.33, 500.55, 100.0,323.34,234.33 ,234};
    // this->powerUsagesWeek = {700.0,2123.34,1434.33, 3500.55,700.0,2123.34,1434.33, 3500.55,700.0,2123.34,1434.33,938};
    // this->powerUsagesMonth = {1500.232, 3000.99, 2599.34, 1200.89, 1500.232, 3000.99, 2599.34, 1200.89,1500.232, 3000.99, 2599.34 ,1893};

    this->setSumOfUsageDay();
    this->setSumOfUsageWeek();
    this->setSumOfUsageMonth();

    this->success = true;
    this->init    = true;
    auto   clock       = std::chrono::system_clock::now();
    double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(clock.time_since_epoch()).count());
    //double currentTime = (std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch()).count()) / 1000.0;
    this->deliveryTime = currentTime;

}

void PowerUsage::fail(){
    this->success = false;
}

double PowerUsage::sumOfUsage(){

    if (this->init){
        double sum_of_usage = 0.0;
        for( std::vector<double>::iterator it =this->powerUsages.begin();it!=this->powerUsages.end();++it){
                    sum_of_usage += *it;
        }
        return sum_of_usage;

    }else{
        return 0.0;
    }
}

void PowerUsage::setSumOfUsageDay(){
    if (this->init){
        double sum_of_usage = 0.0;

        for( std::vector<double>::iterator it =this->powerUsagesDay.begin();it!=this->powerUsagesDay.end();++it){
            sum_of_usage += *it;
        }
        this->kWhUsedDay =  sum_of_usage;
    }
}

void PowerUsage::setSumOfUsageWeek(){
    double sum_of_usage = 0.0;

    if (this->init){
         for( std::vector<double>::iterator it =this->powerUsagesWeek.begin();it!=this->powerUsagesWeek.end();++it){
            sum_of_usage += *it;
        }
        this->kWhUsedWeek =  sum_of_usage;
    }
}

void PowerUsage::setSumOfUsageMonth(){
    double sum_of_usage = 0.0;

    if (this->init){
        for( std::vector<double>::iterator it =this->powerUsagesMonth.begin();it!=this->powerUsagesMonth.end();++it){
            sum_of_usage += *it;
        }
        this->kWhUsedMonth =  sum_of_usage;
    }
}

