/* http_get - Retrieves a web page over HTTP GET.
 *
 * See http_get_ssl for a TLS-enabled version.
 *
 * This sample code is in the public domain.,
 */
#include "espressif/esp_common.h"
#include "espressif/sdk_private.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "ssid_config.h"

#define PORT 80
#define MAXDATASIZE 1024

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_html[] = "<html> \
<head> \
<meta http-equiv=\"Content-Type\" content=\"text/html;charset=ISO-8859-1\"> \
<title>NETWORK CONFIGURATION</title> \
</head> \
<body bgcolor=\"#ffffff\"> \
<h1>NETWORK SETTING</h1> \
<FORM ACTION=\"\" METHOD=\"GET\"> \
<p><span style=\"font-size: 20px\">SSID&nbsp;</span>&nbsp; \
<input maxlength=\"50\" name=\"ssid\" value=\"\" size=\"9\" style=\"height: 22px; width: 152px\" type=\"text\" /></p> \
<p align=\"left\"><span style=\"font-size: 20px\">PASS</span>&nbsp; \
<input type=\"password\" maxlength=\"50\" name=\"pass\" value=\"\" size=\"9\" style=\"height: 22px; width: 152px\" type=\"text\" /></p> \
<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; \
<input name=\"Save\" style=\"height: 22px; width: 68px\" type=\"submit\" value=\"Save\" />&nbsp;&nbsp;&nbsp; \
<input name=\"Reboot\" style=\"height: 22px; width: 68px\" type=\"submit\" value=\"Reboot\" /></p> \
</body> \
</html>";

/*************************************************************************************
  Function name : send_msg ()
  Description   : Send message
  Parametter    : int fd, const char *msg
  Return        : void
  Key word      : @Function send_msg
  Author        : kamejoko80             
*************************************************************************************/
int send_msg (int fd, char *msg)
{
	int numbytes, sent;	        /* Number of byte send   */
    int total;

    total = strlen (msg);
    sent = 0;
    
    do 
    {
        /* Send file to host */
        if ((numbytes = send (fd, msg + sent, total - sent, 0)) < 0)
        { 
            printf("send() error\n");
            return -1; 
        }    
        else if (numbytes == 0)
        {   
            break;
        }
        
        sent += numbytes;
    
    } while (sent < total);
    
    return 0; /* successful */
}

/*************************************************************************************
  Function name : recv_msg ()
  Description   : Receive text message from the host
  Parametter    : 
  Return        : numbytes if success, -1 failed
  Key word      : @Function recv_msg
  Author        : kamejoko80             
*************************************************************************************/
int recv_msg (int fd, char *buf)
{
	int numbytes;	        /* Number of byte received */
	
	if ((numbytes = recv (fd, buf, MAXDATASIZE, 0)) == -1)
	{ 
    	printf("recv() error\n");
    	return -1;
  	}

      buf[numbytes] = '\0';
      
      return numbytes; 

}

/*************************************************************************************
  Function name : init_server ()
  Description   : Initialize net socket parametter of the server
  Parametter    : int *fd, struct sockaddr_in *server
  Return        : 0 if success, -1 failed
  Key word      : @Function init_server
  Author        : kamejoko80             
*************************************************************************************/
int init_server (int *fd, struct sockaddr_in *server)
{
  /* Create file descriptors */	
  if ((*fd = socket (AF_INET, SOCK_STREAM, 0)) == -1 )
  { 
    printf ("socket() error\n");
    return -1;
  }	
  
  /* Set server informations */
  server->sin_family = AF_INET;         
  server->sin_port = htons (PORT);        
  server->sin_addr.s_addr = INADDR_ANY;    /* INADDR_ANY puts your IP address automatically */   
  memset (&(server->sin_zero), 0, 8);      /* zero the rest of the structure                */
  
  /* Calls bind() */
  if (bind (*fd, (struct sockaddr*) server, sizeof (struct sockaddr)) == -1)
  { 
    printf ("bind() error\n");
    return -1;
  }
  
  /* Listen to connection of the client */
  if (listen (*fd, 1) == -1)
  {  
      printf ("listen() error\n");
      return -1;
  }     
  
  return 0; /* successful */ 
}

/*************************************************************************************
  Function name : accept_client ()
  Description   : Wait for accept the connection of the client
  Parametter    : int *fd, struct sockaddr_in *server
  Return        : 0 if success, -1 failed
  Key word      : @Function init_server
  Author        : kamejoko80             
*************************************************************************************/
int accept_client (int *cfd, int sfd,  struct sockaddr_in *client)
{
	unsigned int sin_size = sizeof (struct sockaddr_in);
	
	if ((*cfd = accept (sfd, (struct sockaddr *) client, &sin_size)) == -1)
    { 
	  printf("accept() error\n");
      return -1;
    }
    
    return 0;   
}

void web_server_task(void *pvParameters)
{
    int sfd, cfd;               /* file descriptors             */
    struct sockaddr_in server;  /* server's address information */
    struct sockaddr_in client;  /* client's address information */
    char buf[MAXDATASIZE];      /* buf will store received text */
    int len;
    
    /* Init server */
    if (init_server (&sfd, &server) == -1)
    {
	  return;
    }       

    while(1) 
    {
        if (accept_client (&cfd, sfd, &client) == -1)
        {
            return;
        }   
	     
        /* Prints client's IP                 */
        printf("You got a connection from %s\n",inet_ntoa(client.sin_addr)); 
	         
        /* Receive request */
        if ((len = recv_msg (cfd, buf)) == -1)
        {
            printf("recv_msg() error\n");
            return;
        }        
        
        printf("\r\n");
        printf("Data:\r\n");
        printf("%s\r\n", buf);
        
        if (
             (len >= 5 && buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T' && buf[3] == ' ' && buf[4] == '/' ) ||
             (len >= 5 && buf[0] == 'P' && buf[1] == 'O' && buf[2] == 'S' && buf[3] == 'T')
           )
        {
            printf("HTTP Request OK\n");
            
            /* send to the client welcome message */ 
            send_msg (cfd, (char *)http_html_hdr);
            send_msg (cfd, (char *)http_index_html);
        }              
                
        /*  close cfd */
        close(cfd); 
    }
}

void user_init(void)
{
    sdk_uart_div_modify(0, UART_CLK_FREQ / 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&web_server_task, (signed char *)"web_server", 2048, NULL, 2, NULL);
}

