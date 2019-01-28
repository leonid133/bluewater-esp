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

static const char *sensor_endpoint = "http://bluewater.k8s.hydrosphere.io/sensor";
static const char *welcome_endpoint = "http://bluewater.k8s.hydrosphere.io/welcome";

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
  
  if(net_ip_acquired == true && wifi_acquired == true && (unsigned long) mgos_get_free_heap_size() > 30000) {
    off_count = 0;
    char buf[150];
    sprintf(buf, "{\"sec\": \"%lu\", \"uptime\": \"%.2lf\", \"status\": \"%d\", \"sec_f\": \"%.2lf\", \"sec_s\": \"%.2lf\",  \"ram_total\": \"%lu\",  \"ram_free\" : \"%lu\" }\n",
        (unsigned long)time(NULL),
          mgos_uptime(),
          mgos_gpio_read_out(mgos_sys_config_get_bw_pin_led()),
          TRIG_FIRST_TIME,
          TRIG_SECOND_TIME,
          (unsigned long) mgos_get_heap_size(),
          (unsigned long) mgos_get_free_heap_size());

    struct mg_connection *nc;
    nc = mg_connect_http(mgos_get_mgr(),
      ev_handler,
      NULL,
      sensor_endpoint,
      "Content-Type: application/json\r\n",
      buf
    );
  
    // if(nc != NULL) {
    //   LOG(LL_INFO,("nc is created"));
    // }
    mgos_shadow_updatef(0, "{ram_free: %d, uptime: %g, status: %B, first_time: %g, second_time: %g}",
      (unsigned long) mgos_get_free_heap_size(),
      mgos_uptime(),
      mgos_gpio_read_out(mgos_sys_config_get_bw_pin_led()),
      TRIG_FIRST_TIME,
      TRIG_SECOND_TIME); /* Report status */
    // LOG(LL_INFO,(buf));
  } else {
    off_count = off_count + 1;
    if(off_count>60){
      mgos_system_restart();
    }
  }
  (void) arg;
}

static void sensor_first(int pin, void *arg) {
  // LOG(LL_INFO,("Pin: %d", pin));
  TRIG_FIRST_TIME = mgos_uptime();
  // LOG(LL_INFO, ("First sensor is triggered %.2lf", TRIG_FIRST_TIME));
  mgos_gpio_write(mgos_sys_config_get_bw_pin_led(), 0);
  (void) arg;
}

static void sensor_second(int pin, void *arg) {
  // LOG(LL_INFO,("Pin: %d", pin));
  TRIG_SECOND_TIME = mgos_uptime();
  // LOG(LL_INFO, ("Second sensor is triggered %.2lf", TRIG_SECOND_TIME));
  mgos_gpio_write(mgos_sys_config_get_bw_pin_led(), 1);
  (void) arg;
}

static void connected_cb(int ev, void *ev_data, void *userdata) {
  mgos_shadow_updatef(0, "{status: %B, ram_free: %d}",
   mgos_gpio_read_out(mgos_sys_config_get_bw_pin_led()),
   (unsigned long) mgos_get_free_heap_size(),
   TRIG_FIRST_TIME,
   TRIG_SECOND_TIME); /* Report status */
  (void) ev;
  (void) ev_data;
  (void) userdata;
}

// static void delta_cb(int ev, void *ev_data, void *userdata) {
//   struct mg_str *delta = (struct mg_str *) ev_data;
//   int pin = mgos_sys_config_get_bw_pin_led();
//   bool on = false;

//   LOG(LL_INFO, ("GOT DELTA: [%.*s]", (int) delta->len, delta->p));

//   bool reboot = false;
//   json_scanf(delta->p, delta->len, "{reboot: %B}", &reboot);
//   if(reboot) {
//     mgos_system_restart();
//   }

// if (json_scanf(delta->p, delta->len, "{status: %B}", &on) != 1) {
//   LOG(LL_ERROR, ("Unexpected delta, looking for {on: true/false}"));
// } else if (!mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT)) {
//   LOG(LL_ERROR, ("mgos_gpio_set_mode(%d, GPIO_MODE_OUTPUT)", pin));
// } else {
//   bool inverted = true;
//   mgos_gpio_write(pin, inverted ? !on : on); /* Turn on/off the light */
//   mgos_shadow_updatef(0, "{status: %B}", mgos_gpio_read_out(mgos_sys_config_get_bw_pin_led())); /* Report status */
//   LOG(LL_INFO, ("DELTA applied"));
// }
//   (void) ev;
//   (void) userdata;
// }

enum mgos_app_init_result mgos_app_init(void) {
  
  LOG(LL_INFO, ("Hello from %s!\n", mgos_sys_config_get_device_id()));
    
  mgos_gpio_setup_output(mgos_sys_config_get_bw_pin_led(), 1);
  LOG(LL_INFO, ("Led pin %d", mgos_sys_config_get_bw_pin_led()));

  mgos_gpio_set_button_handler(mgos_sys_config_get_bw_pin_f(), MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_POS, mgos_sys_config_get_bw_debounce(), sensor_first,
    NULL);
  LOG(LL_INFO, ("First Sensor pin %d", mgos_sys_config_get_bw_pin_f()));

  mgos_gpio_set_button_handler(mgos_sys_config_get_bw_pin_s(), MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_POS, mgos_sys_config_get_bw_debounce(), sensor_second,
    NULL);
  LOG(LL_INFO, ("Second Sensor pin %d", mgos_sys_config_get_bw_pin_s()));

#ifdef MGOS_HAVE_WIFI
  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, net_cb, NULL);
  mgos_event_add_group_handler(MGOS_WIFI_EV_BASE, wifi_cb, NULL);
  mg_connect_http(mgos_get_mgr(), ev_handler, NULL, welcome_endpoint, "Content-Type: application/json\r\n", "{\"welcome\": \"hello\"}");
  mgos_set_timer(1000, MGOS_TIMER_REPEAT, timer_cb, NULL);
  // mgos_event_add_handler(MGOS_SHADOW_UPDATE_DELTA, delta_cb, NULL);
  mgos_event_add_handler(MGOS_SHADOW_CONNECTED, connected_cb, NULL);
#endif

  return MGOS_APP_INIT_SUCCESS;

}
