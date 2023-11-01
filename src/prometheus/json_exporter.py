from prometheus_client import start_http_server, Metric, REGISTRY
import json
import requests
import sys
import time


class JsonCollector(object):
  def __init__(self, endpoint):
    self._endpoint = endpoint
  def collect(self):
    # Fetch the JSON
    response = json.loads(requests.get(self._endpoint).content.decode('UTF-8'))
    metric = Metric('fair_acquisition_signal', 'single sinus signal example', 'gauge')
    valuesArray = response['Acquisition']['channelValues']['values']
    refTriggerStamp = response['Acquisition']['refTriggerStamp']
    timestampArray = response['Acquisition']['channelTimeSinceRefTrigger']

    

    signal = ["sinus", "saw", "square"]
    counterSignal = 0
    counterValues = 0
    while counterValues < len(valuesArray):
      counterTime = 0
      while((counterTime < len(timestampArray)) and (counterValues < len(valuesArray)) and (counterSignal < len(signal))):
        timestampV = float(refTriggerStamp/ 1e9 + timestampArray[counterTime])
        metric.add_sample('fair_acquisition_signal', value=valuesArray[counterValues], timestamp=timestampV, labels={'signal' : signal[counterSignal]})
        counterTime += 1
        counterValues += 1
      counterSignal += 1
    
    #for sample in metric.samples:
    #  print(sample)
    #print(str(len(metric.samples)))
    
    yield metric

if __name__ == '__main__':
  # Usage: json_exporter.py port endpoint
  start_http_server(int(sys.argv[1]))
  REGISTRY.register(JsonCollector(sys.argv[2]))

  while True: time.sleep(1)
