#include "hardware/rtc.h"

#define TIMEZONE_OFFSET -4 // Adjust this value for your time zone
#define DST_OFFSET 1

bool getDateNow(struct tm *t)
{
    datetime_t rtc;
    bool state = rtc_get_datetime(&rtc);
    if (state)
    {
        t->tm_sec = rtc.sec;
        t->tm_min = rtc.min;
        t->tm_hour = rtc.hour;
        t->tm_mday = rtc.day;
        t->tm_mon = rtc.month - 1;
        t->tm_year = rtc.year - 1900;
        t->tm_wday = rtc.dotw;
        t->tm_yday = 0;
        t->tm_isdst = -1;
    }
    return state;
}

void setRTC(struct tm *datetime)
{
    datetime_t t;
    t.year = datetime->tm_year + 1900;
    t.month = datetime->tm_mon + 1;
    t.day = datetime->tm_mday;
    t.dotw = datetime->tm_wday;
    t.hour = datetime->tm_hour;
    t.min = datetime->tm_min;
    t.sec = datetime->tm_sec;
    rtc_init();
    rtc_set_datetime(&t);
}
struct timeStatus
{
    bool ready;
    struct udp_pcb *pcb;
};

void recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    struct timeStatus *tstatus = (struct timeStatus *)arg;
    printf("SNTP responded\n");
    if (p != NULL)
    {
        uint8_t seconds_buf[4];
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        time_t seconds_since_1970 = seconds_since_1900 - 2208988800 + TIMEZONE_OFFSET * 3600;
        struct tm *datetime = gmtime(&seconds_since_1970);
        setRTC(datetime);
        pbuf_free(p);
        udp_remove(pcb);
        tstatus->pcb=NULL;
        tstatus->ready = true;
    }
}

bool pollSNTP(struct timeStatus *tstatus)
{
    if (tstatus == NULL)
        return true;
    if (tstatus->ready)
    {
        free(tstatus);
        tstatus = NULL;
        return true;
    }
    return false;
}

void dns_found(const char *name, const ip_addr_t *ip, void *arg)
{
    struct timeStatus *tstatus = (struct timeStatus *)arg;
    printf("DNS %s\n", ipaddr_ntoa(ip));
    struct udp_pcb *pcb = udp_new();
    tstatus->pcb = pcb;
    udp_recv(pcb, recv, arg);
    udp_bind(pcb, IP_ADDR_ANY, 123);
    udp_connect(pcb, ip, 123);
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 48, PBUF_RAM);
    uint8_t *payload = (uint8_t *)p->payload;
    memset(payload, 0, 48);
    *payload = 0x1b;
    err_t er = udp_send(pcb, p);
    pbuf_free(p);
}

struct timeStatus *getSNTP()
{
    struct timeStatus *tstatus = malloc(sizeof(struct timeStatus));
    tstatus->ready = false;
    tstatus->pcb=NULL;
    ip_addr_t ip;
    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname("time.nist.gov", &ip, dns_found, tstatus);
    cyw43_arch_lwip_end();
    if (err == ERR_OK)
    {
        printf("DNS cache %s\n", ipaddr_ntoa(&ip));
        dns_found("", &ip, tstatus);
    }
    return tstatus;
}

void cancelSNTP(struct timeStatus *tstatus)
{
    if (tstatus != NULL)
    {
        udp_remove(tstatus->pcb);
        free(tstatus);
        tstatus = NULL;
        printf("canceled\n");
    }
}