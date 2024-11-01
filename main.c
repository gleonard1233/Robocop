#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

git config --global user.email "gleonard@vassar.edu"

// Placeholder functions for robot controls
void move_forward(float distance);
void move_backward();
void stop();
void grip();
void lift();
void lower_grip();
void drive(float speed_left, float speed_right, float duration);
void sleep_mode();
void spin_search();

// Constants for colors
const int POLLEN_COLOR = 1;   // Example color ID for pollen
const int FLOWER_COLOR = 2;   // Example color ID for flower

// Mock functions for sensor data
int check_light();
int detect_block(int *color, float *distance);
bool is_light_off();

void main() {
    while (true) {
        if (is_light_off()) {
            sleep_mode();
        }

        // Search for the pollen block
        int block1_color;
        float block1_distance;
        detect_block(&block1_color, &block1_distance);

        // If pollen block found, move toward it and grip
        if (block1_color == POLLEN_COLOR) {
            move_forward(block1_distance);
            grip();
            lift();
        }

        // Search for the flower block
        int block2_color = -1;
        float block2_distance = 0;

        while (block2_color != FLOWER_COLOR) {
            detect_block(&block2_color, &block2_distance);

            if (block2_color == FLOWER_COLOR) {
                move_forward(block2_distance);
                grip();  // Drops pollen
                move_backward();
                lower_grip();
            }
        }
    }
}

void sleep_mode() {
    stop();
    lower_grip();
}

int detect_block(int *color, float *distance) {
    bool block_found = false;

    while (!block_found) {
        // Example: Move in a random direction a small distance
        move_forward(1.0); // Placeholder for random movement
        spin_search();

        // Simulate seeing a block
        int detected_color = rand() % 3; // Example color IDs 0 (none), 1 (pollen), 2 (flower)
        float detected_distance = (float)(rand() % 10);

        if (detected_color == POLLEN_COLOR || detected_color == FLOWER_COLOR) {
            *color = detected_color;
            *distance = detected_distance;
            block_found = true;
        }
    }
    return 0;
}

void spin_search() {
    bool block_found = false;

    while (!block_found) {
        drive(1.0, -1.0, 0.5); // Spins the robot in place
        int color;
        float pos_y;
        float distance;
        
        // Simulate detecting a red or blue block (pollen/flower)
        int detected_block = rand() % 3;  // Example 0 (none), 1 (pollen), 2 (flower)
        
        if (detected_block == POLLEN_COLOR || detected_block == FLOWER_COLOR) {
            pos_y = rand() % 3 - 1; // Example position y
            distance = rand() % 10;

            if (pos_y >= -1 && pos_y <= 1) {  // Within view range
                block_found = true;
            }
        }
    }
}

bool is_light_off() {
    return check_light() == 0;
}
