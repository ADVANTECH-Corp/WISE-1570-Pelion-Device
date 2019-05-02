# WISE-1570-Pelion-Device

## Introduction

This example is demonstrated for connecting to the Pelion IoT Platform service on WISE-1570.


## Deployment

Please follow the below steps to setup for development:

1. `git clone https://github.com/ADVANTECH-Corp/WISE-1570-Pelion-Device.git`

1. `cd WISE-1570-Pelion-Device`

1. `mbed config root .`

1. `mbed deploy`

1. `patch -p1 < patch/*`

1. `cp /the/path/to/your/developer/certificate/mbed_cloud_dev_credentials.c ./`

## Compilation

Go into WISE-1570-Pelion-Device directory and run the below script to compile the example.

`./compile.sh -c`
