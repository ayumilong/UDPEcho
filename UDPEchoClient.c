#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "Practical.h"


const unsigned int DEFAULT_ARRAY_LEN = 10000; //The default length of the array that stores all the rtts
const unsigned int TIME_LEN = 50; //The length of timestamp
const unsigned int TIMEOUT_SECS = 2; // Seconds between retransmits
const unsigned int MAXBURSTSIZE = 10000;
double totalTime; //The total excution time of client
size_t testDuration; //Indicate how long the user wants the client to excute
size_t pacNum; //Indicate the packet number
size_t timeoutNum; //The number of timeout packets number
double minRttTime; //Minimum rtt time
double maxRttTime;  //Maximum rtt time
double avgRttTime;  //Average rtt time
double rttMeanDev; //The mean deviation of rtt
double *allRtt; //Record all the rtts of each packet
unsigned int curLen; //The current length of allRtt
size_t burstSize; //Indicate how many packtes there are in each burst
size_t burstCount; //Indicate the probably number of bursts
size_t *burstTimeoutNum; //Record each burst's timeout number
double *totalTimes; //Record each burst's excution time, say from the time sending the first packet to the time recieving the last packet
size_t burstSequence; //Indicate the burst's serial number
size_t pos; //Indicate how many packets has been recieved in the current burst

struct addrinfo *servAddr; // list of server addresses
int sock; //Socket discriptor
char *dstAddress; 
char *dstPortNumber; 

void signal_handler(int sig);  //This functttion is clearly the signal handle function
double getTime();  //Get time in seconds
void getCurTime(char *bufPtr); //Get time in readable format
double calMeanDev(const double *allRtt, const unsigned int length, const double avgRttTime); //Caculate mean deviation
void printEachBurst(); //Print the average loss rate of all the bursts
void printStatistics(); //Print the statistic information of client

int main(int argc, char *argv[]){
  	if (argc != 7){ // Test for correct number of arguments
    	DieWithUserMessage("Parameter(s)","<dstAddress>  <dstPortNumber> <IterationDelay> <dataSize> <burstSize> <TestDuration>");
  	}
  	dstAddress = argv[1];     // First arg: The destination IP address in dotted decimal number or in readable name format 
  	dstPortNumber = argv[2]; // Second arg: The destination's port number

  	double iterDelay = atof(argv[3]); //Third arg: The delay(in seconds) between two packets to the server
  	long timeInterval = iterDelay*1000*1000; //The delay(in microseconds) 
  	ssize_t dataSize = atoi(argv[4]) + 1;//Fourth arg: The total amount of data in each UDP datagram
 
  	if (dataSize >= MAXSTRINGLENGTH || dataSize <= 1){ // Check input data size
    	DieWithUserMessage("dataSize not suitable", "should be (0, 64K)");
  	}
  	if(dataSize < 4){ //The minimum size of data is 4, because we need to send a sequence number and it possess 4 bytes
		dataSize = 4;
  	}

  	burstSize = atoi(argv[5]);
  	if(burstSize > MAXBURSTSIZE || burstSize <= 0){
    	DieWithUserMessage("burstSize not suitable", "should be (0, 10000]");
  	} 
  	testDuration = atoi(argv[6]);
  	if(testDuration <= 0){
    	DieWithUserMessage("testDuration not suitable", "should be larger than 0");
  	}


  	if(iterDelay != 0){
  		burstCount = ceil(testDuration / iterDelay); //The rough count of burst times
  	}else{
	  	burstCount = INT_MAX/1024;
  	}
  	char echoString[dataSize]; //The data that will be sent to server
  	memset(echoString, '0', dataSize);
  	pacNum = 0; //Indicate the packet number
  	timeoutNum = 0; //The number of timeout packets number
  	burstTimeoutNum = (size_t *)malloc(burstCount * sizeof(size_t)); //Record each burst's time out count
  	if(burstTimeoutNum == NULL){
     	DieWithSystemMessage("Alocate memorry failed!");
  	}
  	memset(burstTimeoutNum, 0, burstCount * sizeof(size_t));
  	totalTimes = (double *)malloc(burstCount * sizeof(double)); //Record each burst's total time 
  	if(totalTimes == NULL){
     	DieWithSystemMessage("Alocate memorry failed!");
  	}
  	memset(totalTimes, 0, burstCount * sizeof(double));
  	minRttTime = DBL_MAX; //Minimum rtt time
  	maxRttTime = DBL_MIN;  //Maximum rtt time
  	avgRttTime = 0.0;  //Average rtt time
  	double rttTime[burstSize]; //Rtt time for each packets burst
  	memset(rttTime, 0, burstSize * sizeof(double));
  	rttMeanDev = 0.0; //The mean deviation of rtt
  	allRtt=(double *)malloc(DEFAULT_ARRAY_LEN * sizeof(double)); //Store all the rtt
  	if(allRtt == NULL){
     	DieWithSystemMessage("Alocate memorry failed!");
  	}
  	memset(allRtt, 0, DEFAULT_ARRAY_LEN * sizeof(double));
  	curLen=DEFAULT_ARRAY_LEN; //The current length of allRtt

  	// Tell the system what kind(s) of address info we want
  	struct addrinfo addrCriteria;                   // Criteria for address match
  	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  	// For the following fields, a zero value means "don't care"
  	addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  	addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  	// get address(es)
  	int rtnval = getaddrinfo(dstAddress, dstPortNumber, &addrCriteria, &servAddr);
  	if (rtnval != 0){
    	DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnval));
  	}

  // create a datagram/udp socket
  	sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      	servAddr->ai_protocol); // socket descriptor for client
  	if (sock < 0){
    	DieWithSystemMessage("socket() failed");
  	}

  	struct sigaction sig;  //Set two signals, say, SIGALRM and SIGINT
  	memset(&sig, '0', sizeof(sig));
  	sig.sa_handler = signal_handler; //Registe for handler function
  	if(sigfillset(&sig.sa_mask)<0){
		DieWithSystemMessage("sigfillset() failed");
  	}
  	sig.sa_flags = SA_INTERRUPT;  //The routine that excuting will be terminated
  	if(sigaction(SIGALRM, &sig, NULL)<0){
		DieWithSystemMessage("sigaction(SIGALRM) failed");
  	}  
  	if(sigaction(SIGINT, &sig, NULL)<0){
		DieWithSystemMessage("sigaction(SIGAINT) failed");
  	}

  	long sequenceNum;
  	ssize_t numBytes;
  	size_t restTime = testDuration; //The rest time that the program should be excuted
  	alarm(restTime);
  	burstSequence = -1; //The burst sequence, it indicates the serial number of the current burst 
  	totalTime=getTime(); //Record the start of the total time
  	while(true){ //Begin to send and recieve packets
		++burstSequence;  
		double burstTime = getTime(); //The start time of each burst
		pos = 0; //Indicate each packet in one burst
		while(pos < burstSize){ //Send burstSize packets one time back to back
  			rttTime[pos] = getTime();  //The start time of one specific rtt
			++pos;
  			++pacNum; //Sequence number
  			sequenceNum = (unsigned long)htonl(pacNum); //Send the sequence number at the network byte order
  			memcpy(echoString, &sequenceNum, sizeof(long));
  			numBytes = sendto(sock, echoString, dataSize, 0,
      		servAddr->ai_addr, servAddr->ai_addrlen);
  			if (numBytes < 0){
    			DieWithSystemMessage("sendto() failed");
  			}else if (numBytes != dataSize){
    			DieWithUserMessage("sendto() error", "sent unexpected number of bytes");
  			}
		}

  		struct sockaddr_storage fromAddr; // Source address of server
  		// Set length of from address structure (in-out parameter)
  		socklen_t fromAddrLen = sizeof(fromAddr);
  
  		pos = 0; //Reset pos to zero
		while(pos < burstSize){  //Recieved a packet that you want, say, the correct sequence number
	  		restTime = alarm(TIMEOUT_SECS); //Set the time out time for a packet
	  		bool timeFlag = false; //Just for deliberate control
	  		if(restTime < TIMEOUT_SECS){ //If the rest time of excution is less than TIMEOUT_SECS
				alarm(restTime);
				timeFlag = true;
	  		}
			char buffer[MAXSTRINGLENGTH + 1]; // I/O buffer
			char timeStamp[TIME_LEN];
			if ((numBytes = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0,
				(struct sockaddr *) &fromAddr, &fromAddrLen)) < 0) {
				if (errno == EINTR) {     // Alarm went off
					if(!timeFlag){
						restTime = restTime - TIMEOUT_SECS;
						alarm(restTime);
					}
					getCurTime(timeStamp); //The time that client recieves the current packet
					printf("%s %3.6fs\n", timeStamp, 0.0);
	  				burstTimeoutNum[burstSequence] = burstSize - pos;
					timeoutNum += burstSize - pos; //All the timeout packets
					break; //All the packets that can be recieved in this burst have been recieved
				}else{
					DieWithSystemMessage("recvfrom() failed");
				}
			}else if (numBytes != dataSize){
		  		DieWithUserMessage("recvfrom() error", "received unexpected number of bytes");
	  		}else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr *) &fromAddr)){
		  		DieWithUserMessage("recvfrom()", "received a packet from unknown source");
	  		}
	  		memcpy(&sequenceNum, buffer, sizeof(long));  //Recieve the sequence number and convert it to host order
	  		sequenceNum=(unsigned long)ntohl(sequenceNum); //Convert it to host order from network byte order
	  		if(sequenceNum <= pacNum && sequenceNum > pacNum - burstSize){ //If client recieved a packet that has a right sequence number
				getCurTime(timeStamp); //The time that client recieves the current packet
				buffer[dataSize] = '\0';     // Null-terminate received data

				if(!timeFlag){ //Maybe it just need to write "alarm(restTime)"
					size_t tempTime = alarm(0); //Reset the alarm 
					alarm(restTime - (TIMEOUT_SECS - tempTime));
				}

				rttTime[(sequenceNum - 1) % burstSize] = getTime() - rttTime[(sequenceNum - 1) % burstSize]; //Rtt time, in seconds
				if(pacNum-timeoutNum > curLen){//No available memorry to use, so reallocate
					curLen+=DEFAULT_ARRAY_LEN;
					allRtt=(double *)realloc(allRtt, curLen*sizeof(double));
					if(allRtt == NULL){
						DieWithSystemMessage("Allocate memorry failed!");
					}	
				}
				allRtt[sequenceNum - timeoutNum - 1] = rttTime[(sequenceNum - 1) % burstSize]; //Recorde each rtt
				if(rttTime[(sequenceNum - 1) % burstSize] < minRttTime){
					minRttTime = rttTime[(sequenceNum - 1) % burstSize];
				}
				if(rttTime[(sequenceNum - 1) % burstSize] > maxRttTime){
					maxRttTime = rttTime[(sequenceNum - 1) % burstSize];
				}
				avgRttTime += rttTime[(sequenceNum - 1) % burstSize];  //Compute the total rtt time for computing the average rtt time
				printf("%s %.6fs %ld\n", timeStamp, rttTime[(sequenceNum - 1) % burstSize], sequenceNum); // Print the information of each iteration
	  			++pos;
			}else{
				printf("Incorrect sequence number! sequenceNum = %ld\n", sequenceNum);
	  		}
		} //End of inter while(pos < burstSize)
    	totalTimes[burstSequence] = getTime() - burstTime;
    	usleep(timeInterval);  //Wait for the next sending
	} //End of outer while(true) //Begin to recieve the packets
	return 0;
} //End of main

void signal_handler(int sig){
	  switch(sig){
		case SIGINT:
		   burstTimeoutNum[burstSequence] = burstSize - pos; //Pocess the special case 
  		   printStatistics();
		   exit(0);
		   break;
		case SIGALRM:
		   if((int)(getTime() - totalTime) >= testDuration){
	  		   burstTimeoutNum[burstSequence] = burstSize - pos;//Pocess the special case 
  		   	   printStatistics();
		       exit(0);
		   }
		   break;
		default:
		  break; 
	  }
}	 

double getTime(){
	struct timeval curTime;
	(void) gettimeofday (&curTime, (struct timezone *) NULL);
	return (((((double) curTime.tv_sec) * 1000000.0) 
			 + (double) curTime.tv_usec) / 1000000.0); 
}	 

void getCurTime(char *tmpLine){
	struct timeval curTime;
	(void) gettimeofday(&curTime, (struct timezone *)0);
	sprintf(tmpLine, "%s ",  (char *)ctime((const time_t *)&(curTime.tv_sec)));

	//remove the CR that ctime adds
	tmpLine[strlen(tmpLine) - 2] = 0x0;
	tmpLine[strlen(tmpLine) - 1] = 0x0;
}	 

double calMeanDev(const double *allRtt, const unsigned int length, const double avgRttTime){
  	if(length == 0){ //If no package recieved from server
     	return 0.0;
  	}
  	double mdev=0.0;
  	unsigned int i = 0;
  	for(i = 0; i < length; ++i){
		mdev += fabs(allRtt[i] - avgRttTime);
	}
  	return mdev/(double)length;
}

void printEachBurst(){
	size_t pos = 0;
	double loss = 0.0;
	while(pos < burstSequence + 1){
  		double tempLoss = 100*(double)burstTimeoutNum[pos]/burstSize;
		loss += tempLoss;
		//printf("Burst %d: %d packets transmitted, %d recieved, %.3f%% lost, time %.6f\n", pos + 1, burstSize, burstSize - burstTimeoutNum[pos], tempLoss, totalTimes[pos]);
		++pos;
	}
	printf("The average loss rate of all %d bursts is: %.3f%%\n", burstSequence + 1, loss/(burstSequence + 1));
}

void printStatistics(){
  	totalTime=getTime() - totalTime;
  	unsigned int i;
	timeoutNum = 0;
  	for(i = 0; i <= burstSequence; ++i){
	  	timeoutNum += burstTimeoutNum[i];
  	}
  	rttMeanDev = calMeanDev(allRtt, pacNum - timeoutNum, avgRttTime/(double)(pacNum - timeoutNum));
  	freeaddrinfo(servAddr); //Free allocated memorry
  	close(sock); //Close socket
  	printf("\n-----%s:%s statistics-----\n", dstAddress, dstPortNumber);
  	double loss=100*(double)timeoutNum/pacNum;
  	printf("%d packets transmitted, %d received, %.3f%% loss, time %.6fs\n", pacNum, pacNum - timeoutNum, loss, totalTime);
  	printEachBurst();
  	avgRttTime = avgRttTime/(double)(pacNum-timeoutNum);
  	if(minRttTime == DBL_MAX){ //If no packets recieved from server, the variable min will not be changed
	minRttTime = 0.0;  //So we should let it be 0.0
	avgRttTime=0.0; //Of course the average rtt is 0.0
  	}
  	printf("rtt min/avg/max/mdev = %.6f/%.6f/%.6f/%.6f s\n", minRttTime, avgRttTime, maxRttTime, rttMeanDev);
  	free(allRtt);
  	free(burstTimeoutNum);
  	free(totalTimes);
}
