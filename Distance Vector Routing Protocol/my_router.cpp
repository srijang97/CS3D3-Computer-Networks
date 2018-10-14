//Things left-:
//write router-start script (done)
//Write router data to files (done)
//Handle router disapperance and re-appearance (write script for that)
//Pass packet and see it works properly (write inject script)
//we can improve the reset function

//TinyAODV (we didnt use this, reasons below)
//Didnt implement AODV. didnt see the need for it, especially with such a small no. of routers in our project. that would work better in a real 
//life network. 1) Only affected nodes are informed. 2)AODV reduces the networkwide broadcasts to the extent possible. 
//3)Whenever routes are not used -> get expired -> Discarded
//❍ Reduces stale routes
//❍ Reduces need for route maintenance
//AODV discovers routes as and when necessary
//1. Does not maintain routes from every node to every other
//2. Routes are maintained just as long as necessary
//we didnt need such high level sophisticated network, and so didnt implement aodv.


#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h> 
#include <string.h> 
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <time.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>

#define buffer_size 2048
#define no_of_routers 6

using namespace std;

struct dv_info
{
public: 
 int nextRouterPort() const { return (dead() ? -1 : m_nextRouterPort); }
 char nextRouterName() const { return (dead() ? '0' : m_nextRouterName); }
 int cost() const { return (dead() ? -1 : m_cost); }
 bool dead() const { return m_dead; }

 void setnextRouterPort(int n) { m_nextRouterPort = n; }
 void setnextRouterName(char n) { m_nextRouterName = n; }
 void setCost(int c) { m_cost = c; }
 void setalive() { m_dead = false; }
 void setdead() { m_dead = true; }
private:
 bool m_dead;
 int m_nextRouterPort; // port number of next hop router
 char m_nextRouterName;
 int m_cost; // link cost to destination
};

struct neighbour_info
{
 char name;
 int portno;
 timespec startTime;
 sockaddr_in addr;
};

class DV
{
public:
 DV() {}
 DV(const char *file, const char *my_id);
 ~DV() {}
 
 void reset(char dead);
 dv_info *getDetails() { return m_Details; }
 int getSize() const { return sizeof(m_Details); }
 char getmyID() const { return nameOf(m_my_id); }
 void updateTable(const void *advertisement, char src);
 dv_info routeTo(const char dest) const { return m_Details[indexOf(dest)]; };
 std::vector<neighbour_info> neighbours() const { return m_neighbours; };
 int portNoOf(char router);
 char nameOf(int index) const;
 int indexOf(char router) const;
 void initMyaddr(int portno);
 sockaddr_in myaddr() const { return m_myaddr; }
 void aliveTime(neighbour_info &n);
 bool deadTimer(neighbour_info &n) const;
 int port() { return portNoOf(getmyID()); }

private:
 // member variables
 int m_my_id; 
 int m_size;
 dv_info m_Details[no_of_routers]; // each router's distance vectors
 dv_info m_Details_backup[no_of_routers]; // initial distance vectors (for resetting)
 std::vector<neighbour_info> m_neighbours; // port numbers of my_id's neighbours
 sockaddr_in m_myaddr;
 std::map<char, int> m_portnos;

 int min(int initialCost, int startToPacketcost, int PacketToDestCost, char initialName, char newName, bool &changed) const;
 void print(dv_info dv[], char name, std::string msg, bool timestamp) const;
};

struct header
{
 int type;
 char source;
 char dest;
 int length;
};

enum type
{
 TYPE_DATA, TYPE_ADVERTISEMENT, TYPE_WAKEUP, TYPE_RESET
};

void *createPacket(int type, char source, char dest, int payloadLength, void *payload);
header getHeader(void *packet);
void *getPayload(void *packet, int length);
void multicast(DV &dv, int socketfd);
void wakeSelfUp(DV &dv, int socketfd, int type, char source = 0, char dest = 0, int payloadLength = 0, void *payload = 0);

int main(int argc, char **argv)
{
 // check for errors
 if (argc < 3){
 perror("Not enough arguments.\nUsage: ./my_router <initialization file> <router name>\n");
 return 0;
 } 

 DV dv(argv[1], argv[2]);

 vector<neighbour_info> neighbours = dv.neighbours();

 int myPort = dv.portNoOf(argv[2][0]); // my port

 dv.initMyaddr(myPort);
 sockaddr_in myaddr = dv.myaddr();

 socklen_t addrlen = sizeof(sockaddr_in); // length of addresses

 // create a UDP socket
 int socketfd; // our socket
 if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
 perror("cannot create socket\n");
 return 0;
 }
 
 // bind the socket to localhost and myPort
 if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
 perror("bind failed");
 return 0;
 }

 // send a data packet to router A
 if (dv.getmyID() == 'H'){
 char data[100];
 memset(data, 0, 100);
 cin.getline(data, 100);
 for (int i = 0; i < neighbours.size(); i++){
 if (neighbours[i].name == 'A'){
 void *dataPacket = createPacket(TYPE_DATA, dv.getmyID(), 'D', strlen(data), (void*)data);
 sendto(socketfd, dataPacket, sizeof(header) + dv.getSize(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));

 // print info
 header h = getHeader(dataPacket);
 printf("Sent data packet\n");
 printf("Type: data\n");
 printf("Source: %c\n",h.source);
 printf("Destination: %c\n", h.dest);
 printf("Length of packet: %ld\n",(sizeof(header) + h.length));
 printf("Length of payload: %ld\n", h.length);
 cout << "Payload: " << data << endl;

 free(dataPacket);
 }
 }
 exit(0);
 }

//fork() can be used to create a new process, known as a child process. This child is initially a copy of the the parent, but can be used to run 
//a different branch of the program or even execute a completely different program. After forking, child and parent processes run in parallel. 
//Any variables local to the parent process will have been copied for the child process, so updating a variable in one process will not affect 
//the other
//we forked the program and maintained two separate processes for each router. While the parent process uses a receive-and-then-send loop, 
//the child process periodically sends a packet to the parent process. That way, the parent process can wake up periodically and send 
//advertisements to it neighbors. 
 
 // distance vector routing
 int thread_id = fork();
 if (thread_id < 0){
 perror("fork failed");
 return 0;
 }
 else if (thread_id == 0){ // send to each neighbour periodically
 for (;;){
 // periodically wake up parent process //fork is use to create a child process to wake itself up and then send dv to each neighbour
 wakeSelfUp(dv, socketfd, TYPE_WAKEUP);
 sleep(1);
 }
 }
 else{ // listen for advertisements
 void *rcvbuf = malloc(buffer_size);
 sockaddr_in remaddr;
 for (;;){
 memset(rcvbuf, 0, buffer_size);
 int recvlen = recvfrom(socketfd, rcvbuf, buffer_size, 0, (struct sockaddr *)&remaddr, &addrlen);
 
 header h = getHeader(rcvbuf);
 void *payload = getPayload(rcvbuf, h.length);
 switch(h.type){
 case TYPE_DATA:
 cout << "Received data packet" << endl;
 time_t rawtime;
 time(&rawtime);
 cout << "Timestamp: " << ctime(&rawtime);
 cout << "ID of source neighbour_info: " << h.source << endl;
 cout << "ID of destination neighbour_info: " << h.dest << endl;
 cout << "UDP port in which the packet arrived: " << myPort << endl;
 if (h.dest != dv.getmyID()){ // only forward if this router is not the destination
 if (dv.routeTo(h.dest).nextRouterPort() == -1){
 cout << "Error: packet could not be forwarded" << endl;
 }
 else{
 cout << "UDP port along which the packet was forwarded: " << dv.routeTo(h.dest).nextRouterPort() << endl;
 cout << "ID of neighbour_info that packet was forwarded to: " << dv.routeTo(h.dest).nextRouterName() << endl;
 void *forwardPacket = createPacket(TYPE_DATA, h.source, h.dest, h.length, (void*)payload);
 for (int i = 0; i < neighbours.size(); i++){
 if (neighbours[i].name == dv.routeTo(h.dest).nextRouterName())
 sendto(socketfd, forwardPacket, sizeof(header) + dv.getSize(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));
 }
 free(forwardPacket);
 }
 cout << endl;
 }
 else{
 char data[100];
 memset(data, 0, 100);
 memcpy((void*)data, payload, h.length);
 cout << "Data payload: " << data << endl << endl;
 }
 break;
 case TYPE_ADVERTISEMENT:
 dv_info Details[no_of_routers];
 memcpy((void*)Details, payload, h.length);
 for (int i = 0; i < neighbours.size(); i++){
 if (neighbours[i].name == h.source)
 dv.aliveTime(neighbours[i]);
 }
 dv.updateTable(payload, h.source);
 break;
 case TYPE_WAKEUP: // perform periodic tasks
 for (int i = 0; i < neighbours.size(); i++){
 neighbour_info curneighbour = neighbours[i];
 if ((dv.getDetails()[dv.indexOf(curneighbour.name)].cost() != -1) && dv.deadTimer(neighbours[i])){
 wakeSelfUp(dv, socketfd, TYPE_RESET, dv.getmyID(), neighbours[i].name, dv.getSize() / sizeof(dv_info) - 2);
 }
 }
 multicast(dv, socketfd);
 break;
 case TYPE_RESET:
 int hopcount = (int)h.length - 1;
 dv.reset(h.dest);
 if (hopcount > 0){
 void *forwardPacket = createPacket(TYPE_RESET, dv.getmyID(), h.dest, hopcount, (void*)0);
 for (int i = 0; i < neighbours.size(); i++){
 if (neighbours[i].name != h.source)
 sendto(socketfd, forwardPacket, sizeof(header), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));
 }
 }
 break;
 }
 }
 free(rcvbuf);
 }
 }


DV::DV(const char *file, const char *my_id){
 fstream topology(file);

 string line; // current line of file
 string field; // current token (to be put into entry's field)
 char my_idName = my_id[0]; // name of my_id
 m_my_id = indexOf(my_id[0]);

 // initialize m_Details
 for (int dest = 0; dest < no_of_routers; dest++){
 m_Details[dest].setnextRouterName('0');
 m_Details[dest].setnextRouterPort(-1);
 m_Details[dest].setCost(-1);
 m_Details[dest].setalive();
 }

 while (getline(topology, line)){ // parse file line by line
 stringstream linestream(line);
 dv_info entry;

 entry.setalive();

 // source router
 getline(linestream, field, ',');
 char name = field[0];

 // destination router
 getline(linestream, field, ',');
 int dest = indexOf(field[0]);
 neighbour_info n;
 n.name = field[0];
 entry.setnextRouterName(field[0]);

 // destination port number
 getline(linestream, field, ',');
 int port = atoi(field.c_str());
 entry.setnextRouterPort(port);
 n.portno = port;

 memset((char *)&n.addr, 0, sizeof(n.addr));
 n.addr.sin_family = AF_INET;
 n.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 n.addr.sin_port = htons(port);

 // link cost
 getline(linestream, field, ',');
 entry.setCost(atoi(field.c_str()));

 if (my_idName == 'H'){
 int i;
 for (i = 0; i < m_neighbours.size(); i++){
 if (m_neighbours[i].name == n.name)
 break;
 }
 if (i == m_neighbours.size())
 m_neighbours.push_back(n);
 }
 else if (name == my_idName){
 aliveTime(n);
 m_neighbours.push_back(n); // store neighbour
 m_Details[dest] = entry;
 }

 m_portnos[n.name] = n.portno;
 }

 // special port number for sending data packet
 m_portnos['H'] = 11111;

 memcpy((void*)m_Details_backup, (void*)m_Details, sizeof(m_Details));

 if (nameOf(m_my_id) != 'H')
 print(m_Details, nameOf(m_my_id), "Initial routing table", true);
}

void DV::reset(char dead){
 for (int i = 0; i < m_neighbours.size(); i++){
 if (m_neighbours[i].name == dead){
 if (m_Details_backup[indexOf(dead)].cost() != -1)
 m_Details_backup[indexOf(dead)].setdead();
 }
 }
 memcpy((void*)m_Details, (void*)m_Details_backup, sizeof(m_Details));
 print(m_Details, nameOf(m_my_id), "Reset routing table", true);
}

// updateTable this router's distance vector based on received advertisement from source
// return false if this router's distance vector was not changed
void DV::updateTable(const void *advertisementBuf, char source){
 dv_info originalDetails[no_of_routers];
 memcpy((void*)originalDetails, (void*)m_Details, sizeof(m_Details));

 bool changedDV = false;

 int intermediate = indexOf(source);
 if (m_Details_backup[intermediate].dead()){
 m_Details_backup[intermediate].setalive();
 m_Details[intermediate].setalive();

 changedDV = true;
 }

 // load advertised distance vector
 dv_info advertisement[no_of_routers];
 memcpy((void*)advertisement, advertisementBuf, sizeof(advertisement));
 
 // recalculate my_id's distance vector
 for (int dest = 0; dest < no_of_routers; dest++){
 if (dest == m_my_id)
 continue;
 bool changedEntry = false;
 char a=m_Details[dest].nextRouterName();
 m_Details[dest].setCost(min(m_Details[dest].cost(), m_Details[intermediate].cost(), advertisement[dest].cost(), m_Details[dest].nextRouterName(), source, changedEntry));
 if (changedEntry){
 changedDV = true;
 if(m_Details[indexOf(source)].nextRouterName()!= source){
 m_Details[dest].setnextRouterPort(m_Details[indexOf(source)].nextRouterPort());
 m_Details[dest].setnextRouterName(m_Details[indexOf(source)].nextRouterName());
 if(m_Details[dest].nextRouterName()==a){
 changedDV=false;
 }
 }
 else{ 
 m_Details[dest].setnextRouterPort(portNoOf(source));
 m_Details[dest].setnextRouterName(source);
 }
 }
 }
 m_Details[intermediate].setCost(advertisement[m_my_id].cost());

 if (changedDV){
 print(originalDetails, nameOf(m_my_id), "Change detected!\nRouting table before change", true);
 print(advertisement, source, "DV that caused the change", false);
 print(m_Details, nameOf(m_my_id), "Routing table after change", false);
 }
}

// return index of router
int DV::indexOf(char router) const{
 return router - 'A';
}

// return name of indexed router
char DV::nameOf(int index) const{
 return (char)index + 'A';
}

// return port number of router
int DV::portNoOf(char router){
 return m_portnos[router];
}

void DV::initMyaddr(int portno){
 memset((char *)&m_myaddr, 0, sizeof(m_myaddr));
 m_myaddr.sin_family = AF_INET;
 m_myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 m_myaddr.sin_port = htons(portno);
}

void DV::aliveTime(neighbour_info &n){
 clock_gettime(CLOCK_MONOTONIC, &n.startTime);
}

bool DV::deadTimer(neighbour_info &n) const{
 timespec tend={0,0};
 clock_gettime(CLOCK_MONOTONIC, &tend);

 if (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)n.startTime.tv_sec + 1.0e-9*n.startTime.tv_nsec) > 10)
 return true;
 else
 return false;
}

//-----------------
// HELPER FUNCTIONS
//-----------------

// return minimum cost and set changed flag
int DV::min(int initialCost, int startToPacketcost, int PacketToDestCost, char initialName, char newName, bool &changed) const {
 int newCost = startToPacketcost + PacketToDestCost;

 if(m_Details[newName].nextRouterName()!=initialName){
 if (startToPacketcost == -1 || PacketToDestCost == -1){
 return initialCost;
 }
 else if (initialCost == -1 || newCost < initialCost){
 changed = true;
 return newCost;
 }
 else if (initialCost == newCost){
 if (initialName <= newName){
 changed = false;
 return initialCost;
 } 
 else{
 changed = true;
 return newCost;
 }
 }
 else{
 return initialCost;
 }
 }
 return initialCost;
}

// print a DV
// format: source, destination, port number of nexthop router, cost to destination
void DV::print(dv_info dv[], char name, string msg, bool timestamp) const {
 if (timestamp){
 time_t rawtime;
 time(&rawtime);
 cout << ctime(&rawtime);
 }
 cout << msg << ": " << name << endl;
 cout << "dst nexthop cost" << endl;
 for (int dest = 0; dest < no_of_routers; dest++){
 cout << " " << nameOf(dest) << " ";
 if (dv[dest].nextRouterPort() == -1)
 cout << " ";
 cout << dv[dest].nextRouterPort() << " ";
 if (dv[dest].cost() != -1)
 cout << " ";
 cout << dv[dest].cost();
 cout << endl;
 }
 cout << endl;
}

// create a packet with header and payload
void *createPacket(int type, char source, char dest, int payloadLength, void *payload){
 int allocatedPayloadLength = payloadLength;
 if ((type != TYPE_DATA) && (type != TYPE_ADVERTISEMENT))
 allocatedPayloadLength = 0;

 // create empty packet
 void *packet = malloc(sizeof(header)+allocatedPayloadLength);

 // create header
 header h;
 h.type = type;
 h.source = source;
 h.dest = dest;
 h.length = payloadLength;

 // fill in packet
 memcpy(packet, (void*)&h, sizeof(header));
 memcpy((void*)((char*)packet+sizeof(header)), payload, allocatedPayloadLength);

 return packet;
}

// extract the header from the packet
header getHeader(void *packet){
 header h;
 memcpy((void*)&h, packet, sizeof(header));
 return h;
}

// extract the payload from the packet
void *getPayload(void *packet, int length){
 void *payload = malloc(length);
 memcpy(payload, (void*)((char*)packet+sizeof(header)), length);
 return payload;
}

// multicast advertisement to all neighbours
void multicast(DV &dv, int socketfd){
 vector<neighbour_info> neighbours = dv.neighbours();
 for (int i = 0; i < neighbours.size(); i++){
 void *sendPacket = createPacket(TYPE_ADVERTISEMENT, dv.getmyID(), neighbours[i].name, dv.getSize(), (void*)dv.getDetails());
 sendto(socketfd, sendPacket, sizeof(header) + dv.getSize(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));
 free(sendPacket);
 }
}

// periodically wake yourmy_id up to multicast advertisement
void wakeSelfUp(DV &dv, int socketfd, int type, char source, char dest, int payloadLength, void *payload){
 void *sendPacket = createPacket(type, source, dest, payloadLength, payload);
 sockaddr_in destAddr = dv.myaddr();
 sendto(socketfd, sendPacket, sizeof(header), 0, (struct sockaddr *)&destAddr, sizeof(sockaddr_in));
 free(sendPacket);
}