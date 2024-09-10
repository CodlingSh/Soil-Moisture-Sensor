int connectToWiFi(uint32_t country, const char *ssid, const char *pass, uint32_t auth, const char *hostname, ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw) {

    if (cyw43_arch_init_with_country(country)) {
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if (hostname != NULL) {
        netif_set_hostname(netif_default, hostname);
    }
    if (cyw43_arch_wifi_connect_async(ssid, pass, auth)) {
        return 2;
    }
    int wifi_status = CYW43_LINK_UP + 1; // Set to 4 as to not interfere with getting status
    while (wifi_status != CYW43_LINK_UP && wifi_status > 0) {
        int new_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

        if (new_status != wifi_status) {
            wifi_status = new_status;
            printf("Connection status: %d\n", wifi_status);
        }
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(200);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(200);
    }

    if (wifi_status <= 0) {
        printf("Couldn't connect\n");
        while (true) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(50);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(50);
        }
    }

    if (wifi_status == CYW43_LINK_UP) {
        int rssi = cyw43_wifi_get_rssi(&cyw43_state, 0);
        printf("CONNECTION SUCCESSFUL!\n");
        printf("Signal strength: %d dBm\n", rssi);
    }

    return wifi_status;

    // int flashrate = 1000;
    // int status = CYW43_LINK_UP + 1;
    // while (status >= 0 && status != CYW43_LINK_UP)
    // {
    //     int new_status = cyw43_tcpip_link_status(&cyw43_state,
    //                                              CYW43_ITF_STA);
    //     if (new_status != status)
    //     {
    //         status = new_status;
    //         flashrate = flashrate / (status + 1);
    //         printf("connect status: %d %d\n", status, flashrate);
    //     }
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    //     sleep_ms(flashrate);
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    //     sleep_ms(flashrate);
    // }
    // if (status < 0)
    // {
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    // }
    // else
    // {
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    //     if (ip != NULL)
    //     {
    //         netif_set_ipaddr(netif_default, ip);
    //     }
    //     if (mask != NULL)
    //     {
    //         netif_set_netmask(netif_default, mask);
    //     }
    //     if (gw != NULL)
    //     {
    //         netif_set_gw(netif_default, gw);
    //     }

    //     printf("IP: %s\n", ip4addr_ntoa(netif_ip_addr4(netif_default)));
    //     printf("Mask: %s\n", ip4addr_ntoa(netif_ip_netmask4(netif_default)));
    //     printf("Gateway: %s\n", ip4addr_ntoa(netif_ip_gw4(netif_default)));
    //     printf("Host Name: %s\n", netif_get_hostname(netif_default));
    // }
    // return status;
}