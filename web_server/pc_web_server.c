/* http_get - Retrieves a web page over HTTP GET.
 *
 * See http_get_ssl for a TLS-enabled version.
 *
 * This sample code is in the public domain.,
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
          
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 80
#define MAXDATASIZE 1024

typedef struct __HTML_Input_Tag
{
    char *name;
    char *type;
    char *value;  
    char *size;
    char *maxlength;
    char *style;
    char *extra;
}HTML_Input_Tag;


const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

const static char http_wb_begin[] ="<html> \
<head> \
<meta http-equiv=\"Content-Type\" content=\"text/html;charset=ISO-8859-1\"> \
<title>NETWORK CONFIGURATION</title> \
</head> \
<body bgcolor=\"#ffffff\"> \
<h1>NETWORK SETTING</h1> \
<FORM ACTION=\"\" METHOD=\"GET\"> \
<p><span style=\"font-size: 20px\">SSID&nbsp;</span>&nbsp;";

const static char http_wb_end[] = "</body></html>";

HTML_Input_Tag Input_01 = {
    .name  = "ssid",
    .type  = "text",
    .value = "",
    .size = "9",
    .maxlength = "50",
    .style = "height: 22px; width: 152px",
    .extra = "</p><p align=\"left\"><span style=\"font-size: 20px\">PASS</span>&nbsp;" 
};

HTML_Input_Tag Input_02 = {
    .name  = "pass",
    .type  = "password",
    .value = "",
    .size = "9",
    .maxlength = "50",
    .style = "height: 22px; width: 152px",
    .extra = "</p><p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" 
};

HTML_Input_Tag Button_01 = {
    .name  = "Save",
    .type  = "submit",
    .value = "Save",
    .size = "",
    .maxlength = "",
    .style = "height: 22px; width: 68px",
    .extra = "&nbsp;&nbsp;&nbsp;" 
};

HTML_Input_Tag Button_02 = {
    .name  = "Reboot",
    .type  = "submit",
    .value = "Reboot",
    .size = "",
    .maxlength = "",
    .style = "height: 22px; width: 68px",
    .extra = "</p>" 
};

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

#if 0 /* Test message output */    
    
    printf("%s", msg);
    
#else
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
#endif    
    
    return 0; /* successful */
}

void print_tag(int fd, HTML_Input_Tag *pTag)
{
    send_msg(fd, " <input ");
    send_msg(fd, " name=\"");
    send_msg(fd, pTag->name);
    send_msg(fd, "\"");
    send_msg(fd, " type=\"");
    send_msg(fd, pTag->type);
    send_msg(fd, "\"");    
    send_msg(fd, " value=\"");
    send_msg(fd, pTag->value);
    send_msg(fd, "\"");    
    send_msg(fd, " maxlength=\"");
    send_msg(fd, pTag->maxlength);
    send_msg(fd, "\"");    
    send_msg(fd, " style=\"");
    send_msg(fd, pTag->style);
    send_msg(fd, "\"");
    send_msg(fd, " />");    
    send_msg(fd,pTag->extra);    
}

void print_web_page(int fd)
{
    send_msg (fd, (char *)http_html_hdr);
    send_msg (fd, (char *)http_wb_begin);
    print_tag(fd, &Input_01);
    print_tag(fd, &Input_02);
    print_tag(fd, &Button_01);
    print_tag(fd, &Button_02);
    send_msg (fd, (char *)http_wb_end); 
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

/*************************************************************************************
                     MAIN FUNCTION
*************************************************************************************/

int main (int argc, char *argv[])
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
            //send_msg (cfd, (char *)http_html_hdr);
            //send_msg (cfd, (char *)http_index_html);
            print_web_page(cfd);
            
        }              
                
        /*  close cfd */
        close(cfd); 
    }
}



