//Using SDL and standard IO
#include <stdio.h>
#include <SDL.h>
#include <iostream>
#include <enet/enet.h>
#include <Windows.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

ENetHost * client;
ENetPeer *server_peer;
ENetPeer *connected_peer;
ENetPacket  *packet;
char currentKeys[256];
int keyCount = 0;

//Key press surfaces constants
enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// Surfaces containing the key images
SDL_Surface* blank = NULL;
SDL_Surface* w = NULL;
SDL_Surface* a = NULL;
SDL_Surface* s = NULL;
SDL_Surface* d = NULL;
SDL_Surface* wa = NULL;
SDL_Surface* ws = NULL;
SDL_Surface* wd = NULL;
SDL_Surface* as = NULL;
SDL_Surface* ad = NULL;
SDL_Surface* sd = NULL;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
	//Create window
	gWindow = SDL_CreateWindow("SakuStream", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Get window surface
		gScreenSurface = SDL_GetWindowSurface(gWindow);
	}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load key images
	blank = SDL_LoadBMP("keys/blank.bmp");
	w = SDL_LoadBMP("keys/w.bmp");
	a = SDL_LoadBMP("keys/a.bmp");
	s = SDL_LoadBMP("keys/s.bmp");
	d = SDL_LoadBMP("keys/d.bmp");
	wa = SDL_LoadBMP("keys/wa.bmp");
	ws = SDL_LoadBMP("keys/ws.bmp");
	wd = SDL_LoadBMP("keys/wd.bmp");
	as = SDL_LoadBMP("keys/as.bmp");
	ad = SDL_LoadBMP("keys/ad.bmp");
	sd = SDL_LoadBMP("keys/sd.bmp");

	//Apply the image
	SDL_BlitSurface(blank, NULL, gScreenSurface, NULL);
	//Update the surface
	SDL_UpdateWindowSurface(gWindow);

	if (blank == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", "blank.bmp", SDL_GetError());
		success = false;
	}

	return success;
}

void close()
{
	//Deallocate surface
	SDL_FreeSurface(blank);
	blank = NULL;
	w = NULL;
	a = NULL;
	s = NULL;
	d = NULL;
	wa = NULL;
	ws = NULL;
	wd = NULL;
	as = NULL;
	ad = NULL;
	sd = NULL;


	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void connectToServer() {

	char input[10];
	printf("Enter IP address of server:");
	std::cin >> input;
	// Handle input errors and defaults here.
	std::cout << "Conencting to server " << input << std::endl;

	ENetAddress address;
	ENetEvent event;
	ENetPeer *peer;
	/* Connect to some.server.net:1234. */
	enet_address_set_host(&address, input);
	address.port = 1234;
	/* Initiate the connection, allocating the two channels 0 and 1. */
	peer = enet_host_connect(client, &address, 2, 0);
	if (peer == NULL)
	{
		printf("Could not connect.");
		exit(EXIT_FAILURE);
	}
	/* Wait up to 5 seconds for the connection attempt to succeed. */
	if (enet_host_service(client, &event, 50) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
	{
		puts("Connection to 127.0.0.1:1234 succeeded.");

		// Set the connected peer
		connected_peer = peer;

		printf("A new client connected from %x:%u.\n",
			event.peer->address.host,
			event.peer->address.port);
		//enet_host_flush(client);
	}
	else
	{
		printf("Spooky 5 seconds occurred.");
		/* Either the 5 seconds are up or a disconnect event was */
		/* received. Reset the peer in the event the 5 seconds   */
		/* had run out without any significant event.            */
		enet_peer_reset(peer);
		puts("Connection to 127.0.0.1:1234 failed.");
	}
}

// Starts up the client to connect to the remote machine.
// In this case it is 192.168.1.50
void startClient() {

	printf("Starting client.");
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */);
	if (client == NULL)
	{
		fprintf(stderr,
			"An error occurred while trying to create an ENet client host.\n");
		exit(EXIT_FAILURE);
	}
	printf("Client started.");
	connectToServer();
}

void shutdownAll() {
	enet_host_destroy(client);
	atexit(enet_deinitialize);
	connected_peer = NULL;
	printf("Server and client shut down.");
}

void sendPacket() {
	if (currentKeys[0] == NULL) {

	}
	else {
		printf("Sent " + currentKeys[0]+ currentKeys[1]);
		ENetPacket *packet = enet_packet_create(currentKeys, strlen("packet") + 1, ENET_PACKET_FLAG_RELIABLE);

		/* Extend the packet so and append the string "foo", so it now */
		/* contains "packetfoo\0"
		*/
		enet_packet_resize(packet, keyCount + 1);

		/* Send the packet to the peer over channel id 0. */
		/* One could also broadcast the packet by         */
		/* enet_host_broadcast (host, 0, packet);         */
		enet_peer_send(connected_peer, 0, packet);
		enet_host_flush(client);

		// Free up the keybuffer
		for (int i = 0; i < keyCount; i++) {
			currentKeys[i] = NULL;
		}
		keyCount = 0;
	}
}

void applyupdate(SDL_Surface *letter) {
	//Apply the image
	SDL_BlitSurface(letter, NULL, gScreenSurface, NULL);
	//Update the surface
	SDL_UpdateWindowSurface(gWindow);
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		// Maybe here too
		//printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			// Weird stuff here as well
			//printf("Failed to load media!\n");
		}
		else
		{
			// Initialize ENet
			if (enet_initialize() != 0)
			{
				// Something wonky is going on here
				//fprintf(stderr, "An error occurred while initializing ENet.\n");
				return EXIT_FAILURE;
			}
			//atexit(enet_deinitialize);W
			
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{

					//If a key was pressed
					if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
					{
						//Adjust the velocity
						switch (e.key.keysym.sym)
						{
						case SDLK_F1: printf("Connecting.."); startClient(); break;
						case SDLK_w: printf("W"); currentKeys[keyCount] = 'W'; keyCount++; applyupdate(w); break;
						case SDLK_a: printf("A"); currentKeys[keyCount] = 'A'; keyCount++; applyupdate(a); break;
						case SDLK_s: printf("S"); currentKeys[keyCount] = 'S'; keyCount++; applyupdate(s); break;
						case SDLK_d: printf("D"); currentKeys[keyCount] = 'D'; keyCount++; applyupdate(d); break;
						case SDLK_q: shutdownAll(); quit = true; break;
						case SDLK_p: sendPacket(); break;
						}
					}

					//If a key was released
					if (e.type == SDL_KEYUP && e.key.repeat == 0)
					{
						//Adjust the velocity
						switch (e.key.keysym.sym)
						{
						case SDLK_w: printf("w"); currentKeys[keyCount] = 'w'; keyCount++; applyupdate(blank); break;
						case SDLK_a: printf("a"); currentKeys[keyCount] = 'a'; keyCount++; applyupdate(blank); break;
						case SDLK_s: printf("s"); currentKeys[keyCount] = 's'; keyCount++; applyupdate(blank); break;
						case SDLK_d: printf("d"); currentKeys[keyCount] = 'd'; keyCount++; applyupdate(blank); break;
						}
					}

					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
				}

				// Send packet update to server in 100 milliseconds.
				Sleep(50);
				if (connected_peer != NULL) {
					sendPacket();
				}
			}
		}
	}

	//Free resources and close SDL
	system("pause");
	close();
	return 0;
}