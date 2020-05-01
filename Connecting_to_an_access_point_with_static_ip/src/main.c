/* 
Connecting to an access point with static ip example
Developed by: Walid Amriou
www.walidamriou.com

From Espressif IoT Development Framework Examples by comments (Notes) for Education purpose
https://github.com/walidamriou/ESP-IDF_Examples

Last update: April 2020
*/


#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <tcpip_adapter.h>
#include <lwip/sockets.h>

// The IP address that we want our device to have.
#define DEVICE_IP "192.168.1.111"
// The Gateway address where we wish to send packets. This will commonly be our access point.
#define DEVICE_GW "192.168.1.1"
// The netmask specification.
#define DEVICE_NETMASK "255.255.255.0"
// The identity of the access point to which we wish to connect.
#define AP_TARGET_SSID "your_WiFi_ssid_here"
// The password we need to supply to the access point for authorization.
#define AP_TARGET_PASSWORD "your_WiFi_password_here"

/* Event handler is used to tie events from WiFi/Ethernet/LwIP stacks into application logic.
   In general, event mean somethings that happens and event handler is a method that's called 
   when something happens, or is attached to an event. so here we programme to print the IP
   when SYSTEM_EVENT_STA_GOT_IP and wifi connect when SYSTEM_EVENT_STA_START. 

   but why use the method of event here?
   During the course of operating as a WiFi device, certain events may occur that board needs 
   to know about. These may be of importance or interest to the applications running within it.
   Since we don't know when, or even if, any events will happen, we can't have our board
   block waiting for them to occur. Instead what we should do is define a callback function that
   will be invoked should an event actually occur.
*/
esp_err_t event_handler(void * ctx, system_event_t * event) {

    /*
    we will be informed that got an assigned IP address from the access point that we connected 
    to while being a station when a SYSTEM_EVENT_STA_GOT_IP event is published, and The got_ip 
    data field is valid to be accessed. The got_ip union member corresponds to a struct of type
    system_event_sta_got_ip_t and This struct contains a member called ip_info, which is a 
    struct of type tcpip_adapter_ip_info_t, which contains three fields:
    ip (The IP address), netmask (The network mask) and gw (The gateway for communications); All 
    three of these fields are of ip4_addr_t which is a 32bit representation of an IP address.
    */
    if (event -> event_id == SYSTEM_EVENT_STA_GOT_IP) {
        /* 
        We use IP2STR() to generate four int values used in a printf statement 
        IP2STR(ip_addr_t *address)
        This is a macro which takes a pointer to an IP address and returns four comma separated 
        decimal values representing the 4 bytes of an IP address. This is commonly used in code 
        such as: printf("%d.%d.%d.%d", IP2STR(&addr)); and we can use the macro IPSTR can be used
        in place of 4 "%d" macros: printf(IPSTR, IP2STR(&addr)).
        In C, we use IP2STR() to converts the IP address (v4 or v6) to a string.
        */

        printf("Our IP address is " IPSTR "\n", IP2STR( & event -> event_info.got_ip.ip_info.ip));
        printf("We have now connected to a station and can do things...\n");
    }

    /* 
    We use esp_wifi_connect() to connect to an access point. Prior to calling this function we should
    have: • Initialized WiFi – esp_wifi_init()
          • Set our mode to be either a station or station+ap – esp_wifi_set_mode()
          • Set our desired access point connection information – esp_wifi_set_config()
          • Started the WiFi subsystem – esp_wifi_start() and received a SYSTEM_EVENT_STA_START event 
            message.
    It is very important to realize that we can't just call esp_wifi_start() and then immediately turn
    around and call esp_wifi_connect() . We must wait for the start event to have signaled completion
    by wait SYSTEM_EVENT_STA_START message. In other, We can disconnect from an access point by 
    calling  esp_wifi_disconnect() . Experience seems to show that before we can connect to a new 
    access point, we must first explicitly disconnect from a previous access point.
    */
    if (event -> event_id == SYSTEM_EVENT_STA_START) {
        /*
        ESP_ERROR_CHECK() macro serves similar purpose as assert, except that it 
        checks esp_err_t value rather than a bool condition. If the argument of 
        ESP_ERROR_CHECK() is not equal ESP_OK, then an error message is printed 
        on the console, and abort() is called.
        */
        ESP_ERROR_CHECK(esp_wifi_connect());
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
    The ip_info element is an instance of a tcpip_adapter_ip_info_t which contains three
    fields: ip (The IP address), netmask (the network mask) and gw (the gateway for 
    communication).
    so, in the next lines we just put our data to this fields. 
    Note: All three of these fields are of ip4_addr_t which is a 32bit representation of 
    an IP address.
    */
    tcpip_adapter_ip_info_t ipInfo;
    inet_pton(AF_INET, DEVICE_IP, &ipInfo.ip);
    inet_pton(AF_INET, DEVICE_GW, &ipInfo.gw);
    inet_pton(AF_INET, DEVICE_NETMASK, &ipInfo.netmask);

    /*
    Now, we use tcpip_adapter_set_ip_info() to Set our IP info.

    esp_err_t tcpip_adapter_set_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t* ip_info)
    */
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);


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
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    /* 
    The wifi_config_t defines properties of the interface. It is a C language union of 
    wifi_ap_config_t and wifi_sta_config_t .
    The wifi_sta_config_t contains: char ssid[32]
                                    char password[64]
                                    wifi_scan_method_t scan_method
                                    bool bssid_set
                                    uint8_t bssid[6]
                                    uint8_t channel
                                    wifi_sort_method_t sort_method
                                    wifi_fast_scan_threshold threshold
    */
    wifi_config_t sta_config = {
        /*
        What is Bssid?
        Basic Service Set Identifiers (BSSID) A service set consists of a group of wireless network 
        devices which operates with the same parameters of networking.BSSID is simply the MAC address
        of a wireless access point or also known as WAP.
        and here in our example we don't set it. 
        */
        .sta = {
            .ssid = AP_TARGET_SSID,
            .password = AP_TARGET_PASSWORD,
            .bssid_set = 0
        }
    };

    /* 
    We use esp_wifi_set_config() to set the WiFi interface configuration.
    esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t* conf)
     The interface is one of:  WIFI_IF_STA and it means The station interface or
    WIFI_IF_AP and it means The access point interface. We should previously have called
    esp_wifi_set_mode()
    */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, & sta_config));


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