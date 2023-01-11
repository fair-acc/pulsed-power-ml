# Prometheus

##Getting Started
1. Install Prometheus from https://prometheus.io/

##Prometheus
How to Run
- Run `./prometheus --config.file=/path/to/config/prometheus.yml`

##JSON Exporter
How to Run 
- `python3 -m http.server 8000`
- `python3 json_exporter.py 1234 http://localhost:8000/data.json`
