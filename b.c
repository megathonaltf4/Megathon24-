#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <SDL2/SDL_stdinc.h>
const double MAX_ANGLE = M_PI/2 ;
const double SPEED = 2; // Oscillation speed

double pendulumTime = 0; // Variable for pendulum oscillation timing

// Define item structure
typedef struct {
 int x, y; // Position
 int radius; // Collision radius
 bool active; // Whether item is still collectible
 int type; // Type of item (can be used to differentiate between items)
} Item;

// Define inventory structure
typedef struct {
 int items[10]; // Array to store collected item types
 int count; // Number of items collected
} Inventory;

#define MAX_ITEMS 5 // Maximum number of items in the game world

// Add new structures for player roles
typedef struct {
 int x, y;
 int radius;
 bool isController; // true for player controlling movement
 SDL_Window *window;
 SDL_Renderer *renderer;
} Player;

// Add visibility radius for controller
const int VISIBILITY_RADIUS = 150;


// Global variables for items and inventory
Item items[MAX_ITEMS];
Inventory playerInventory = {0};

// Initialize items
void initItems() {
 // Initialize the three keys at specified positions
 items[0] = (Item){30, -880, 10, true, 1}; // Key 1
 items[1] = (Item){1300, 20, 10, true, 1}; // Key 2
 items[2] = (Item){1550, -590, 10, true, 1}; // Key 3
 // Set remaining items as inactive
 for (int i = 3; i < MAX_ITEMS; i++) {
 items[i].active = false;
 }
}

// Add a function to find the nearest key within range
int findNearestKey(int playerX, int playerY, float maxDistance) {
 int nearestIndex = -1;
 float nearestDistance = maxDistance;
 
 for (int i = 0; i < MAX_ITEMS; i++) {
 if (items[i].active) {
 float dx = playerX - items[i].x;
 float dy = playerY - items[i].y;
 float distance = sqrt(dx*dx + dy*dy);
 
 if (distance < nearestDistance) {
 nearestDistance = distance;
 nearestIndex = i;
 }
 }
 }
 
 return nearestIndex;
}

// Check if the player collects an item
void checkItemCollection(int playerX, int playerY, int playerRadius) {
 for (int i = 0; i < MAX_ITEMS; i++) {
 if (items[i].active) {
 // Calculate distance between player and item
 int dx = playerX - items[i].x;
 int dy = playerY - items[i].y;
 float distance = sqrt(dx*dx + dy*dy);
 
 // If player touches item, collect it
 if (distance < (playerRadius + items[i].radius)) {
 items[i].active = false; // Deactivate item
 if (playerInventory.count < 10) { // Check if inventory has space
 playerInventory.items[playerInventory.count] = items[i].type;
 playerInventory.count++;
 }
 }
 }
 }
}

// Modify the drawItems function to draw keys with a highlight for the nearest one
void drawItems(SDL_Renderer *renderer, int cameraX, int cameraY, int nearestIndex) {
 for (int i = 0; i < MAX_ITEMS; i++) {
 if (items[i].active) {
 // Draw highlight for nearest key
 if (i == nearestIndex) {
 SDL_SetRenderDrawColor(renderer, 255, 255, 0, 128); // Yellow highlight
 for (int w = 0; w < (items[i].radius + 5) * 2; w++) {
 for (int h = 0; h < (items[i].radius + 5) * 2; h++) {
 int dx = (items[i].radius + 5) - w;
 int dy = (items[i].radius + 5) - h;
 if ((dx*dx + dy*dy) <= ((items[i].radius + 5) * (items[i].radius + 5))) {
 SDL_RenderDrawPoint(renderer,
 items[i].x - cameraX + dx - (items[i].radius + 5),
 items[i].y - cameraY + dy - (items[i].radius + 5));
 }
 }
 }
 }
 
 // Draw key in gold color
 SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gold color for keys
 for (int w = 0; w < items[i].radius * 2; w++) {
 for (int h = 0; h < items[i].radius * 2; h++) {
 int dx = items[i].radius - w;
 int dy = items[i].radius - h;
 if ((dx*dx + dy*dy) <= (items[i].radius * items[i].radius)) {
 SDL_RenderDrawPoint(renderer,
 items[i].x - cameraX + dx - items[i].radius,
 items[i].y - cameraY + dy - items[i].radius);
 }
 }
 }
 }
 }
}



// Draw inventory
void drawInventory(SDL_Renderer *renderer) {
 // Draw inventory background
 SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
 SDL_Rect inventoryBG = {10, 10, 200, 30};
 SDL_RenderFillRect(renderer, &inventoryBG);
 
 // Draw collected items
 for (int i = 0; i < playerInventory.count; i++) {
 // Set color based on item type
 switch(playerInventory.items[i]) {
 case 1:
 SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
 break;
 case 2:
 SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
 break;
 case 3:
 SDL_SetRenderDrawColor(renderer, 184, 115, 51, 255);
 break;
 case 4:
 SDL_SetRenderDrawColor(renderer, 147, 112, 219, 255);
 break;
 case 5:
 SDL_SetRenderDrawColor(renderer, 64, 224, 208, 255);
 break;
 }
 
 // Draw item icon in inventory
 SDL_Rect itemRect = {20 + (i * 30), 15, 20, 20};
 SDL_RenderFillRect(renderer, &itemRect);
 }
}

// Add a function to check if player is in inventory area
bool isPlayerInInventoryArea(int ballX, int ballY, int ballRadius, int cameraX, int cameraY) {
 SDL_Rect inventoryArea = {10, 10, 200, 30};
 SDL_Rect playerArea = {
 ballX - cameraX - ballRadius,
 ballY - cameraY - ballRadius,
 ballRadius * 2,
 ballRadius * 2
 };
 
 return SDL_HasIntersection(&inventoryArea, &playerArea);
}

typedef struct {
 int x1, y1, x2, y2;
} Line;

typedef struct {
 int x1,y1,x2,y2;
} bush;

typedef struct{
 int x1,y1,x2,y2;
} spiky;


typedef struct {
 float x;
 float y;
 int radius;
 bool active;
 int direction; // 1 for right-to-left, 2 for left-to-right
} Projectile;

typedef struct {
 int x1, y1, x2, y2;
 bool isActive;
 Uint32 lastToggleTime;
} Fire;

typedef struct {
 int baseX1, baseY; 
 int baseX2; 
 float currentX; 
 bool expanding; 
 Uint32 lastUpdateTime;
} Spike;

// Modify the pendulum structure to store bob position
typedef struct {
 float anchorX, anchorY; // Anchor point
 float angle; // Current angle in radians
 float length; // Length of pendulum string
 int bobRadius; // Radius of pendulum bob
 float bobX, bobY; // Current bob position
} Pendulum;

// Initialize pendulum
Pendulum pendulum = {
 .anchorX = 600,
 .anchorY = -900,
 .angle = 0,
 .length = 100,
 .bobRadius = 15,
 .bobX = 600,
 .bobY = -800
};

#define FIRE_ON_DURATION 2000 
#define FIRE_OFF_DURATION 1000 
#define MAX_FIRES 3
#define MAX_PROJECTILES 300 
#define PROJECTILE_SPEED 7
#define SHOOT_INTERVAL 1500 
#define MAX_SPIKES 3
#define SPIKE_SPEED 2.0f 
#define SPIKE_UPDATE_INTERVAL 16


bush bushes[]={
 {1800,-725,1800,-570},
 {1775,-725,1775,-570},
 {1775,-725,1800,-725},
 {1775,-570,1800,-570},
 {0,-600,240,-600},
 {240,-750,240,-600},
 {240,-750,0,-750},
 {0,-750,0,-600},
 {1000,-350,1000,-175},
 {950,-350,950,-175},
 {950,-350,1000,-350},
 {950,-175,1000,-175},
 {800,-175,800,325},
 {850,-175,850,325},
 {800,-175,850,-175},
 {800,325,850,325},
 {1100,325,1100,0},
 {1200,325,1200,0},
 {1100,325,1200,325},
 {1100,0,1200,0}
};

spiky spik[]={
 {800,400,1200,400},
 {800,400,800,597},
 {1200,400,1200,597},
 {800,597,1200,597},
 {1000,-600,1000,-610},
 {1200,-600,1200,-610},
 {1000,-600,1200,-600},
 {1000,-610,1200,-610}
};
Line lines[] = {
 {0, 597, 1800, 597}, 
 {0, -900, 0, 597},
 {600, -400, 600, 325},
 {0,0,400,0},
 {200,-400,600,-400},
 {0,-350,400,-350},
 {0,-600,600,-600},
 {600,-800,600,-600},
 {300,-900,300,-750},
 {800,-900,800,-400},
 {0,-900,1800,-900},
 {200,-400,1000,-400},
 {800,-350,800,597},
 {1000,-400,1000,325},
 {1200,-725,1200,597},
 {1200,0,1300,0},
 {1200,-570,1300,-570},
 {1000,-600,1200,-600},
 {1000,-750,1000,-600},
 {1100,-900,1100,-650},
 {1300,250,1400,250},
 {1300,-250,1400,-250},
 {1300,-400,1300,-200},
 {1300,-650,1400,-650},
 {1400,-725,1400,597},
 {1300,250,1700,250},
 {1700,250,1700,0},
 {1500,150,1600,150},
 {1600,-250,1600,150},
 {1600,-250,1800,-250},
 {1400,-315,1600,-315},
 {1600,-400,1600,-315},
 {1500,-570,1700,-570},
 {1700,-650,1700,-570},
 {1500,-750,1500,-570},
 {1500,-750,1800,-750},
 {1800,-900,1800,597}
};

Spike spikes[MAX_SPIKES] = {
 {600, -350, 800, 600, false, 0}, 
 {600, -175, 800, 600, false, 0}, 
 {600, 0, 800, 600, false, 0} 
};

Fire fires[MAX_FIRES] = {
 {400, 0, 600, 0, true, 0}, 
 {1400, -650, 1500, -650, true, 0}, 
 {1400, -600, 1500, -600, true, 0} 
};

bool checkAndHandleCollision(int ballX, int ballY, int ballRadius, Line line) {
    int closestX = fmax(line.x1, fmin(ballX, line.x2));
    int closestY = fmax(line.y1, fmin(ballY, line.y2));
    int distanceX = ballX - closestX;
    int distanceY = ballY - closestY;
    
    return (distanceX * distanceX + distanceY * distanceY) < (ballRadius * ballRadius);
}



// Function to update pendulum physics
void updatePendulum() {
 // Calculate the desired angle based on time
 pendulum.angle = MAX_ANGLE * sin(SPEED * pendulumTime);
 
 // Calculate bob position based on this angle
 float bobX = pendulum.anchorX + sin(pendulum.angle) * pendulum.length;
 float bobY = pendulum.anchorY + cos(pendulum.angle) * pendulum.length;
 
 // Store the current position for drawing
 pendulum.bobX = bobX;
 pendulum.bobY = bobY;
}

// Function to draw pendulum
void drawPendulum(SDL_Renderer *renderer, int cameraX, int cameraY) {
 // Calculate bob position
 float bobX = pendulum.anchorX + sin(pendulum.angle) * pendulum.length;
 float bobY = pendulum.anchorY + cos(pendulum.angle) * pendulum.length;
 
 // Draw string
 SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
 SDL_RenderDrawLine(renderer,
 pendulum.anchorX - cameraX, pendulum.anchorY - cameraY,
 bobX - cameraX, bobY - cameraY);
 
 // Draw bob
 SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
 for (int w = 0; w < pendulum.bobRadius * 2; w++) {
 for (int h = 0; h < pendulum.bobRadius * 2; h++) {
 int dx = pendulum.bobRadius - w;
 int dy = pendulum.bobRadius - h;
 if ((dx * dx + dy * dy) <= (pendulum.bobRadius * pendulum.bobRadius)) {
 SDL_RenderDrawPoint(renderer,
 bobX - cameraX + dx - pendulum.bobRadius,
 bobY - cameraY + dy - pendulum.bobRadius);
 }
 }
 }
}

void updateSpikes(Uint32 currentTime) {
 for (int i = 0; i < MAX_SPIKES; i++) {
 Spike *spike = &spikes[i];
 
 if (currentTime - spike->lastUpdateTime >= SPIKE_UPDATE_INTERVAL) {
 if (spike->expanding) {
 spike->currentX += SPIKE_SPEED;
 if (spike->currentX >= spike->baseX2) {
 spike->expanding = false;
 }
 } else {
 spike->currentX -= SPIKE_SPEED;
 if (spike->currentX <= spike->baseX1) {
 spike->expanding = true;
 }
 }
 spike->lastUpdateTime = currentTime;
 }
 }
}

void drawSpikes(SDL_Renderer *renderer, int cameraX, int cameraY) {
 SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
 
 for (int i = 0; i < MAX_SPIKES; i++) {
 Spike *spike = &spikes[i];
 SDL_RenderDrawLine(renderer,
 spike->baseX1 - cameraX, spike->baseY - cameraY,
 spike->currentX - cameraX, spike->baseY - cameraY);
 }
}

void drawFire(SDL_Renderer *renderer, Fire *fire, int cameraX, int cameraY) {
 if (!fire->isActive) return;

 int steps = 20;
 for (int i = 0; i < steps; i++) {
 float t = (float)i / steps;
 int x = fire->x1 + (fire->x2 - fire->x1) * t;
 int y = fire->y1 + (fire->y2 - fire->y1) * t;
 
 int offsetY = rand() % 10;
 
 for (int j = 0; j < 3; j++) {
 int particleX = x + (rand() % 10 - 5);
 int particleY = y - offsetY + (rand() % 10 - 5);
 
 SDL_SetRenderDrawColor(renderer, 255,
 128 + (rand() % 128),
 0, 255);
 
 for (int dx = -2; dx <= 2; dx++) {
 for (int dy = -2; dy <= 2; dy++) {
 if (dx*dx + dy*dy <= 4) {
 SDL_RenderDrawPoint(renderer,
 particleX + dx - cameraX,
 particleY + dy - cameraY);
 }
 }
 }
 }
 }
}

void updateFires(Uint32 currentTime) {
 for (int i = 0; i < MAX_FIRES; i++) {
 Uint32 timeSinceLastToggle = currentTime - fires[i].lastToggleTime;
 
 if (fires[i].isActive && timeSinceLastToggle >= FIRE_ON_DURATION) {
 fires[i].isActive = false;
 fires[i].lastToggleTime = currentTime;
 }
 else if (!fires[i].isActive && timeSinceLastToggle >= FIRE_OFF_DURATION) {
 fires[i].isActive = true;
 fires[i].lastToggleTime = currentTime;
 }
 }
}

bool checkCollision(int ballX, int ballY, int ballRadius, Line line) {
 int closestX = fmax(line.x1, fmin(ballX, line.x2));
 int closestY = fmax(line.y1, fmin(ballY, line.y2));
 int distanceX = ballX - closestX;
 int distanceY = ballY - closestY;

 return (distanceX * distanceX + distanceY * distanceY) < (ballRadius * ballRadius);
}

void drawBall(SDL_Renderer *renderer, int x, int y, int radius, int cameraX, int cameraY) {
 for (int w = 0; w < radius * 2; w++) {
 for (int h = 0; h < radius * 2; h++) {
 int dx = radius - w;
 int dy = radius - h;
 if ((dx * dx + dy * dy) <= (radius * radius)) {
 SDL_RenderDrawPoint(renderer, x - cameraX + dx, y - cameraY + dy);
 }
 }
 }
}

void shootNewProjectiles(Projectile projectiles[]) {
 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 200;
 projectiles[i].y = -400;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 1; 
 break;
 }
 }
 
 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1400;
 projectiles[i].y = -250;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 2; 
 break;
 }
 }
 
 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1800;
 projectiles[i].y = -315;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 1; 
 break;
 }
 }

 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1600;
 projectiles[i].y = -175;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 1; 
 break;
 }
 }

 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1400;
 projectiles[i].y = -100;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 2; 
 break;
 }
 }

 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1600;
 projectiles[i].y = -25;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 1; 
 break;
 }
 }

 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1400;
 projectiles[i].y = 50;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 2; 
 break;
 }
 }

 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (!projectiles[i].active) {
 projectiles[i].x = 1600;
 projectiles[i].y = 125;
 projectiles[i].radius = 5;
 projectiles[i].active = true;
 projectiles[i].direction = 1; 
 break;
 }
 }
}

void drawGameElements(SDL_Renderer *renderer, int ballX, int ballY, int cameraX, int cameraY, Projectile projectiles[], int nearestKeyIndex, int ballRadius);


int main(int argc, char *argv[]) {
 SDL_Init(SDL_INIT_VIDEO);
 Player player1 = {
 .x = 420, .y = 320,
 .radius = 10,
 .isController = true,
 .window = SDL_CreateWindow(
 "Player 1 (Controller) - Limited Vision",
 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
 800, 600, 0
 ),
 };
 
 Player player2 = {
 .x = 420, .y = 320,
 .radius = 10,
 .isController = false,
 .window = SDL_CreateWindow(
 "Player 2 (Observer) - Full Vision",
 SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
 800, 600, 0
 ),
 };
 
 player1.renderer = SDL_CreateRenderer(player1.window, -1, SDL_RENDERER_ACCELERATED);
 player2.renderer = SDL_CreateRenderer(player2.window, -1, SDL_RENDERER_ACCELERATED);


 int ballX = 420, ballY = 320;
 int ballRadius = 10;
 int cameraX = 0, cameraY = 0;
 int speed = 5;
 const float PICKUP_RANGE = 50.0f; // Maximum distance to pick up keys
 int nearestKeyIndex = -1;
 
 int windowWidth = 800;
 int windowHeight = 600;
 initItems();
 
 Projectile projectiles[MAX_PROJECTILES] = {0};
 Uint32 lastShootTime = SDL_GetTicks();

 Uint32 currentTime = SDL_GetTicks();
 for (int i = 0; i < MAX_FIRES; i++) {
 fires[i].lastToggleTime = currentTime;
 }
 for (int i = 0; i < MAX_SPIKES; i++) {
 spikes[i].currentX = spikes[i].baseX1;
 spikes[i].lastUpdateTime = currentTime;
 }
 
 bool running = true;
 SDL_Event event;

 SDL_Surface *maskSurface = SDL_CreateRGBSurface(0, 800, 600, 32, 0, 0, 0, 0);
 SDL_Texture *maskTexture = SDL_CreateTexture(player1.renderer, 
 SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);

 while (running) {
 while (SDL_PollEvent(&event)) {
 if (event.type == SDL_QUIT) {
 running = false;
 }
 }
 

while (SDL_PollEvent(&event)) {
 if (event.type == SDL_QUIT) {
 running = false;
 }
 // Add spacebar detection for key pickup
 if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
 if (nearestKeyIndex != -1) {
 // Add key to inventory and deactivate it
 if (playerInventory.count < 10) {
 playerInventory.items[playerInventory.count] = items[nearestKeyIndex].type;
 playerInventory.count++;
 items[nearestKeyIndex].active = false;
 }
 }
 }
 }


 currentTime = SDL_GetTicks();
 updateFires(currentTime);
 updateSpikes(currentTime);
 pendulumTime += 0.016; // Increment pendulum time
 updatePendulum();

// Check for item collection - add this after player movement
 checkItemCollection(ballX, ballY, ballRadius);

 int prevBallX = ballX;
 int prevBallY = ballY;

 const Uint8 *state = SDL_GetKeyboardState(NULL);
 if (state[SDL_SCANCODE_UP]) ballY -= speed;
 if (state[SDL_SCANCODE_DOWN]) ballY += speed;
 if (state[SDL_SCANCODE_LEFT]) ballX -= speed;
 if (state[SDL_SCANCODE_RIGHT]) ballX += speed;

 if (currentTime - lastShootTime >= SHOOT_INTERVAL) {
 shootNewProjectiles(projectiles);
 lastShootTime = currentTime;
 }

for (int i = 0; i < 37; i++) {
    if (checkAndHandleCollision(ballX, ballY, ballRadius, lines[i])) {
        // Clean up SDL resources
        SDL_DestroyTexture(maskTexture);
        SDL_FreeSurface(maskSurface);
        SDL_DestroyRenderer(player1.renderer);
        SDL_DestroyRenderer(player2.renderer);
        SDL_DestroyWindow(player1.window);
        SDL_DestroyWindow(player2.window);
        SDL_Quit();
        return 0;
    }
}


 if (ballX + ballRadius > cameraX + 800) cameraX = ballX + ballRadius - 800;
 if (ballY - ballRadius < cameraY) cameraY = ballY - ballRadius;
 if (ballY + ballRadius > cameraY + 600) cameraY = ballY + ballRadius - 600;

 cameraX = ballX - windowWidth / 2;
 cameraY = ballY - windowHeight / 2;

 // Render for Player 1 (Controller) - Limited Vision
 SDL_SetRenderDrawColor(player1.renderer, 0, 0, 0, 255);
 SDL_RenderClear(player1.renderer);

 // Create visibility mask
 SDL_SetRenderTarget(player1.renderer, maskTexture);
 SDL_SetRenderDrawColor(player1.renderer, 0, 0, 0, 255);
 SDL_RenderClear(player1.renderer);

 // Draw visible circle
 SDL_SetRenderDrawColor(player1.renderer, 255, 255, 255, 255);
 for(int x = -VISIBILITY_RADIUS; x <= VISIBILITY_RADIUS; x++) {
 for(int y = -VISIBILITY_RADIUS; y <= VISIBILITY_RADIUS; y++) {
 if(x*x + y*y <= VISIBILITY_RADIUS*VISIBILITY_RADIUS) {
 SDL_RenderDrawPoint(player1.renderer, 
 400 + x, 300 + y);
 }
 }
 }

 // Reset render target and draw game elements
 SDL_SetRenderTarget(player1.renderer, NULL);
 
 // Draw game elements for Player 1
 drawGameElements(player1.renderer, ballX, ballY, cameraX, cameraY, 
 projectiles, nearestKeyIndex, ballRadius);

 SDL_SetRenderDrawBlendMode(player1.renderer, SDL_BLENDMODE_BLEND);
 SDL_RenderCopy(player1.renderer, maskTexture, NULL, NULL);
 SDL_SetTextureBlendMode(maskTexture, SDL_BLENDMODE_MOD);
 SDL_RenderCopy(player1.renderer, maskTexture, NULL, NULL);
 // Render for Player 2 (Observer) - Full Vision
 SDL_SetRenderDrawColor(player2.renderer, 0, 0, 0, 255);
 SDL_RenderClear(player2.renderer);
 
 // Draw all game elements for Player 2 (full visibility)
 drawGameElements(player2.renderer, ballX, ballY, cameraX, cameraY, 
 projectiles, nearestKeyIndex, ballRadius);

 // Present both renderers
 SDL_RenderPresent(player1.renderer);
 SDL_RenderPresent(player2.renderer);

for (int i = 0; i < 37; i++) {
    if (checkAndHandleCollision(ballX, ballY, ballRadius, lines[i])) {
        // Display end screen on both windows        
        // Wait for 2 seconds
        SDL_Delay(2000);
        
        // Clean up and exit
        SDL_DestroyTexture(maskTexture);
        SDL_FreeSurface(maskSurface);
        SDL_DestroyRenderer(player1.renderer);
        SDL_DestroyRenderer(player2.renderer);
        SDL_DestroyWindow(player1.window);
        SDL_DestroyWindow(player2.window);
        SDL_Quit();
        return 0;
    }
}


 SDL_Delay(16);
 }

 // Cleanup
 SDL_DestroyTexture(maskTexture);
 SDL_FreeSurface(maskSurface);
 SDL_DestroyRenderer(player1.renderer);
 SDL_DestroyRenderer(player2.renderer);
 SDL_DestroyWindow(player1.window);
 SDL_DestroyWindow(player2.window);
 SDL_Quit();

 return 0;
}

// New helper function to draw all game elements
void drawGameElements(SDL_Renderer *renderer, int ballX, int ballY, 
 int cameraX, int cameraY, Projectile projectiles[], int nearestKeyIndex, int ballRadius) {
 
 // Draw lines
 SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
 for (int i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
 SDL_RenderDrawLine(renderer,
 lines[i].x1 - cameraX, lines[i].y1 - cameraY,
 lines[i].x2 - cameraX, lines[i].y2 - cameraY);
 }

 // Draw bushes
 SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
 for(int i = 0; i < sizeof(bushes)/sizeof(bushes[0]); i++) {
 SDL_RenderDrawLine(renderer,
 bushes[i].x1 - cameraX, bushes[i].y1 - cameraY,
 bushes[i].x2 - cameraX, bushes[i].y2 - cameraY);
 }

 // Draw spiky elements
 SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
 for(int i = 0; i < sizeof(spik)/sizeof(spik[0]); i++) {
 SDL_RenderDrawLine(renderer,
 spik[i].x1 - cameraX, spik[i].y1 - cameraY,
 spik[i].x2 - cameraX, spik[i].y2 - cameraY);
 }

 // Draw items
 drawItems(renderer, cameraX, cameraY, nearestKeyIndex);

 // Draw projectiles
 SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
 for (int i = 0; i < MAX_PROJECTILES; i++) {
 if (projectiles[i].active) {
 drawBall(renderer, projectiles[i].x, projectiles[i].y,
 projectiles[i].radius, cameraX, cameraY);
 }
 }

 // Draw pendulum
 drawPendulum(renderer, cameraX, cameraY);

 // Draw spikes and fires
 drawSpikes(renderer, cameraX, cameraY);
 for (int i = 0; i < MAX_FIRES; i++) {
 drawFire(renderer, &fires[i], cameraX, cameraY);
 }

 // Draw player ball
 SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
 drawBall(renderer, ballX, ballY, ballRadius, cameraX, cameraY);

 // Draw inventory if needed
 if (!isPlayerInInventoryArea(ballX, ballY, ballRadius, cameraX, cameraY)) {
 drawInventory(renderer);
 }
}
