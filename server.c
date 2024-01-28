#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define SERVER_UDP_PORT 8886
#define SERVER_TCP_PORT 8887
#define SERVER_IP "10.1.1.37"


typedef struct pdu {
	char type;  // 'R' for content registration, 'D' for content download request, 'S' for search for content and the associated content server, 'T' for content de-registration, 'C' for content data, 'O' for list of online registered content, 'A' for acknowledgement, 'E' for error
	char data[100];
} pdu;

//global variables
char stringArray[10][10][80]; 
int download_counter[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// search a file that retruns the minimum download counter
int searchFile(char filename[10]){ // search a file that retruns the minimum download counter
    printf("entered SearchFile()\n");
    int coordinate[10]; //coordinate for peer name 
    //ROWS = 5;
    //COLS = 5;
    bool found = false;
    char matches[10][10]; // max: 10 matches, filename is 10-bit long
    int matchCount = 0;
    int i,j;
    for (i = 2; i < 10; i++) {
	for (j = 0; j < 10; j++) {
	    if (strcmp(stringArray[i][j], filename) == 0) {
		found = true;
		printf("i: %d j: %d\n",i,j);
		coordinate[matchCount]=j;
		snprintf(matches[matchCount], 10, "(%d, %d)", i, j);
		++matchCount;
	    }
	}
    }
    //[(2,0) , (2,2)]
    // Print the result
    if (found) {
	printf("String %s found at indices: ",filename);
	int i;
	for (i = 0; i < matchCount; ++i) {
	    printf("%s", matches[i]);
	    if (i < matchCount - 1) {
		printf(", ");
	    }
	}

	printf("\n");
    } else {
	printf("String %s does not exist in the array.\n",filename);
	return -1;
    }

    //printf("matchcount: %d\n", matchCount);
    int min=coordinate[0];
    char peername[10];
    for(i=0;i<matchCount;i++){
	//printf("peer-name coordinate: %d\n", coordinate[i]);
	printf("counter value: %d with coordinate %d\n",download_counter[i], coordinate[i]);
	printf("download_counter[matchCount]: %d min: %d\n",download_counter[matchCount],min);
	if (download_counter[i] <= min) {
	    min = coordinate[i]; // Update the minimum value
	    printf("%d\n", min);
	}
    }
    printf("min peer coordinate value: %d\n", min);
    return(min);



}

// dereg a file, return 1 if success and 0 if fail
int dereg(char peer_name[10], char filename[10]){ 
	printf("entered dereg()\n");
	int coordinate[10]; //coordinate for peer name 
	int i,j,col, peer_col_index,peer_row_index;
	bool found = false;
	char matches[10][10]; // max: 10 matches, filename is 10-bit long
	int matchCount = 0;
	peer_col_index = -1;
	
	printf("debug msg\n");
	//search if peer name exist
	for (i = 0;i < 10; i++){
		if(!(strcmp(stringArray[0][i], peer_name))){
			peer_col_index = i;
			break;
		}		
	}

	//if peer does not exist, return false
	if (peer_col_index == -1){
		return 0;
	}

	//peer found, look for the file index
	for (i = 2; i < 10; i++) {
		if (strcmp(stringArray[i][peer_col_index], filename) == 0) {
			found = true;
			printf("i: %d j: %d\n",i,peer_col_index);
			peer_row_index = i;
			coordinate[matchCount]=peer_col_index;
			snprintf(matches[matchCount], 10, "(%d, %d)", i, peer_col_index);
			++matchCount;

		}
	}

	//if file is found, clear the file index content and return true
	//else return false
	if (found) {
		strcpy(stringArray[peer_row_index][peer_col_index],"");
		printf("file successfully dereged\n");
		return 1;
	} else {
		printf("dereg file does not exist\n"); 
		return 0;
	}
}

// dereg a peer when peer closes connections
int dereg_peer(char peer_name[10]){  
	printf("entered dereg_peer()\n");
	int i,j,col, peer_col_index,peer_row_index;
	peer_col_index = -1;

	//search if peer name exist
	for (i = 0;i < 10; i++){
		if(!(strcmp(stringArray[0][i], peer_name))){
			peer_col_index = i;
			break;
		}		
	}

	//if peer name is found, clear the whole column that stores info for the peer exiting
	if (peer_col_index != -1){
		for (i = 0; i < 10; i++) {
			strcpy(stringArray[i][peer_col_index],"");
		}
	}

}

void handleReceivedContent(int udpSocket) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	pdu recievedPDU, sendOutPDU;
	ssize_t recvBytes = recvfrom(udpSocket, &recievedPDU, sizeof(recievedPDU), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

	if (recvBytes == -1) {
		perror("Error receiving content");
		exit(EXIT_FAILURE);
	}

	//using if statements to select what to do depends on the data type
	if (recievedPDU.type == 'R') {
		// Parse registration information
		char peer_name[10], content_name[10], address[20];
		sscanf(recievedPDU.data, "%9[^,], %9[^,], %19s", peer_name, content_name, address);

		printf("Received registration from %s for content %s at address %s, port_number %s\n", peer_name, content_name, address);
		int i,col, peer_col_index;
		peer_col_index = -1;
		//search if peer name already exist
		for (i = 0;i < 10; i++){
			if(!(strcmp(stringArray[0][i], peer_name))){
				peer_col_index = i;
				break;
			}		
		}

		//add peer name into the array if it does not exit in the array
		if (peer_col_index == -1){
			for (i = 0;i < 10; i++){
				if(stringArray[0][i][0] == '\0'){
					strcpy(stringArray[0][i], peer_name);
					printf("Address = %s\n", address);
					strcpy(stringArray[1][i], address);
					printf("stringArray[1][i] = %s\n", stringArray[1][i]);
					peer_col_index = i;
					break;
				}		
			}
		}

		//iterate through row for finding filename repeatation
		for(i = 2; i < 10;i++){  
			if (!(strcmp(stringArray[i][peer_col_index], content_name))){
				sendOutPDU.type = 'E';
				sprintf(sendOutPDU.data, "File already exist for the peer, did not register file again!");
				sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
				break;
			}
			else if(stringArray[i][peer_col_index][0] == '\0'){
				//printf("%d \n",i);
				strcpy(stringArray[i][peer_col_index], content_name);
				sendOutPDU.type = 'A';
				sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
				break;
			}
		}


		//print out array content to check if everything is working fine
		int x,y;
		for (x = 0; x < 10; x++) {
			for (y = 0; y < 10; y++) {
				printf("%-20s", stringArray[x][y]);
			}
			printf("\n");
		}

	}
	//download file search
	else if (recievedPDU.type == 'S'){ 
		char peer_name[10], filename[10];
		sscanf(recievedPDU.data, "%9[^,],%9[^,]", peer_name, filename);
		printf("Received registration from %s for content %s at address %s\n", peer_name, filename);
		/////DEBUG download counter/////	
		/*
		   int i;	
		   for (i = 0; i < 10; i++)printf("peer%d: %d",i,download_counter[i]);
		   */
		///////////end///////////
		int result = searchFile(filename);
		if (result == -1){
			printf("file not found\n");
			sendOutPDU.type = 'E';
			sprintf(sendOutPDU.data, "File not found!");
			sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
		} else{
			printf("min peer is %s\naddress: %s \n",stringArray[0][result], stringArray[1][result]);
			sendOutPDU.type = 'S';
			//address
			char sendbackdata[80];
			strcpy(sendbackdata,"");
			strcat(sendbackdata,stringArray[0][result]);
			strcat(sendbackdata,"/");
			strcat(sendbackdata,stringArray[1][result]);
			strcpy(sendOutPDU.data,sendbackdata);
			download_counter[result] += 1;
			//sprintf(sendOutPDU.data, "%-9.9s,%-19.19s", stringArray[0][result], stringArray[1][result]);
			//	printf("%s\n", recievedPDU.data);
			sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

		}
	}
	//Deregister files or whole peer
	else if (recievedPDU.type == 'T'){ 
		char peer_name[10], content_name[10];
		sscanf(recievedPDU.data, "%9[^,],%9[^,]", peer_name, content_name);
		printf("%s\n", content_name);
		//if content name passed over are special characters, it deletes the whole peer
		//else it will delete the specific file for the peer only
		if (!(strcmp(content_name, "*********"))){
			dereg_peer(peer_name);
		}
		else{
			printf("Received deregistration from %s for content %s at address %s\n", peer_name, content_name);
			int result;
			result = dereg(peer_name,content_name);
			if (result == 0){
				printf("file not found\n");
				sendOutPDU.type = 'E';
				sprintf(sendOutPDU.data, "File not found!");
				sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
			} else{
				//printf("min peer is %s\naddress: %s \n",stringArray[0][result], stringArray[1][result]);
				sendOutPDU.type = 'A';
				sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

			}
		}
	}
	//List all files in index server
	else if (recievedPDU.type == 'O') { 
		char sendbackData[80];
		int i;    
		for (i = 0; i < 10; i++){
			strcpy(sendbackData, "");
				int j;
				strcpy(sendbackData, stringArray[0][i]);
				for (j = 2; j < 10; j++){
					strcat(sendbackData, "/");
					strcat(sendbackData, stringArray[j][i]);
				}
				sendOutPDU.type = 'O';
				sprintf(sendOutPDU.data, sendbackData);
				sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

		}
		sendOutPDU.type = 'E';
		sprintf(sendOutPDU.data, "End of List all File");
		sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
	}
	else{
		printf("type not known");
	}
}

int main() {
	int i,j;
	//initialize the string array to empty
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			strcpy(stringArray[i][0], "");
		}	
	}	

	//create UDP connection
	int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) {
		perror("UDP socket creation failed");
		exit(EXIT_FAILURE);
	}

	int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket == -1) {
		perror("TCP socket creation failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_UDP_PORT);
	//serverAddr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

	if (bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	//create UDP connection code ends

	printf("UDP server is listening on port %d...\n", SERVER_UDP_PORT);

	while (1) {
		handleReceivedContent(udpSocket);
	}

	// Close sockets
	close(udpSocket);
	close(tcpSocket);

	return 0;
}

