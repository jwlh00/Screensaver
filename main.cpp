#include <SDL2/SDL.h>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
int NUM_CIRCLES = 100;  // Default number of circles, can be modified via command line
int global_num_threads = 0; 

// Struct representing a circle with position, size, velocity, and color properties
struct Circle {
    int x, y;           // Position of the circle
    int radius;         // Radius of the circle
    int dx, dy;         // Velocity (direction and speed) in x and y
    SDL_Color color;    // Color of the circle
};

// Vector to store all the circles
std::vector<Circle> circles;

// Initialize the circles with random positions, velocities, sizes, and colors
void initialize_circles() {
    circles.clear(); // Ensure the circles vector is empty before initializing
    srand(time(NULL)); // Seed the random number generator

    // Generate properties for each circle and store them in the vector
    for (int i = 0; i < NUM_CIRCLES; ++i) {
        Circle c;
        c.x = rand() % SCREEN_WIDTH;
        c.y = rand() % SCREEN_HEIGHT;
        c.radius = 10 + rand() % 40;
        c.dx = (rand() % 5) - 2; // Random speed in x direction
        c.dy = (rand() % 5) - 2; // Random speed in y direction
        
        // Assign a random color to the circle
        c.color = {Uint8(rand() % 256), Uint8(rand() % 256), Uint8(rand() % 256), 255};
        circles.push_back(c);
    }
}

// Draw a circle 
void SDL_RenderDrawCircle(SDL_Renderer* renderer, int x_center, int y_center, int radius) {
    int x = radius;
    int y = 0;
    int p = 1 - radius;

    if (radius == 0) {
        SDL_RenderDrawPoint(renderer, x_center, y_center);
        return;
    }

    // Drawing four initial points for given radius
    SDL_RenderDrawPoint(renderer, x_center + radius, y_center);
    SDL_RenderDrawPoint(renderer, x_center - radius, y_center);
    SDL_RenderDrawPoint(renderer, x_center, y_center + radius);
    SDL_RenderDrawPoint(renderer, x_center, y_center - radius);

    //Bresenham's algorithm 
    while (x > y) {
        y++;
        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x--;
            p = p + 2 * y - 2 * x + 1;
        }

        if (x < y)
            break;

        // Drawing points generated by the algorithm
        SDL_RenderDrawPoint(renderer, x_center + x, y_center + y);
        SDL_RenderDrawPoint(renderer, x_center - x, y_center + y);
        SDL_RenderDrawPoint(renderer, x_center + x, y_center - y);
        SDL_RenderDrawPoint(renderer, x_center - x, y_center - y);

        if (x != y) {
            SDL_RenderDrawPoint(renderer, x_center + y, y_center + x);
            SDL_RenderDrawPoint(renderer, x_center - y, y_center + x);
            SDL_RenderDrawPoint(renderer, x_center + y, y_center - x);
            SDL_RenderDrawPoint(renderer, x_center - y, y_center - x);
        }
    }
}

// Move circles based on their velocity in a sequential manner
void move_circles_sequential() {
    for (int i = 0; i < NUM_CIRCLES; ++i) {
        circles[i].x += circles[i].dx;
        circles[i].y += circles[i].dy;

        // Boundary check: If a circle goes beyond the screen, reverse its direction
        if (circles[i].x < 0 || circles[i].x > SCREEN_WIDTH) {
            circles[i].dx = -circles[i].dx;
        }
        if (circles[i].y < 0 || circles[i].y > SCREEN_HEIGHT) {
            circles[i].dy = -circles[i].dy;
        }
    }
}

// Move circles based on their velocity using parallel processing
void move_circles_parallel() {
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic, 10)
        for (int i = 0; i < NUM_CIRCLES; ++i) {
            Circle &c = circles[i];
            int new_x = c.x + c.dx;
            int new_y = c.y + c.dy;

            // Check for boundary collision
            if (new_x < 0 || new_x > SCREEN_WIDTH) {
                c.dx = -c.dx;  // Reverse x-direction
                new_x = c.x + c.dx; // Update new_x after reversing direction
            }
            if (new_y < 0 || new_y > SCREEN_HEIGHT) {
                c.dy = -c.dy;  // Reverse y-direction
                new_y = c.y + c.dy; // Update new_y after reversing direction
            }
            
            c.x = new_x;
            c.y = new_y;
        }
    }
}

// Main loop for the sequential screensaver
int main_sequential() {
    SDL_Init(SDL_INIT_VIDEO); // Initialize SDL video subsystem
    SDL_Window* window = SDL_CreateWindow("Screensaver Sequential", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Create a new renderer
    
    bool running = true; // Flag to keep track of the running state
    SDL_Event e;

    // Variables for FPS tracking
    int frameCount = 0;
    int lastTime = SDL_GetTicks(), currentTime;
    float fps;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            // Check for quit event to exit the loop
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        
        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Move the circles and draw them
        move_circles_sequential();
        for (const Circle& c : circles) {
            SDL_SetRenderDrawColor(renderer, c.color.r, c.color.g, c.color.b, c.color.a);
            SDL_RenderDrawCircle(renderer, c.x, c.y, c.radius);
        }

        // Update the screen with the new render
        SDL_RenderPresent(renderer);

        // FPS tracking logic
        frameCount++;
        currentTime = SDL_GetTicks();
        
        if (currentTime > lastTime + 1000) {  // update every second
            fps = frameCount / ((currentTime - lastTime) / 1000.f);
            std::cout << "FPS: " << fps << std::endl;

            lastTime = currentTime;
            frameCount = 0;
        }
    }

    // Clean up resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


// Main loop for the parallel screensaver
int main_parallel() {
    SDL_Init(SDL_INIT_VIDEO); // Initialize SDL video subsystem
    SDL_Window* window = SDL_CreateWindow("Screensaver Parallel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Create a new renderer

    bool running = true; // Flag to keep track of the running state
    SDL_Event e;

    // Variables for FPS tracking
    int frameCount = 0;
    int lastTime = SDL_GetTicks(), currentTime;
    float fps;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            // Check for quit event to exit the loop
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Move the circles and draw them using parallel processing
        move_circles_parallel();
        for (const Circle& c : circles) {
            SDL_SetRenderDrawColor(renderer, c.color.r, c.color.g, c.color.b, c.color.a);
            SDL_RenderDrawCircle(renderer, c.x, c.y, c.radius);
        }

        // Update the screen with the new render
        SDL_RenderPresent(renderer);

        // FPS tracking logic
        frameCount++;
        currentTime = SDL_GetTicks();
        
        if (currentTime > lastTime + 1000) {  // update every second
            fps = frameCount / ((currentTime - lastTime) / 1000.f);
            std::cout << "FPS: " << fps << std::endl;

            lastTime = currentTime;
            frameCount = 0;
        }
    }

    // Clean up resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

// Main entry point for the program
int main(int argc, char* argv[]) {
    // Check if the number of circles is provided as a command-line argument
    if (argc > 1) {
        int num = std::stoi(argv[1]);
        if (num > 0) {
            NUM_CIRCLES = num; // Update global circle count
        } else {
            std::cerr << "Please provide a positive integer for the number of circles." << std::endl;
            return 1;
        }
    }
    initialize_circles(); // Initialize circles based on NUM_CIRCLES value

    // Choose which version to run: sequential or parallel
    return main_sequential();
    // return main_parallel();
}
