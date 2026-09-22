pio := "pio"
environment := "m5stick-cplus2"

default:
    @just --list

build:
    {{pio}} run -e {{environment}}

upload:
    {{pio}} run -e {{environment}} --target upload

monitor:
    {{pio}} device monitor --baud 115200

upload-monitor: upload
    {{pio}} device monitor --baud 115200

devices:
    {{pio}} device list

clean:
    {{pio}} run -e {{environment}} --target clean

