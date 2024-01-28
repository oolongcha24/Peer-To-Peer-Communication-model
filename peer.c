#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_UDP_PORT 8886
#define SERVER_TCP_PORT 8887
#define FIXED_IP_ADDRESS "10.1.1.34" //THIS IS YOUR OWN IP
#define INDEX_IP_ADDRESS "10.1.1.37" //THIS IS INDEX SERVER IP

typedef struct pdu {
	char type;  // 'R' for content registration, 'D' for data, 'F' for final, 'E' for error
	char data[100];
} pdu;


void listAllFiles(int udpSocket, struct sockaddr_in udpServerAddr, char *name) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	char tempArray[10][10][80];
	int i, j, A, B, x, y;
	A = B = 0;

	//initialize tempArray with empty elements to avoid random special characters
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			strcpy(tempArray[i][j], "\0");
		}    
	} 

	//setup UDP transmit type and data
	pdu sendOutPDU, recievedPDU;
	sendOutPDU.type = 'O';
	sprintf(sendOutPDU.data, "%-9.9s", name);
	sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
	printf("peer name: %s request all file list\n", name);

	//request have been send, waiting for index server to return
	while(1){
		if (recvfrom(udpSocket, &recievedPDU, sizeof(recievedPDU), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0){
			fprintf(stderr, "recvfrom error\n");
			break;
		}   

		//handle server returned data depending on the type
		if (recievedPDU.type == 'O') {
			int A = 0;
			char inputString[80];
			strcpy(inputString, recievedPDU.data);
			char *token;
			token = strtok(inputString, "/");
			while (token != NULL) {
				//printf("Token: %s\n", token);
				strcpy(tempArray[A][B], token);
				A++;
				// Get the next token
				token = strtok(NULL, "/");
			}
			B++;
		}
		else if(recievedPDU.type == 'E'){
			//printf("%s\n", recievedPDU.data);
			break;
		}
	}

	//print out array content 
	for (x = 0; x < 10; x++) {
		for (y = 0; y < 10; y++) {
			printf("%-10s", tempArray[x][y]);
		}
		printf("\n");
	}
}


void registerWithServer(int udpSocket, struct sockaddr_in udpServerAddr, char *name, char *ip, char *filename) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	//setup UDP transmit type and data
	pdu sendOutPDU, recievedPDU;
	sendOutPDU.type = 'R';
	sprintf(sendOutPDU.data, "%-9.9s,%-9.9s,%-19.19s", name, filename, ip);
	sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
	printf("\nRegistration sent to server %s for %s.\n", ip, filename);

	//request have been send, waiting for index server to return
	while(1){
		if (recvfrom(udpSocket, &recievedPDU, sizeof(recievedPDU), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0){
			fprintf(stderr, "recvfrom error\n");
			break;
		}   

		//handle server returned data depending on the type
		if (recievedPDU.type == 'A') {
			printf("\nfile have been registered in index server!\n");
			break;
		} 
		else if(recievedPDU.type == 'E'){
			printf("error catched in reg\n");
			printf("%s\n", recievedPDU.data);
			break;
		}
	}
}

void DeregisterFile(int udpSocket, struct sockaddr_in udpServerAddr, char *name, char *filename) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	//setup UDP transmit type and data
	pdu sendOutPDU, recievedPDU;
	sendOutPDU.type = 'T';
	sprintf(sendOutPDU.data, "%-9.9s,%-9.9s", name, filename);
	sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
	printf("De-registration with peer name: %s , file name: %s.\n", name, filename);

	//request have been send, waiting for index server to return
	while(1){
		if (recvfrom(udpSocket, &recievedPDU, sizeof(recievedPDU), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0){
			fprintf(stderr, "recvfrom error\n");
			break;
		}   

		//handle server returned data depending on the type
		if (recievedPDU.type == 'A') {
			printf("file have been de-registered in index server!\n");
			break;
		} 
		else if(recievedPDU.type == 'E'){
			//printf("error catched in de-regegister\n");
			printf("%s\n", recievedPDU.data);
			break;
		}
	}
}

void DeregisterPeer(int udpSocket, struct sockaddr_in udpServerAddr, char *name) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	//setup UDP transmit type and data
	pdu sendOutPDU, recievedPDU;
	sendOutPDU.type = 'T';
	sprintf(sendOutPDU.data, "%-9.9s,%-9.9s", name, "*********");
	sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
	printf("De-registration the peer name: %s\n", name);
}


int requestDownloadFileUDP(int udpSocket, struct sockaddr_in udpServerAddr, char *name, char *ip, char *filename) {
	int port_numberINT, peer_addressINT, file_download;
	char peername[10], host[10];//*host;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	file_download = 0;

	//setup UDP transmit type and data
	pdu sendOutPDU, recievedPDU;
	sendOutPDU.type = 'S';
	sprintf(sendOutPDU.data, "%-9.9s,%-9.9s", name, filename);
	sendto(udpSocket, &sendOutPDU, sizeof(sendOutPDU), 0, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
	printf("\npeer name: %s Sending Search content for download to index server, file name: %s.\n", name, filename);

	//request have been send, waiting for index server to return
	while(1){
		if (recvfrom(udpSocket, &recievedPDU, sizeof(recievedPDU), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0){
			fprintf(stderr, "recvfrom error\n");
			break;
		}   

		//handle server returned data depending on the type
		if (recievedPDU.type == 'S'){
			printf("%s\n", recievedPDU.data);
			char peer_name[10];
			char peer_address[50];
			char port_number[50];
			char inputString[80];
			strcpy(inputString, recievedPDU.data);
			char *token;
			token = strtok(inputString, "/");
			if (token != NULL) {
				strcpy(peer_name, token);
				token = strtok(NULL, "/");
				if (token != NULL) {
					strcpy(peer_address, token);
					token = strtok(NULL, "/");
					if (token != NULL) {
						strcpy(port_number, token);
						token = strtok(NULL, "/");
						//printf("Peer Name: %s\n", peer_name);
						//printf("Peer Address: %s\n", peer_address);
						//printf("Port Number: %s\n", port_number);
						port_numberINT = atoi(port_number); //convert port to int variable
						strcpy(peername, peer_name);
						strcpy(host, peer_address);
						//printf("host = %s\n", host);
						file_download = 1;
					} else {
						printf("Invalid input format\n");
					}
				} else {
					printf("Invalid input format\n");
				}

			} else {
				printf("Invalid input format\n");
			}
			break;
		}
		else if(recievedPDU.type == 'E'){
			printf("error catched in requestdownload\n");
			printf("%s\n", recievedPDU.data);
			return 0;
		}
	}//end of while loop

	//Create A TCP server request for file download
	//printf("port number in int value: %d\n", port_numberINT);

	int client_socket;
	struct sockaddr_in server_addr;

	// Create socket
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	// Initialize server address structure
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_numberINT);
	server_addr.sin_addr.s_addr = inet_addr(host);

	

	// Connect to the server
	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Connection failed");
		exit(EXIT_FAILURE);
	}

	printf("\nConnected to server on port %d\n", port_numberINT);

	pdu sendOutPDU2;
	sendOutPDU2.type = 'D';
	sprintf(sendOutPDU2.data, "%-9.9s,%-9.9s", peername, filename);


	write(client_socket, &sendOutPDU2, sizeof(sendOutPDU2));

	char filepath[10];
	strcpy(filepath, filename);
	//const char* filePath = "/home/condor/Desktop/p1.txt"; //download path
	strcat(filepath, ".txt");

	FILE* file = fopen(filepath, "w");


   if (file == NULL) {
        // Handle the error if fopen fails
		printf("can't open file");
        perror("Error opening file\n");
    }

	int first_packet=0; //checking if the downloaded data is the first packet (for error data checking)
	int n;
	while(1){  //read message from server 
		n = read(client_socket, &recievedPDU, sizeof(recievedPDU));
		//printf("n = %d\n",n);
		//printf("%s\n", recievedPDU.data);

		if (strlen(recievedPDU.data) == 0) {
			printf("done receiving, close the file");
			break;
		}

		fprintf(file, "%s", recievedPDU.data);
		fflush(file); // Add this line to flush the buffer


		//write(1, sbuf, n);
		//printf("\n");

	}

	printf("file close\n");
    fclose(file);
	close(client_socket);
	return file_download;
}

int main() {
	///////////////////////////////////////////////////////////////////
	//Createing TCP socket connections
	//////////////////////////////////////////////////////////////////
	int tcpSocket, client_sock, udpSocket;    
	struct sockaddr_in tcp_server_addr, client_addr, udpServerAddr;
	socklen_t tcp_client_len = sizeof(client_addr);

	char name[20];
	char filename[50] = "\0";
	char port_number[10] = "12345";

	// Fixed IP address
	char ip[20];
	strcpy(ip, FIXED_IP_ADDRESS);

	printf("Enter your name: ");
	scanf("%s", name);

	// Create a TCP socket
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocket == -1) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	// Set up the server address struct
	memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
	tcp_server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, FIXED_IP_ADDRESS, &tcp_server_addr.sin_addr);
	tcp_server_addr.sin_port = htons(0);

	// Bind the socket to the specified address and port
	if (bind(tcpSocket, (struct sockaddr*)&tcp_server_addr, sizeof(tcp_server_addr)) == -1) {
		perror("Binding failed");
		close(tcpSocket);
		exit(EXIT_FAILURE);
	}
	//////////////////////////////////////////////////////////
	//create TCP connection ends
	//////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////
	// Creating UDP connection 
	/////////////////////////////////////////////////////////
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) {
		perror("UDP socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&udpServerAddr, 0, sizeof(udpServerAddr));
	udpServerAddr.sin_family = AF_INET;
	udpServerAddr.sin_port = htons(SERVER_UDP_PORT);
	inet_pton(AF_INET, INDEX_IP_ADDRESS, &udpServerAddr.sin_addr);
	////////////////////////////////////////////////////////
	// create UDP connection ends              
	////////////////////////////////////////////////////////

	// Listen for incoming connections
	if (listen(tcpSocket, 5) == -1) {
		perror("Listening failed");
		close(tcpSocket);
		exit(EXIT_FAILURE);
	}

	//printf("Server is listening on port 8080...\n");
	printf("Enter the file name you want to share: ");
	scanf("%s", filename);

	if (getsockname(tcpSocket, (struct sockaddr*)&tcp_server_addr, &tcp_client_len) == -1) {
		perror("Error getting socket name");
		close(tcpSocket);
		exit(EXIT_FAILURE);
	}

	//get port number and save it with ip address
	snprintf(port_number, sizeof(port_number), "%d", ntohs(tcp_server_addr.sin_port));
	//printf("Server listening on port %d\n", ntohs(tcp_server_addr.sin_port));

	strcat(ip, "/");
	strcat(ip, port_number);
	registerWithServer(udpSocket, udpServerAddr, name, ip, filename);

	printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");

	fd_set rfds, afds;
	FD_ZERO(&afds);
	FD_SET(tcpSocket, &afds);
	FD_SET(0, &afds);

	while (1) {
		fd_set tmp_fds = afds;  // Make a temporary copy for select

		// Wait for activity on any of the file descriptors
		if (select(FD_SETSIZE, &tmp_fds, NULL, NULL, NULL) == -1) {
			perror("Select failed");
			break;
		}

		// Check for activity on the socket
		if (FD_ISSET(tcpSocket, &tmp_fds)) {
			int client_socket;
			// Accept the incoming connection
			client_socket = accept(tcpSocket, (struct sockaddr*)&client_addr, &tcp_client_len);
			if (client_socket == -1) {
				perror("Accept failed");
				continue;
			}
			printf("\n\nAccepted a TCP connection\n\n\n");
			struct pdu received_pdu;
			
			int n = read(client_socket, &received_pdu, sizeof(received_pdu));
			
			char peer_name[10], filename[10];
			sscanf(received_pdu.data, "%9[^,],%9[^,]", peer_name, filename);
			//printf("received download request, peer name: %-10s, filename: %-10s\n", peer_name, filename);

			//download content
			pdu responsePDU;
			char filepath[100];
	
			snprintf(filepath, sizeof(filepath), "%s.txt", filename);
			   size_t length = strlen(filepath);

			// Iterate through each character in the string
			size_t i,j;
			for (i = 0; i < length; i++) {
				if (filepath[i] == ' ') {
				    // Shift the remaining characters to the left
				    for (j = i; j < length; j++) {
				        filepath[j] = filepath[j + 1];
				    }
				    // Update the length of the string
				    length--;
				    // Decrement the index to recheck the current position
				    i--;
				}
			}



			//printf("filepath = %s\n", filepath);
			FILE *file = fopen(filepath, "r");
			if (file == NULL) {
				// File opening failed
				perror("Error opening file");
				return 1; // You may choose a different return value based on your needs
			}
			//printf("file path: %s\n", filepath);
			if (file) {
				while (1) {
					strcpy(responsePDU.data, "");
					responsePDU.type = 'C';  // 'C' for content data

					if (fgets(responsePDU.data, sizeof(responsePDU.data), file) != NULL) {
						// Send data to the client
						//printf("%s\n", responsePDU.data);
						write(client_socket, &responsePDU, sizeof(responsePDU));
					} else{ //end of file
						responsePDU.type = 'F';
						write(client_socket, &responsePDU, sizeof(responsePDU));
						fclose(file);
						printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");
						break;
					}


				}
			}
		}

		// Check for activity on user input
		if (FD_ISSET(0, &tmp_fds)) {
			// Read input from the console
			//printf("entered user input mode\n");
			char buffer[256];

			//fgets(buffer, sizeof(buffer), stdin);
			scanf("%s", buffer);

			//register new file
			if (!(strcmp(buffer, "1"))){
				printf("\nRegistering files to index server\n");
				printf("Enter filename to register: ");
				scanf("%s", filename);
				registerWithServer(udpSocket, udpServerAddr, name, ip, filename);
				printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");
			}
			//download file from other peer
			else if (!(strcmp(buffer, "2"))){
				printf("enter the file name you want to download:\n");
				char filetoDownload[10];
				int result;
				scanf("%s", filetoDownload);
				result = requestDownloadFileUDP(udpSocket, udpServerAddr, name, ip, filetoDownload);
				//printf("returned result: %d", result);	
				if (result == 1){
					registerWithServer(udpSocket, udpServerAddr, name, ip, filetoDownload);
				}
				printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");
			}
			//deregister a file from the index server
			else if (!(strcmp(buffer, "3"))){
				printf("Enter deregister file from index server\n");	
				char filetoDereg[10];
				scanf("%s", filetoDereg);
				DeregisterFile(udpSocket, udpServerAddr, name, filetoDereg);
				printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");
			}
			//list all registered files and it's corresponding peer
			else if (!(strcmp(buffer, "4"))){
				printf("List registered files\n");		
				listAllFiles(udpSocket, udpServerAddr, name);
				printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");
			}
			//exit peer, deregister peer in index server
			else if (!(strcmp(buffer, "5"))){
				printf("exit\n");
				DeregisterPeer(udpSocket, udpServerAddr, name);		
				break;
			}
			//invalid input by user
			else{
				printf("invalid input\n");
				printf("\n1. Reg another file\n2.Download a file\n3.Deregister\n4.List all files\n5.Exit\n");
			}

		}
	}

	// Cleanup
	close(tcpSocket);
	close(udpSocket);
	return 0;
}

