#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>


#include "httpRequest.h"
#define MESSAGE_SIZE 1000000



using namespace std; //apparently not good practice to use namespace std

char *host, *path;

int StartsWith(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}
  
void parseUrl(char url[])
{
  host = new char(strlen(url)); 
  path = new char(strlen(url));
 
  if(StartsWith(url, "http"))
    strcpy(url, &url[6]);  // removing the http:// part from string

  const char delim[2] = "/";
  strcpy(host,strtok(url,delim)); //gets the host in the host string
  strcpy(path, "/");
  char *temp = new char(strlen(url));
  strcpy(temp,url);
  while(temp!=NULL)   //puts evrything in page until the string ends
    {
      temp = strtok(NULL, delim);
      if(temp!=NULL)
       strcat(path,temp);
    }

  printf("path %s\n",path);

}

int main(int argc, char **argv)
{
	int create_socket  = socket(AF_INET, SOCK_STREAM, 0); // create socket
  
  //create connection with the server (using sample server.cpp file so using same struct)
	struct sockaddr_in serveradder;
  serveradder.sin_family = AF_INET;
  serveradder.sin_port = htons(4000);     // short, network byte order
 	serveradder.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serveradder.sin_zero, '\0', sizeof(serveradder.sin_zero));

  parseUrl(argv[1]);
  
  int connecterr = connect(create_socket, (struct sockaddr *)&serveradder, sizeof(serveradder));
  if(connecterr == -1){
    cout << "failed to connect to server" << endl; 
    return 2;
  }
  
  struct sockaddr_in clientAdder;
  socklen_t lengthCAdd = sizeof(clientAdder);
  int getnameerr = getsockname(create_socket, (struct sockaddr *)&clientAdder, &lengthCAdd); 
  if(getnameerr == -1){
    cout << "failed to get socket name" << endl;
    return 3;
  }

  char ipaddress[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAdder.sin_family, &clientAdder.sin_addr, ipaddress, sizeof(ipaddress));
  cout << "Setting up connection from: " << ipaddress << ":" <<  ntohs(clientAdder.sin_port) << endl;
  
  
  char buffer[MESSAGE_SIZE] = {0}; //males sure buffer matches recv or error occurs
  int senderr;
  int recverr;

  httpRequest request("GET", host, path);
 
	
  //while(request.fullRequest != "close"){
    
  senderr = send(create_socket, request.fullRequest.c_str(), request.fullRequest.size(), 0);
  if(senderr == -1){
  	cout << "sending error " << endl;
  }
    
  
  recverr = recv(create_socket, buffer, MESSAGE_SIZE, 0);

  
  
  if(recverr == -1){
      cout << "error receiving" << endl;
    }
    
  //cout << buffer << endl;
  
  if(buffer[9] == '4' && buffer[10] == '0' && buffer[11] == '4'){ //check status code in buffer
  	cout << "404 Not found" << endl;
  }
  
  else if(buffer[9] == '4' && buffer[10] == '0' && buffer[11] == '0'){
  	cout << "400 Bad Request" << endl;
  }
  
  else if(buffer[9] == '2' && buffer[10] == '0' && buffer[11] == '0') {
  	cout << "200 OK" << endl;
  	ofstream myfile;
    char makestring[MESSAGE_SIZE];
    int count = 0;
    while(buffer[count]!= '\0')
    {
       makestring[count] = buffer[count];
       count++;
 		}
 		char* http[2];
 		http[0] = strtok(makestring, "\r\n\r\n");
 		http[1] = strtok(NULL, "");
 		
 		string str(http[1]);
 		myfile.open("httpfile.html");
 		myfile << str;
 		myfile.close();
  }
  
  else {
  	cout << "unknown status code" << endl;
  }
  
  /*if(buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T'){
 		for(int i = 0; i<10; i++){
  		while(buffer[i] != ' '){
				r.method[i] = buffer[i];
			}
			int j = i;
			break;
  	} 
  
  	for(int i = j; i<256; i++){
  		while(buffer[i] != ' '){
  			r.URL[i] = buffer[i];
  		}
  		j = i;
  		break;
  	}
  
  	for(int i = j; i<256; i++){
  		while(buffer[i] != ' '){
  			r.HTTPversion[i] = buffer[i];
  		}
  		j = i;
  		break;
  	}
  
  	for(int i = j; i<256; i++){
  			r.headers[i] = buffer[i];
  	}
  }
  
  
  
    
  cout << "message: " ;
  cin >> message;
  
}*/
//}
  
	close(create_socket);

	return 0;
}
