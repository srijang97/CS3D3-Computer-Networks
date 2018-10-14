#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "httpResponse.h"


#define BACKLOG 10
#define MESSAGE_SIZE 1000000
using namespace std;

bool writeDataToClient(int socket, const char *data, int datalen)
{
    const char *pdata = data;

    while (datalen > 0){
        int numSent = send(socket, pdata, datalen, 0);
        if (numSent <= 0){
            if (numSent == 0){
                printf("The client was not written to: disconnected\n");
            } else {
                perror("The client was not written to");
            }
            return false;
        }
        pdata += numSent;
        datalen -= numSent;
    }

    return true;
}

bool writeStrToClient(int sckt, const char *str)
{
    return writeDataToClient(sckt, str, strlen(str));
}

void respond(int n, char buffer[MESSAGE_SIZE], int *sock_clients, char* path)
{
	char mesg[MESSAGE_SIZE], *reqline[3], data_to_send[MESSAGE_SIZE];
	int rcvd, fd, bytes_read;

    memset(mesg, 0, MESSAGE_SIZE);
	strcpy(mesg, buffer);

	printf("%s", mesg);
	reqline[0] = strtok (mesg, " \t\n");
	if ( strncmp(reqline[0], "GET\0", 4)==0 )
	{
		reqline[1] = strtok (NULL, " \t");
		reqline[2] = strtok (NULL, " \t\n");
		if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
		{
            httpResponse response(400, "");
            writeDataToClient(sock_clients[n], response.fullResponse.c_str(), response.getResponseLength());
		}
		else
		{
			if ( strncmp(reqline[1], "/\0", 2)==0 )
				reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...
            strcpy(&path[strlen(path)], reqline[1]);
            printf("file: %s\n", path);

			if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
			{
                bytes_read=read(fd, data_to_send, MESSAGE_SIZE);

                httpResponse response(200, data_to_send);

                if (!writeDataToClient(sock_clients[n], response.fullResponse.c_str(), response.getResponseLength())){
                    close(sock_clients[n]);
                }  
            }
            
			else  {
                httpResponse response(404, "");
                writeDataToClient(sock_clients[n], response.fullResponse.c_str(), response.getResponseLength());
            }
		}
    }
    
    	//Closing SOCKET
	shutdown(sock_clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(sock_clients[n]);
    sock_clients[n]=0;

    return;
}




void *get_in_addr(struct sockaddr *sa)
{
 if (sa->sa_family == AF_INET) {
 return &(((struct sockaddr_in*)sa)->sin_addr);
 }
 return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char** argv)
{
  struct addrinfo hints, *serverinfo, *p;
  struct sockaddr_storage client_addr;
  char *path;
  int addr;
  int sock, sock_new, sock_clients[30], max_clients = 30, max_sd, sd, activity, valread;
  fd_set readfds;
  char buffer[1025];
  int yes = 1;

  path = new char(strlen(argv[3]));
  strcpy(path, argv[3]);

  for (int i = 0; i < max_clients; i++) 
    {
        sock_clients[i] = 0;
    }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;


  if((addr = getaddrinfo(argv[1], argv[2], &hints, &serverinfo))!=0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr));
    return 1;   
  }
  p = serverinfo;

  while( p != NULL)
  {
    if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))==-1) 
    {
      perror("Error: socket");
      continue;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
    {
      perror("Error: setsockopt");
      exit(1);
    }

    if(bind(sock, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sock);
      perror("Error: bind");
      p = p->ai_next;
      continue;
    }

    break; 
  }

  if(p==NULL)
  {
    fprintf(stderr, "Failed to bind\n");
    exit(1);
  } 

  if(listen(sock, BACKLOG)== -1)
  {
    perror("Error: Listen");
    exit(1);
  }

  int addrlen = sizeof(client_addr);

  cout<<"Waiting for connections.."<<endl;

  while(1) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
  
        //add master socket to set
        FD_SET(sock, &readfds);
        max_sd = sock;
         
        //add child sockets to set
        for (int i = 0 ; i < max_clients ; i++) 
        {
            //socket descriptor
            sd = sock_clients[i];
             
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);
             
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
   
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
          
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(sock, &readfds)) 
        {
            if ((sock_new = accept(sock, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(1);
            }
          
            //inform user of socket number - used in send and receive commands
            //printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));
        
            //send new connection greeting message
            /*if( send(sock_new, message, strlen(message), 0) != strlen(message) ) 
            {
                perror("send");
            }*/
              
            puts("Welcome message sent successfully");
              
            //add new socket to array of sockets
            for (int i = 0; i < max_clients; i++) 
            {
                //if position is empty
                if( sock_clients[i] == 0 )
                {
                    sock_clients[i] = sock_new;
                    printf("Adding to list of sockets as %d\n" , i);
                     
                    break;
                }
            }
        }
          
        //else its some IO operation on some other socket :)
        for (int i = 0; i < max_clients; i++) 
        {
            sd = sock_clients[i];
              
            if (FD_ISSET( sd , &readfds)) 
            {
                memset(path, 0, strlen(path));
                strcpy(path, argv[3]);
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&client_addr , (socklen_t*)&addrlen);
                    //printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));
                      
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    sock_clients[i] = 0;
                }
                  
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    respond(i, buffer, sock_clients, path);
                }
                
            }
            
        }
      }
      return 0;
      
   


}
