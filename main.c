#include <stdio.h>
#include <time.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "pico/unique_id.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/smtp.h"
//#include "lwip/apps/sntp.h"
#include "hardware/rtc.h"
#include "setupWiFi.h"
#include "credentials.h"
#include "sntp.h"

void my_smtp_result_fn(void *arg, u8_t smtp_result, u16_t srv_err, err_t err) {
    printf("mail (%p) sent with results 0x%02x, 0x%08x\n", arg, smtp_result, srv_err, err);
}

void send_smtp_mail() {
    struct tm t;
    getDateNow(&t);

    char Date[100];
    strftime(Date, sizeof(Date), "%a, %d %b %Y %H:%M:%S %Z", &t);
    char message[] = "This email was sent at ";
    strcat(message, Date);

    smtp_send_mail("sheldonspeppers@gmail.com", "5193284249@txt.bell.ca", "", message, my_smtp_result_fn, NULL);
}

datetime_t *set_init_alarm_time(datetime_t *curr_time, datetime_t *alarm_time) {
    int final_day;
    bool is_leap_year = false;

    alarm_time->year = curr_time->year;
    alarm_time->hour = 12;
    alarm_time->min = 0;
    alarm_time->sec = 0;

    if (curr_time->year % 4 == 0 && (curr_time->year % 100 != 0 || curr_time->year % 400 == 0)) {
        is_leap_year = true;
    }

    if (curr_time->hour >= 12) {
        // Determine final day of the month
        switch (curr_time->month) {
            case 2:
                final_day = is_leap_year ? 29 : 28;
                break;
            case 4:
            case 6: 
            case 9: 
            case 11:
                final_day = 30;
                break;
            default:
                final_day = 31;
                break;
        }

        // Determine if it is the last day of the month
        if (curr_time->day >= final_day) {
            if (curr_time->month < 12) {
                alarm_time->month = curr_time->month + 1;
            } else {
                alarm_time->month = 1;
                alarm_time->year = curr_time->year + 1;
            }
            alarm_time->day = 1;
        } else {
            alarm_time->month = curr_time->month;
            alarm_time->day = curr_time->day + 1;
        }
    } else {
        alarm_time->month = curr_time->month;
        alarm_time->day = curr_time->day;
    }

    return alarm_time;
}

int main() {

    stdio_init_all();

    pico_unique_board_id_t uid;
    pico_get_unique_board_id(&uid);
    printf("\n\n%02x\n", uid.id);
    printf("%d\n\n", uid.id);

    printf("\n\n\n");
    printf("%x%x%x%x%x%x%x%x\n", uid.id[0],  uid.id[1], uid.id[2], uid.id[3], uid.id[4], uid.id[5], uid.id[6], uid.id[7]);
    
    // Connect to Wifi and initialize TLS client for SMTP
    connectToWiFi(country, ssid, pass, auth, "Soil-sensor", NULL, NULL, NULL);
    struct altcp_tls_config *tls_config = altcp_tls_create_config_client(NULL, 0);
    smtp_set_tls_config(tls_config);


    while (true)
    {
        struct timeStatus *tstatus = getSNTP();
        sleep_ms(500);
        if (pollSNTP(tstatus))
            break;
        cancelSNTP(tstatus);
    }

    // Create 2 datetime variables to grab time and set alarm time based on current time.
    datetime_t t;
    datetime_t alarm_time; 
    rtc_get_datetime(&t);
    set_init_alarm_time(&t, &alarm_time);

    smtp_set_server_addr("smtp.gmail.com");
    smtp_set_server_port(465);
    smtp_set_auth(email_address, email_password);

    rtc_set_alarm(&alarm_time, *send_smtp_mail);
    printf("current time is %d/%d/%d at %d:%02d:%02d\n", t.month, t.day, t.year, t.hour, t.min, t.sec);
    printf("Alarm is set for %d/%d/%d at %d:%02d:%02d\n", alarm_time.month, alarm_time.day, alarm_time.year, alarm_time.hour, alarm_time.min, alarm_time.sec);

    
    // Send initial email once system is started
    //smtp_send_mail("sheldonspeppers@gmail.com", "5193284249@txt.bell.ca", "", message, my_smtp_result_fn, NULL);
    
    //smtp_send_mail("sheldonspeppers@gmail.com", "5193284249@txt.bell.ca", "", "Hi Sheldon, This is a test message from the soil sensor", my_smtp_result_fn, NULL);
    //struct repeating_timer timer;
    //add_repeating_timer_ms(60000, send_smtp_mail, NULL, &timer);

    while(1) {
        tight_loop_contents();
    }
}