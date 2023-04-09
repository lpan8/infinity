/**
 * Examples - Read/Write/Send Operations
 *
 * (c) 2018 Claude Barthels, ETH Zurich
 * Contact: claudeb@inf.ethz.ch
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cassert>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/requests/RequestToken.h>

#define PORT_NUMBER 8013
#define SERVER_IP "10.0.2.1"

// Usage: ./progam -s {numClients} for server and ./program {clientId} for client component
int main(int argc, char **argv) {

	bool isServer = false;
    int clientId = 0;
    int numClients = 0;

	if (argc > 1) {
		if (argv[1][0] == '-') {
            char arg = argv[1][1];
			if (arg == 's') {
                isServer = true;
                numClients = argv[1][3] - '0';
                printf("num clients: %d\n", numClients);
            }
           
		} else {
            clientId = argv[1][0] - '0';
            printf("client id: %d\n", clientId);

        }
	}

	infinity::core::Context *context = new infinity::core::Context();
	infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);
	infinity::queues::QueuePair *qp;

	if(isServer) {
        infinity::queues::QueuePair* clientSockets[numClients];
        printf("Creating buffers to read from and write to\n");
        printf("Setting up connection (blocking)\n");
        qpFactory->bindToPort(PORT_NUMBER);
        infinity::memory::Buffer *bufferToReadWrite = new infinity::memory::Buffer(context, 128 * sizeof(char));
        infinity::memory::RegionToken *bufferToken = bufferToReadWrite->createRegionToken();
        
        for (int i = 0; i < numClients; i++) {
            qp = qpFactory->acceptIncomingConnection(bufferToken, sizeof(infinity::memory::RegionToken));
            clientSockets[i] = qp;

            printf("Message received\n");
   
            sleep(1);
            printf("Message: %s\n", (char*)bufferToReadWrite->getData());
            
        }
        delete bufferToReadWrite;

	} else {

		printf("Connecting to remote node\n");
		qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
		infinity::memory::RegionToken *remoteBufferToken = (infinity::memory::RegionToken *) qp->getUserData();
		
        if (clientId == 0) {
            printf("Creating buffers\n");
            char data[] = "hello world";
            infinity::memory::Buffer *buffer1Sided = new infinity::memory::Buffer(context, data, 128 * sizeof(char));
            infinity::requests::RequestToken requestToken(context);
            printf("Writing content to remote buffer\n");
            qp->write(buffer1Sided, remoteBufferToken, &requestToken);
            requestToken.waitUntilCompleted();

            printf("Reading content from remote buffer\n");
            
            qp->read(buffer1Sided, remoteBufferToken, &requestToken);
            requestToken.waitUntilCompleted();

            printf("message: %s\n", (char*)(buffer1Sided->getData()));

            delete buffer1Sided;
        } else {
            infinity::memory::Buffer *buffer1Sided = new infinity::memory::Buffer(context, 128 * sizeof(char));
            infinity::requests::RequestToken requestToken(context);
            printf("Reading content from remote buffer\n");
            qp->read(buffer1Sided, remoteBufferToken, &requestToken);
            requestToken.waitUntilCompleted();

            printf("message: %s\n", (char*)(buffer1Sided->getData()));
        }
		

	}

	delete qp;
	delete qpFactory;
	delete context;

	return 0;

}
