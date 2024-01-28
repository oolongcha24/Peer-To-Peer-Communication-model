# Peer-To-Peer-Communication-model
A peer-to-peer communication model that allows peers to upload and download files from another peer

This Peer-to-Peer (P2P) application enables users to share and download files. It consists of an index server and multiple peers (users). Each peer can act as both a content server and a client. The application involves registering content with the index server, which then facilitates the process of other peers finding and downloading this content. It uses UDP and TCP protocols for communication and content transfer, ensuring efficient and reliable data exchange.


Server.c behaved as an index server of the P2P system. It consists of UDP sockets for communication and exchanging packets with peer.c to perform specific functions. Both programs use protocol data unit (PDU) structure to represent the communication packets with various types. Including content registration(R), content download request(D), content search (S), content deregistration(T), content data(C), list of online registered content (O), acknowledgement (A), and error (E). Server.c provides an index server that keeps track of peer’s name, downloadable content, address, and port number into a 2D array. When a peer exists, it will automatically register the contents into the index server’s 2D array, which could be extracted by another peer to establish a TCP connection to download desired content. In addition, when a peer wants to exit, the program would also update the 2D array by removing its peer name and corresponding address and downloadable content. 

Peer.c behaves as an individual node that interacts with the index server and other nodes for exchanging resources. This program consists of following functions:
    1. registerFile: When a peer entered the network, it will call this
    function to register its peer name, address, port and downloadable
    content to index server via UDP sockets, all these content will be
    stored in index server’s 2D array
    2. Dereg_File: Allows peer to deregister a specific downloadable
    content by sending a remove request UDP sockets to index server
    3. Dereg_Peer: When a peer wants to exit, it will call this function,
    which sends a remove request UDP sockets to index server to remove
    its peername, address, port and downloadable content in the 2D array
    4. DownloadFile: Request downloadable content to index server via UDP,
    index server will return the corresponding peername and its address
    and port that have the lowest download counts, then establish a TCP
    connection with that peer and request for file download. Upon
    download complete, call registerFile() function for register the
    newest downloaded file into index server’s 2D array
    5. get_All_Files: Send UDP to index server, and request all the
    existent peer name and downloadable content back to this peer.
