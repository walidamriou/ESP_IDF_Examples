/* 
Board as an WiFi access point WAP example
Developed by: Walid Amriou
www.walidamriou.com

From Espressif IoT Development Framework Examples by comments (Notes) for Education purpose
https://github.com/walidamriou/ESP-IDF_Examples

Last update: May 2020
*/

#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <tcpip_adapter.h>
#include <lwip/sockets.h>
#include <stdint.h>

// The identity of the access point
#define AP_SSID "ESP32_board_AP"
// The password we need to supply to the access point for authorization.
#define AP_PASSWORD "123456789AP"

#define AP_AUTH_TYPE WIFI_AUTH_WPA2_PSK

uint8_t number_devices_connected;
uint8_t number_devices_disconnected;


/* Event handler is used to tie events from WiFi/Ethernet/LwIP stacks into application logic.
   In general, event mean somethings that happens and event handler is a method that's called 
   when something happens, or is attached to an event.  

   but why use the method of event here?
   During the course of operating as a WiFi device, certain events may occur that board needs 
   to know about. These may be of importance or interest to the applications running within it.
   Since we don't know when, or even if, any events will happen, we can't have our board
   block waiting for them to occur. Instead what we should do is define a callback function that
   will be invoked should an event actually occur.
*/

esp_err_t event_handler(void * ctx, system_event_t * event) {
 
    //When WiFi Access point start, the ESP32 WiFi event is produced of type SYSTEM_EVENT_AP_START.
    if (event -> event_id == SYSTEM_EVENT_AP_START) {
        /*
        ESP_ERROR_CHECK() macro serves similar purpose as assert, except that it 
        checks esp_err_t value rather than a bool condition. If the argument of 
        ESP_ERROR_CHECK() is not equal ESP_OK, then an error message is printed 
        on the console, and abort() is called.
        */
        printf("WiFi Access Point started\n");
        printf("WiFi AP name: %s \n",AP_SSID); //print WiFi AP name
        printf("WiFi AP password: %s \n",AP_PASSWORD); //Print WiFi AP passwordwifi_ap_list_t apList[10];
    }
    
    /*
    When a station connects, the ESP32 will raise the SYSTEM_EVENT_AP_STACONNECTED event. When a
    station disconnects, we will see the SYSTEM_EVENT_AP_DISCONNECTED event.
    */
    if(event -> event_id == SYSTEM_EVENT_AP_STACONNECTED){
        printf("Device connected to the %s WiFi Access Point.\n",AP_SSID);
        
        //the number of stations(devices) connected to the AP
        // struct wifi_sta_list_t is the list of stations associated with the ESP32 Soft-AP. 
        wifi_sta_list_t count;
        esp_wifi_ap_get_sta_list(&count);
        //int  int num is the number of stations in the list (other entries are invalid)
        printf("Number of stations connected Now = %d\n", count.num);

    }
    if(event -> event_id == SYSTEM_EVENT_AP_STADISCONNECTED){
        printf("Device disconnected from the %s WiFi Access Point.\n",AP_SSID);

        //the number of stations(devices) connected to the AP
        // struct wifi_sta_list_t is the list of stations associated with the ESP32 Soft-AP. 
        wifi_sta_list_t count;
        esp_wifi_ap_get_sta_list(&count);
        //int  int num is the number of stations in the list (other entries are invalid)
        printf("Number of stations connected Now = %d\n", count.num);
    }
    
    
    //ESP_OK is esp_err_t value indicating success (no error)
    return ESP_OK;
}

int app_main(void) {

     
    /* 
    -- nvs_flash_init() Initialize the default NVS partition.
    This API initialises the default NVS partition. 
    The default NVS partition is the one that is labeled “nvs” in the partition table.
    -- Return:  ESP_OK if storage was successfully initialized.
                ESP_ERR_NVS_NO_FREE_PAGES if the NVS storage contains no empty pages 
                   (which may happen if NVS partition was truncated)
                ESP_ERR_NOT_FOUND if no partition with label “nvs” is found in the 
                   partition table one of the error codes from the underlying flash storage driver
    */
    nvs_flash_init();
    

    /*
    When the ESP32 connects to an access point as a station, it also runs a DHCP client to
    connect to the DHCP server that it assumes is also available at the access point. From
    there, the station is supplied its IP address, gateway address and netmask. But in this
    example we want to supply our own values for this data (IP,gateway and netmask). We can
    do this by calling tcpip_adapter_set_ip_info() during setup. The recipe is as follows:
    tcpip_adapter_init();
    tcpip_adapter_dhcpc_stop();
    tcpip_adapter_set_ip_info();
    esp_wifi_init();
    esp_wifi_set_mode();
    esp_wifi_set_config();
    esp_wifi_start();
    esp_wifi_config();

    tcpip_adapter_init() is for Initialize the underlying TCP/IP stack 
    This function should be called exactly once from application code, 
    when the application starts up. 
    */
    tcpip_adapter_init();

    //// Don't run a DHCP client (to note get auto ip and other date)
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); 


    /*
    esp_event_loop_init() registers a event_handler function to called when the board detects 
    SYSTEM_EVENT_STA_GOT_IP or SYSTEM_EVENT_STA_START event . so the next line just to register
    the callback function.
    */
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    //Initialize the ESP32 WiFi environment
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init( & cfg));


    /*
    -- Set the WiFi API configuration storage type as WIFI_STORAGE_RAM. The default value is 
    WIFI_STORAGE_FLASH
    -- Return: ESP_OK: succeed
               ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
               ESP_ERR_INVALID_ARG: invalid argument
    */
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    /*
    Set the WiFi operating mode by esp_wifi_set_mode(wifi_mode_t); The parameter is an instance of
    wifi_mode_t which can have a value of: WIFI_MODE_NULL , WIFI_MODE_STA , WIFI_MODE_AP or 
    WIFI_MODE_APSTA. We can call esp_wifi_get_mode() to retrieve our current mode state.
    */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    /* 
    The wifi_config_t defines properties of the interface. It is a C language union of 
    wifi_ap_config_t and wifi_sta_config_t .
    The wifi_ap_config_t contains: 
     -- uint8_t ssid[32]    : SSID of ESP32 soft-AP. If ssid_len field is 0, this must be a Null 
                           terminated string. Otherwise, length is set according to ssid_len.
     -- uint8_t password[64]    : Password of ESP32 soft-AP. Null terminated string.
     -- uint8_t ssid_len    :Optional length of SSID field.
     -- uint8_t channel    :Channel of ESP32 soft-AP.
     -- wifi_auth_mode_tauthmode    : Auth mode of ESP32 soft-AP. Do not support AUTH_WEP in soft-AP
                                   mode.
     -- uint8_t ssid_hidden    : Broadcast SSID or not, default 0, broadcast the SSID.
     -- uint8_t max_connection    : Max number of stations allowed to connect in, default 4, max 10.
     -- uint16_t beacon_interval    : Beacon interval, 100 ~ 60000 ms, default 100 ms.
    
    */
    wifi_config_t apConfig = {
        .ap = {
            .ssid=AP_SSID,
            .ssid_len=0,
            .password=AP_PASSWORD,
            .channel=0,
            .authmode=AP_AUTH_TYPE,
            .ssid_hidden=0,
            .max_connection=4,
            .beacon_interval=100
            }
        };


    /* 
    We use esp_wifi_set_config() to set the WiFi interface configuration.
    esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t* conf)
     The interface is one of:  WIFI_IF_STA and it means The station interface or
    WIFI_IF_AP and it means The access point interface. We should previously have called
    esp_wifi_set_mode()
    */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfig));


    /*
    -- Start WiFi according to current configuration.
    If mode is WIFI_MODE_STA, it create station control block and start station. (our mode here)
    If mode is WIFI_MODE_AP, it create soft-AP control block and start soft-AP.
    If mode is WIFI_MODE_APSTA, it create soft-AP and station control block and start soft-AP and 
    station.
    -- Return: ESP_OK: succeed
               ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
               ESP_ERR_INVALID_ARG: invalid argument
               ESP_ERR_NO_MEM: out of memory
               ESP_ERR_WIFI_CONN: WiFi internal error, station or soft-AP control block wrong
               ESP_FAIL: other WiFi internal errors
    */
    ESP_ERROR_CHECK(esp_wifi_start());
    return 0;
}