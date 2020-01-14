#include "mgos.h"
#include "mgos_gpio.h"
#ifdef MGOS_HAVE_WIFI
#include "mgos_wifi.h"
#include "mgos_shadow.h" 
#include "mgos_net.h"
#include "mongoose.h"
#endif

static float TRIG_FIRST_TIME = 0.0;
static float TRIG_SECOND_TIME = 0.0;

static int exit_flag = 0;
static bool net_ip_acquired = false;
static bool wifi_acquired = false;

static int off_count = 0;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
  struct http_message *hm = (struct http_message *) ev_data;
  int connect_status;

  switch (ev) {
    case MG_EV_CONNECT:
      connect_status = *(int *) ev_data;
      if (connect_status != 0) {
        printf("Error connecting: %s\n", strerror(connect_status));
        exit_flag = 1;
      }
      break;
    case MG_EV_HTTP_REPLY:
      printf("Got reply:\n%.*s\n", (int) hm->body.len, hm->body.p);
      nc->flags |= MG_F_SEND_AND_CLOSE;
      exit_flag = 1;
      break;
    case MG_EV_CLOSE:
      if (exit_flag == 0) {
        printf("Server closed connection\n");
        exit_flag = 1;
      };
      break;
    default:
      break;
  }
      
}

static void net_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_NET_EV_DISCONNECTED:
      net_ip_acquired = false;
      LOG(LL_INFO, ("%s", "Net disconnected"));
      break;
    case MGOS_NET_EV_CONNECTING:
      LOG(LL_INFO, ("%s", "Net connecting..."));
      net_ip_acquired = false;
      break;
    case MGOS_NET_EV_CONNECTED:
      LOG(LL_INFO, ("%s", "Net connected"));
      break;
    case MGOS_NET_EV_IP_ACQUIRED:
      LOG(LL_INFO, ("%s", "Net got IP address"));
      net_ip_acquired = true;
      break;
  }

  (void) evd;
  (void) arg;
}

#ifdef MGOS_HAVE_WIFI
static void wifi_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_WIFI_EV_STA_DISCONNECTED: {
      wifi_acquired = false;      
      struct mgos_wifi_sta_disconnected_arg *da =
          (struct mgos_wifi_sta_disconnected_arg *) evd;
      LOG(LL_INFO, ("WiFi STA disconnected, reason %d", da->reason));
      break;
    }
    case MGOS_WIFI_EV_STA_CONNECTING:
      wifi_acquired = false;  
      LOG(LL_INFO, ("WiFi STA connecting %p", arg));
      break;
    case MGOS_WIFI_EV_STA_CONNECTED:
      LOG(LL_INFO, ("WiFi STA connected %p", arg));
      break;
    case MGOS_WIFI_EV_STA_IP_ACQUIRED:
      wifi_acquired = true;
      LOG(LL_INFO, ("WiFi STA IP acquired %p", arg));
      
      break;
    case MGOS_WIFI_EV_AP_STA_CONNECTED: {
      struct mgos_wifi_ap_sta_connected_arg *aa =
          (struct mgos_wifi_ap_sta_connected_arg *) evd;
      LOG(LL_INFO, ("WiFi AP STA connected MAC %02x:%02x:%02x:%02x:%02x:%02x",
                    aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
                    aa->mac[5]));
      break;
    }
    case MGOS_WIFI_EV_AP_STA_DISCONNECTED: {
      struct mgos_wifi_ap_sta_disconnected_arg *aa =
          (struct mgos_wifi_ap_sta_disconnected_arg *) evd;
      LOG(LL_INFO,
          ("WiFi AP STA disconnected MAC %02x:%02x:%02x:%02x:%02x:%02x",
           aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
           aa->mac[5]));
      break;
    }
  }
  (void) arg;
}
#endif /* MGOS_HAVE_WIFI */

static void timer_cb(void *arg) {
  if(net_ip_acquired == true && wifi_acquired == true) {
    off_count = 0;
    if(mgos_dash_is_connected()) {
      mgos_shadow_updatef(0, "{time: %d,ram_free: %d, uptime: %g, status: %B, first_time: %g, second_time: %g}",
          (unsigned long) time(NULL),
          (unsigned long) mgos_get_free_heap_size(),
          mgos_uptime(),
          mgos_gpio_read_out(mgos_sys_config_get_bw_pin_led()),
          TRIG_FIRST_TIME,
          TRIG_SECOND_TIME); /* Report status */
    }
  } else {
    off_count = off_count + 1;
    if(off_count>120){
      mgos_system_restart();
    }
  }
  (void) arg;
}

static void sensor_first(int pin, void *arg) {
  // LOG(LL_INFO,("Pin: %d", pin));
  TRIG_FIRST_TIME = mgos_uptime();
  mgos_gpio_write(mgos_sys_config_get_bw_pin_led(), 0);
  (void) arg;
}

static void sensor_second(int pin, void *arg) {
  // LOG(LL_INFO,("Pin: %d", pin));
  TRIG_SECOND_TIME = mgos_uptime();
  mgos_gpio_write(mgos_sys_config_get_bw_pin_led(), 1);
  (void) arg;
}

static void connected_cb(int ev, void *ev_data, void *userdata) {
  (void) ev;
  (void) ev_data;
  (void) userdata;
}

enum mgos_app_init_result mgos_app_init(void) {
  
  LOG(LL_INFO, ("Hello from %s!\n", mgos_sys_config_get_device_id()));
    
  mgos_gpio_setup_output(mgos_sys_config_get_bw_pin_led(), 1);
  mgos_gpio_set_button_handler(mgos_sys_config_get_bw_pin_f(), MGOS_GPIO_PULL_NONE, MGOS_GPIO_INT_EDGE_POS, mgos_sys_config_get_bw_debounce(), sensor_first,
    NULL);
  mgos_gpio_set_button_handler(mgos_sys_config_get_bw_pin_s(), MGOS_GPIO_PULL_NONE, MGOS_GPIO_INT_EDGE_POS, mgos_sys_config_get_bw_debounce(), sensor_second,
    NULL);

#ifdef MGOS_HAVE_WIFI
  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, net_cb, NULL);
  mgos_event_add_group_handler(MGOS_WIFI_EV_BASE, wifi_cb, NULL);
  mgos_set_timer(1000, MGOS_TIMER_REPEAT, timer_cb, NULL);
  mgos_event_add_handler(MGOS_SHADOW_CONNECTED, connected_cb, NULL);
#endif

  return MGOS_APP_INIT_SUCCESS;

}
