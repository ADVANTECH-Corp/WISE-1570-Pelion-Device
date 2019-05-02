// ----------------------------------------------------------------------------
// Copyright 2016-2018 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#ifndef MBED_TEST_MODE

#include "mbed.h"
#include "simple-mbed-cloud-client.h"
#include "LittleFileSystem.h"

#if MBED_MEM_TRACING_ENABLED
#include "mbed_mem_trace.h"
#endif

#if defined(SENSOR_HDC1050)
#include "hdc1050.h"
#endif

#if defined(WISE_1570_CONSOLE)
#include "console.h"
#include "cmd_set.h"
#endif
#include "config_api.h"

#include "CellularInformation.h"
#include "CellularDevice.h"
#include "CellularNetwork.h"

#define SW_VERSION "1570S00MMX010001"

//#define OTA_TEST 1
#define MS_SCHEDULE_TIME 30
#define SECOND_UPDATE_SENSOR_TIME 30
#define MINUTE_UPDATE_REGISTRATION 1
#if defined(OTA_TEST)
#define DISPLAY_TITLE "PDMC (OTA)"
#else
#define DISPLAY_TITLE "PDMC"
#endif

// Default block device
BlockDevice *bd = BlockDevice::get_default_instance();
LittleFileSystem fs(((const char*)PAL_FS_MOUNT_POINT_PRIMARY+1));
StorageHelper sh(bd, &fs);

// Default network interface object
NetworkInterface *net = NetworkInterface::get_default_instance();
CellularDevice *dev = CellularDevice::get_default_instance();
CellularNetwork *network;
CellularInformation *modem_info;


// Declaring pointers for access to Pelion Device Management Client resources outside of main()
MbedCloudClientResource *gpio_res;
#if defined(SENSOR_HDC1050)
MbedCloudClientResource *temperature_res;
MbedCloudClientResource *humidity_res;
static DigitalOut g_tPower(CB_PWR_ON);
static DigitalInOut g_tGpio(GPIO0);
#endif
static DigitalIn g_tBackUp(BACKUP);
static int g_iSubscribeTemperature = 0;
static int g_iSubscribeHumidity = 0;
static struct mcc_resource_def g_ResourceData;

/**
 * Registration callback handler
 * @param endpoint Information about the registered endpoint such as the name (so you can find it back in portal)
 */
void registered(const ConnectorClientEndpointInfo *endpoint) 
{
    printf("Connected to Pelion Device Management. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
}

void gpio_updated(MbedCloudClientResource *resource, m2m::String newValue) 
{
    char cCh;

    MSG_print("GPIO PUT received, new value: %s\n\r", newValue.c_str());
  
    cCh = *((char*)newValue.c_str());
    if( cCh == '0') {
        MSG_print("===> Set GPIO to 0 \n\r");
        g_tGpio = 0;
    }
    else if (cCh == '1') {
        MSG_print("===> Set GPIO to 1 \n\r");
        g_tGpio = 1;
    }
  
}

void vShowModemInfo(void)
{
    char rsp_buf[500];
    int rsp_buf_len = 500;
    
    if (modem_info) {
        modem_info->get_revision(rsp_buf, (rsp_buf_len -1));
        MSG_print("modem_info:\n\r");
        MSG_print(rsp_buf);
    }
    else {
        MSG_print("modem_info is NULL !!!!\n\r");
    }

    MSG_print("\n\r");
}

void vShowRssi(void)
{
    int rs = -1;
    int ber = -1;

    if(network) {
        network->get_signal_quality(rs, ber);
        MSG_print("===> rssi:%d\n\r", rs);
    }
    else {
        MSG_print("network is NULL !!!!\n\r");
    }

    MSG_print("\n\r");
}

int iSetCredential(void)
{
    int res;
    CellularBase *cellular_iface;
    char strBuf[KV_MAX_BUFFER_LENGTH] = {0};

    res = Config_iGetApn(strBuf);
    if (res != 0) {
        MSG_print("Not found APN, res: %d\n\r", res);
    }
    else {
        MSG_print("Set APN: %s\n\r", strBuf);
        cellular_iface = reinterpret_cast<CellularBase*>(net);
        cellular_iface->set_credentials(strBuf, NULL, NULL);
    }

    return 0;
}

void notification_status(MbedCloudClientResource *resource, const NoticationDeliveryStatus status)
{
    int *piSubscribe = NULL;
    const char *strResourceName = NULL;

    resource->get_data(&g_ResourceData);
    strResourceName = g_ResourceData.name.c_str();
    printf("===> [Resource] %s\n", strResourceName);
    if(strcmp(strResourceName, "temperature") == 0) {
         piSubscribe = &g_iSubscribeTemperature;
    }
    else if(strcmp(strResourceName, "humidity") == 0) {
         piSubscribe = &g_iSubscribeHumidity;
    }
    
    switch(status) {
        case NOTIFICATION_STATUS_BUILD_ERROR:
            printf("===> [Notify] Error when creating a notification message. Can happen if running out of memory.\n");
            break;
        case NOTIFICATION_STATUS_RESEND_QUEUE_FULL:
            printf("===> [Notify] Cannot create a message because the CoAP resend queue is full.\n");
            break;
        case NOTIFICATION_STATUS_SENT:
            printf("===> [Notify] Notification message created and sent to the server.\n");
            break;
        case NOTIFICATION_STATUS_DELIVERED:
            printf("===> [Notify] Notification message delivered to the server.\n");
            break;
        case NOTIFICATION_STATUS_SEND_FAILED:
            printf("===> [Notify] Cannot send a notification message because there are no more CoAP retry attempts left.\n");
            break;
        case NOTIFICATION_STATUS_SUBSCRIBED:
            printf("===> [Notify] Observation has started.");
            if(piSubscribe != NULL) {
                *piSubscribe = 1;
                printf(" subscribed:%d\n", *piSubscribe);
            }
            printf("\n");
            break;
        case NOTIFICATION_STATUS_UNSUBSCRIBED:
            printf("===> [Notify] Observation removed.");
            if(piSubscribe != NULL) {
                *piSubscribe = 0;
                printf(" subscribed:%d\n", *piSubscribe);
            }
            printf("\n");
            break;
        default:
            printf("===> [Notify] Unknown status.\n");
            break;
    }
}


void vPdmcThread(void) 
{
    nsapi_error_t status;
    int client_status;
    unsigned int count = 0;
    unsigned int uiMod, uiRegUpdateMod;
#if defined(SENSOR_HDC1050)
    float fTemperature = 0;
    uint16_t u16Humidity = 0;
#endif

    //
    // Initiation for all we need
    //
    g_tPower = 1;
    g_tGpio = 0;
    iSetCredential();
#if defined(SENSOR_HDC1050)
    HDC1050_Init();
#endif       

    printf("Starting Simple Pelion Device Management Client example\n");
    printf("Connecting to the network...\n");

    //
    // Connect to the internet (DHCP is expected to be on)
    //
    status = net->connect();
    if (status != NSAPI_ERROR_OK) {
        printf("Connecting to the network failed %d!\n", status);
        return;
    }

    printf("Connected to the network successfully. IP address: %s\n", net->get_ip_address());

    //
    // Get some information about rssi and modem version
    //
    network = dev->open_network();
    if(!network) {
        printf("Open the network for getting information failed!\n");
    }
    else {
        for(int i = 0; i < 5; i++) {
            vShowRssi();
            wait_ms(200);
        }
    }

    modem_info = dev->open_information();
    if(!modem_info) {
        printf("Open the information for getting modem version failed!\n");
    }
    else {
        vShowModemInfo();
    }

    //
    // Call StorageHelper to initialize FS for us
    //
    printf("PAL_FS_MOUNT_POINT_PRIMARY = %s\n", PAL_FS_MOUNT_POINT_PRIMARY);
    sh.init();

    //
    // SimpleMbedCloudClient handles registering over LwM2M to Pelion Device Management
    //
    SimpleMbedCloudClient client(net, bd, &fs);
    client_status = client.init();
    if (client_status != 0) {
        printf("Pelion Client initialization failed (%d)\n", client_status);
        return;
    }

    //
    // Create resources for gpio, temperature and humidity
    //
    gpio_res = client.create_resource("3201/0/5550", "my_gpio");
    gpio_res->set_value("0");
    gpio_res->methods(M2MMethod::GET | M2MMethod::PUT);
    gpio_res->attach_put_callback(gpio_updated);

#if defined(SENSOR_HDC1050)
    temperature_res = client.create_resource("3303/0/5700", "temperature");
    temperature_res->set_value(0);
    temperature_res->methods(M2MMethod::GET);
    temperature_res->observable(true);
    temperature_res->attach_notification_callback(notification_status);
    humidity_res = client.create_resource("3304/0/5700", "humidity");
    humidity_res->set_value(0);
    humidity_res->methods(M2MMethod::GET);
    humidity_res->observable(true);
    humidity_res->attach_notification_callback(notification_status);
#endif  

    printf("Initialized Pelion Client. Registering...\n");

    // Callback that fires when registering is complete
    client.on_registered(&registered);

    // Register with Pelion Device Management
    client.register_and_connect();


    uiMod = SECOND_UPDATE_SENSOR_TIME * 1000 / MS_SCHEDULE_TIME;
    uiRegUpdateMod = MINUTE_UPDATE_REGISTRATION * 60 * 1000 / MS_SCHEDULE_TIME;
    while (client.is_register_called()) 
    {
        if(count % uiMod == 0 && client.is_client_registered()) {
            vShowRssi();
#if defined(SENSOR_HDC1050)        
            HDC1050_GetSensorData(&fTemperature, &u16Humidity);
            printf("### %s Observe_res data update per %ds to keep alive: count: %d, time: %ds, temperature:%.4f, humidity:%d\n", 
                        DISPLAY_TITLE, 
                        (MS_SCHEDULE_TIME * uiMod)/1000, 
                        count, 
                        (count * MS_SCHEDULE_TIME)/1000, 
                        fTemperature, 
                        u16Humidity);
            temperature_res->set_value(fTemperature);
            humidity_res->set_value(u16Humidity);
#endif // SENSOR_HDC1050
        }

        //
        // Registration update to mbed cloud periodically (MINUTE_UPDATE_REGISTRATION)
        //        
        if((g_iSubscribeTemperature == 0) && (g_iSubscribeHumidity == 0) && (count % uiRegUpdateMod == 0) && (count != 0)) {
            printf("### Registration update to mbed cloud after time: %ds\n", (count * MS_SCHEDULE_TIME)/1000);
            client.register_update();
        }
        
        wait_ms(MS_SCHEDULE_TIME);
        count++;    
    }
    
}



//****************************************************************************
//
//! The thread to show console for configuratons
//!
//! \param none
//!
//! \return None.
//
//****************************************************************************
#if defined(WISE_1570_CONSOLE)
static void vConsoleThread(void)
{
    char aBuf[64];

    while(1) 
    {      
        ThisThread::sleep_for(100);
        MSG_print("> ");
        Console_iGetString(aBuf, 64);
        Console_iRunCmd(aBuf);
    }
    
}
#endif // WISE_1570_CONSOLE



//****************************************************************************
//
//! Reset to default
//!
//! \param none
//!
//! \return  On success, zero is returned. 
//!          On error, negative is returned.
//
//****************************************************************************
int Main_iResetToDefault(void)
{
    int iRet;

    iRet = Config_iReset();
    if(iRet != eCfg_Ok)
        return -1;

    return 0;
}



//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the communication task
//!    2. Invokes the console Task
//!
//! \return None.
//
//****************************************************************************
int main(void) 
{
    int iRet;

    //
    // Initiation for board
    //
    g_tPower = 1;
#if defined(WISE_1570_CONSOLE)
    Console_iInit();
#endif // WISE_1570_CONSOLE
    Config_iInit();

    // Display version information
    MSG_print("\f");
    MSG_print("\t\t *************************************************\n\r");
    MSG_print("\t\t\t\t");
    MSG_print("%s", SW_VERSION);
    MSG_print("\n\r");
    MSG_print("\t\t *************************************************\n\r");

    //
    // Running reset to default by checking 2-3 pin of SW3 on WISE-DB-1500
    //
    if(g_tBackUp.read() == 0) {
        MSG_print("Reset to default in progress... \n\r");
        if((iRet = Main_iResetToDefault()))
            MSG_print("Failed, ret: %d \n\r", iRet);
        else
            MSG_print("Done. \n\r");
    }

#if MBED_MEM_TRACING_ENABLED
    mbed_mem_trace_set_callback(mbed_mem_trace_default_callback);
#endif

#if defined(WISE_1570_CONSOLE)
    //
    // Running main thread by checking GPIO0
    //
    g_tGpio.output();
    g_tGpio = 0;
    if(g_tGpio.read() == 0) {
        vPdmcThread();
    }
    else {
        Cmd_iInit();
        vConsoleThread();
    }
#else
    vPdmcThread();
#endif

    //
    // Forever loop
    //
    while(1) {
        wait(1);
    }

    return 0;
}

#endif
