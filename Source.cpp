#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <time.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED

#include "SDL.h"

constexpr auto mapWidth = 24;
constexpr auto mapHeight = 24;
constexpr auto screenWidth = 640;
constexpr auto screenHeight = 480;

int oldTime = 0;
double frameTime = 0;

static const Uint8* readKeys() {
	return SDL_GetKeyboardState(NULL);
}

static int keyDown(SDL_Scancode key) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	return state[key];
}

int worldMap[mapWidth][mapHeight] =
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static int done() {
	fflush(stdout);
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			printf("Goodbye\n");
			return 1;
		}
	}
	return 0;
}

int main(int /*argc*/, char /*argv*/[]) {
	double posX = 22, posY = 12;
	double dirX = -1, dirY = 0;
	double planeX = 0, planeY = 0.66;
	double time = 0;
	double oldTime = 0;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Non-Euclidean Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	while (!done()) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		for (int x = 0; x < screenWidth; x++) {
			double cameraX = 2 * x / double(screenWidth) - 1;
			double rayDirX = dirX + planeX * cameraX; 
			double rayDirY = dirY + planeY * cameraX;

			double rayAngle = atan2(rayDirY, rayDirX);
			double sphericalRadius = 2;
			double rayDist = sphericalRadius * rayAngle;

			int mapX = int(posX);
			int mapY = int(posY);
			double sideDistX = 0.0;
			double sideDistY = 0.0;

			double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
			double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
			double perpWallDist = 0.0;

			int stepX = 0;
			int stepY = 0;

			int hit = 0;
			int side = 0;

			//calculate stuff

			if (rayDirX < 0) {
				stepX = -1;
				sideDistX = (posX - mapX) * deltaDistX;
			}
			else {
				stepX = 1;
				sideDistX = (mapX + 1.0 - posX) * deltaDistX;
			}
			if (rayDirY < 0) {
				stepY = -1;
				sideDistY = (posY - mapY) * deltaDistY;
			}
			else {
				stepY = 1;
				sideDistY = (mapY + 1.0 - posY) * deltaDistY;
			}

			while (hit == 0) {
				if (sideDistX < sideDistY) {
					sideDistX += deltaDistX;
					mapX += stepX;
					side = 0;
				}
				else {
					sideDistY += deltaDistY;
					mapY += stepY;
					side = 1;
				}
				if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
					hit = 1; // out of bounds, treat as hit
					break; // Avoid accessing invalid memory
				}
				if (worldMap[mapX][mapY] > 0) hit = 1;
			}
			if (side == 0) {
				perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
			}
			else {
				perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
			}

			perpWallDist = rayDist;

			int lineHeight = (int)(screenHeight / perpWallDist);
			int drawStart = -lineHeight / 2 + screenHeight / 2;
			if (drawStart < 0)drawStart = 0;
			int drawEnd = lineHeight / 2 + screenHeight / 2;
			if (drawEnd >= screenHeight)drawEnd = screenHeight - 1;

			SDL_Color color;
			switch (worldMap[mapX][mapY]) {
			case 1: color = { 255, 0, 0, 255 }; break;
			case 2: color = { 0, 255, 0, 255 }; break;
			case 3: color = { 0 , 0 , 255, 255 }; break;
			case 4: color = { 255, 255, 255, 255 }; break;
			case 5: color = { 255, 255, 0, 255 }; break;
			}
			if (side == 1) {
				color.r /= 2;
				color.g /= 2;
				color.b /= 2;
			}

			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_RenderDrawLine(renderer, x, drawStart, x, drawEnd);
			
		}
		time = SDL_GetTicks(); 
		frameTime = (time - oldTime) / 1000.0;
		oldTime = time;

		const double moveSpeed = frameTime * 5.0;
		const double rotSpeed = frameTime * 3.0;

		readKeys();
		//move forward if no wall in front of you
		if (keyDown(SDL_SCANCODE_UP))
		{
			if (worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
			if (worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
		}
		//move backwards if no wall behind you
		if (keyDown(SDL_SCANCODE_DOWN))
		{
			if (worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
			if (worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
		}
		//rotate to the right
		if (keyDown(SDL_SCANCODE_RIGHT))
		{
			//both camera direction and camera plane must be rotated
			double oldDirX = dirX;
			dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
			dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
			planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
		}
		//rotate to the left
		if (keyDown(SDL_SCANCODE_LEFT))
		{
			//both camera direction and camera plane must be rotated
			double oldDirX = dirX;
			dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
			dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
			planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);

		}
		SDL_RenderPresent(renderer);
	}
}