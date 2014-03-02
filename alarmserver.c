#include <stdio.h>
#include <string.h>
#include "mongoose.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


static const char *html_form =
  "<html><body style=\"background:black; color: green\">"
  "<h1>Alarm</h1>"
  "<p>%s<p>"
  "<p>"
  "<form method=\"POST\" action=\"/set_alarm\">"
  "Hour:<br/>"
  "<input type=\"text\" name=\"hour_input\" value=\"%d\"/> <br/>"
  "Minute:<br/>"
  "<input type=\"text\" name=\"minute_input\" value=\"%d\" /> <br/>"
  "<input type=\"submit\" value=\"Set Alarm\"/>"
  "</form>"
  "</p>"
  "%s"
  "</body></html>";

static const char *deactivate_form =
  "<p>"
  "<form method=\"POST\" action=\"/deactivate\">"
  "<input type=\"submit\" value=\"Deactivate Alarm\" />"
  "</form>"
  "</p>";

static int alarm_hour = 10;
static int alarm_minute = 0;
static int alarm_active = 0;

static void catch_alarm (int sig) {
  if(alarm_active){
    printf("HELLO AT TIME\n");
    system("./onalarm.sh &");
  }
  signal (sig, catch_alarm);
}

static int print_alarm_page(struct mg_connection *conn){
  time_t now = time(NULL);
  char * now_string = ctime(&now);
  mg_send_header(conn, "Content-Type", "text/html");
  mg_printf_data(conn,
		 html_form,
		 now_string,
		 alarm_hour,
		 alarm_minute,
		 alarm_active ? deactivate_form : "");
}

static void activate_alarm(){
    time_t now  = time(NULL);
    struct tm *localTime = localtime(&now);
    unsigned int secondsToAlarm;
    if (alarm_hour < localTime->tm_hour || 
	(alarm_hour == localTime->tm_hour && alarm_minute < localTime->tm_min)){
      //Next day
      secondsToAlarm = 
	(24 - localTime->tm_hour)*3600 - 
	(localTime->tm_min*60) +
	alarm_hour*3600 +
	alarm_minute*60 -
	localTime->tm_sec;
    } else if (alarm_hour == localTime->tm_hour && 
	       alarm_minute == localTime->tm_min) {
      secondsToAlarm = 1;
    }else{
      secondsToAlarm = 
	alarm_hour*3600 + 
	alarm_minute*60 - 
	localTime->tm_hour*3600 -
	localTime->tm_min*60 -
	localTime->tm_sec;
    }
    printf("ALARM IN %d seconds\n", secondsToAlarm);
    alarm_active = 1;
    alarm (secondsToAlarm);
}

static void deactivate_alarm(){
    alarm_active = 0;
    alarm (0); //cancel
}

static int handler(struct mg_connection *conn) {

  if (strcmp(conn->uri, "/set_alarm") == 0) {
    char hourText[500];
    char minuteText[500];
    mg_get_var(conn, "hour_input", hourText, sizeof(hourText));
    mg_get_var(conn, "minute_input", minuteText, sizeof(minuteText));

    alarm_hour = strtol(hourText, NULL, 10);
    alarm_minute = strtol(minuteText, NULL, 10);
    activate_alarm();
  } else if (strcmp(conn->uri, "/activate") == 0) {
    activate_alarm();
  } else if (strcmp(conn->uri, "/deactivate") == 0) {
    deactivate_alarm();
  }
  
  print_alarm_page(conn);

  return MG_REQUEST_PROCESSED;
}

int main(void) {
  struct mg_server *server = mg_create_server(NULL);
  mg_set_option(server, "listening_port", "8080");
  mg_set_request_handler(server, handler);
  signal (SIGALRM, catch_alarm);
  printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
  for (;;) {
    mg_poll_server(server, 1000);
  }
  mg_destroy_server(&server);
  return 0;
}
