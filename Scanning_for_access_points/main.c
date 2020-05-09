/* 
Scanning for access points example
Developed by: Walid Amriou
www.walidamriou.com

From Espressif IoT Development Framework Examples by comments (Notes) for Education purpose
https://github.com/walidamriou/ESP-IDF_Examples

Last update: April 2020
*/

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

/* Event handler is used to tie events from WiFi/Ethernet/LwIP stacks into application logic.
   In general, event mean somethings that happens and event handler is a method that's called 
   when something happens, or is attached to an event. so here we programme to print the result
   when SYSTEM_EVENT_SCAN_DONE. 

   but why use the method of event here?
   During the course of operating as a WiFi device, certain events may occur that board needs 
   to know about. These may be of importance or interest to the board running within it.
   Since we don't know when, or even if, any events will happen, we can't have our board
   block waiting for them to occur. Instead what we should do is define a callback function that
   will be invoked should an event actually occur.
*/
esp_err_t event_handler(void * ctx, system_event_t * event) {

    /*
    we will be informed that the scan completed when a SYSTEM_EVENT_SCAN_DONE event is published, 
    we get it from the event parameter system_event_t which contain the details of the event:
     system_event_id_t event_id   and    system_event_info_t event_info
    */

    if (event -> event_id == SYSTEM_EVENT_SCAN_DONE) {

        //print Number of access points found
        printf("Number of access points found: %d\n", event -> event_info.scan_done.number);

        uint16_t apCount = event -> event_info.scan_done.number;
        if (apCount == 0) {
            //ESP_OK is esp_err_t value indicating success (no error) and exit
            return ESP_OK;
        }
        /* a scan record is contained in an instance of a wifi_ap_record_t structure that contains:
        uint8_t bssid[6] , uint8_t ssid[32] , uint8_t primary , wifi_second_chan_t second
          , int8_t rssi , wifi_auth_mode_t authmode ;

        */
        wifi_ap_record_t * list = (wifi_ap_record_t * ) malloc(sizeof(wifi_ap_record_t) * apCount);
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records( & apCount, list));
        int i;
        for (i = 0; i < apCount; i++) {
            //The wifi possible auth modes
            char * authmode;
            switch (list[i].authmode) {
                //No security
                case WIFI_AUTH_OPEN:
                    authmode = "WIFI_AUTH_OPEN";
                    break;
                //WEP security
                case WIFI_AUTH_WEP:
                    authmode = "WIFI_AUTH_WEP";
                    break;
                //WPA security
                case WIFI_AUTH_WPA_PSK:
                    authmode = "WIFI_AUTH_WPA_PSK";
                    break;
                //WPA2 security
                case WIFI_AUTH_WPA2_PSK:
                    authmode = "WIFI_AUTH_WPA2_PSK";
                    break;
                //WPA or WPA2 security
                case WIFI_AUTH_WPA_WPA2_PSK:
                    authmode = "WIFI_AUTH_WPA_WPA2_PSK";
                    break;
                //Unknown
                default:
                    authmode = "Unknown";
                    break;
            }
            printf("ssid=%s, rssi=%d, authmode=%s\n",list[i].ssid, list[i].rssi, authmode);
        }
        free(list);
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
    tcpip_adapter_init() is for Initialize the underlying TCP/IP stack 
    This function should be called exactly once from application code, 
    when the application starts up. 
    */
    tcpip_adapter_init();
    

    /*
    ESP_ERROR_CHECK() macro serves similar purpose as assert, except that it 
    checks esp_err_t value rather than a bool condition. If the argument of 
    ESP_ERROR_CHECK() is not equal ESP_OK, then an error message is printed 
    on the console, and abort() is called.
    */

    /*
    esp_event_loop_init() registers a event_handler function to called when the board detects 
    SYSTEM_EVENT_SCAN_DONE event . so the next line just to register the callback function.
    */
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    //Initialize the ESP32 WiFi environment
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init( & cfg));

    /*
    -- Set the WiFi API configuration storage type as WIFI_STORAGE_RAM. The default value is WIFI_STORAGE_FLASH
    -- Return: ESP_OK: succeed
               ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
               ESP_ERR_INVALID_ARG: invalid argument
    */
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    
    /*
    Set the WiFi operating mode by esp_wifi_set_mode(wifi_mode_t); The parameter is an instance of
    wifi_mode_t which can have a value of: WIFI_MODE_NULL , WIFI_MODE_STA , WIFI_MODE_AP or WIFI_MODE_APSTA.
    We can call esp_wifi_get_mode() to retrieve our current mode state.
    */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    /*
    -- Start WiFi according to current configuration.
    If mode is WIFI_MODE_STA, it create station control block and start station.
    If mode is WIFI_MODE_AP, it create soft-AP control block and start soft-AP.
    If mode is WIFI_MODE_APSTA, it create soft-AP and station control block and
     start soft-AP and station.
    -- Return: ESP_OK: succeed
               ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
               ESP_ERR_INVALID_ARG: invalid argument
               ESP_ERR_NO_MEM: out of memory
               ESP_ERR_WIFI_CONN: WiFi internal error, station or soft-AP control block wrong
               ESP_FAIL: other WiFi internal errors
    */
    ESP_ERROR_CHECK(esp_wifi_start());

    /* We config the parameters for an SSID scan. and generally it has:
       uint8_t *ssid          SSID of AP
       uint8_t *bssid         MAC address of AP
       uint8_t channel        channel, scan the specific channel
       bool show_hidden       enable to scan AP whose SSID is hidden
       wifi_scan_type_t scan_type      scan type, active or passive
       wifi_scan_time_t scan_time    scan time per channel

       we note that The values of maximum active scan time and passive scan time 
       per channel are limited to 1500 milliseconds. Values above 1500ms may cause 
       station to disconnect from AP and are not recommended.

    */
    wifi_scan_config_t scanConf = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = 1
    };

    /* 
       -- Scan all available APs.the found APs are stored in WiFi driver dynamic 
       allocated memory and we get it by esp_wifi_scan_get_ap_records.
       -- Return: ESP_OK: succeed
                  ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
                  ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
                  ESP_ERR_WIFI_TIMEOUT: blocking scan is timeout
                  ESP_ERR_WIFI_STATE: wifi still connecting when invoke esp_wifi_scan_start
                  others: refer to error code in esp_err.h
        -- Parameters: config: configuration of scanning
                    block: if block is true (1), this API will block the caller until the scan
                      is done, otherwise it will return immediately.
                      
        NOTE: After issuing the request to start performing a scan, we will be informed that 
        the scan completed when a SYSTEM_EVENT_SCAN_DONE event is published.
    */
    ESP_ERROR_CHECK(esp_wifi_scan_start( & scanConf, 0));
    return 0;
}