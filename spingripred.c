#include <kipr/wombat.h>  // KIPR Wombat native library
#include <stdlib.h>       // General-purpose functions
#include <stdbool.h>      // Boolean support

// Define integer keys for each action type
#define SEEK_LIGHT_TYPE 0
#define SEEK_DARK_TYPE 1
#define APPROACH_TYPE 2
#define AVOID_TYPE 3
#define ESCAPE_F_TYPE 4
#define ESCAPE_B_TYPE 5
#define CRUISE_S_TYPE 6
#define CRUISE_A_TYPE 7

// Define PIN Addresses
#define RIGHT_IR_PIN 2
#define LEFT_IR_PIN 3
#define RIGHT_PHOTO_PIN 0
#define LEFT_PHOTO_PIN 1

#define FRONT_BUMP_LEFT_PIN 5
#define FRONT_BUMP_CENTER_PIN 3
#define FRONT_BUMP_RIGHT_PIN 4
#define BACK_BUMP_LEFT_PIN 2
#define BACK_BUMP_CENTER_PIN 0
#define BACK_BUMP_RIGHT_PIN 1

#define RIGHT_MOTOR_PIN 0
#define LEFT_MOTOR_PIN 1
#define GRIPPER_PIN 2

// Define gripper positions
#define GRIPPER_OPEN_POSITION 0
#define GRIPPER_CLOSED_POSITION 1023

// Global Variables
int hierarchy_length;
int timer_duration = 500;
unsigned long start_time = 0;

// Function Declarations
void initialize_camera();
bool search_snapshot(int channel);
void spin_search();
void approach_object();
void stop();
void drive(float left, float right, float delay_seconds);
bool timer_elapsed();
float map(float value, float start_range_low, float start_range_high, float target_range_low, float target_range_high);

// Main Function
int main() {    
    enable_servo(LEFT_MOTOR_PIN);
    enable_servo(RIGHT_MOTOR_PIN);
    enable_servo(GRIPPER_PIN);

   
    drive(0.0, 0.0, 1.0);
    
    initialize_camera();

    while (true) {
        if (timer_elapsed()) {
            if (search_snapshot(0)) {
                // Object detected, approach it
                approach_object();

        } else {
                // No object detected, continue spinning search
                spin_search();
            }
        }
    }
    return 0;
}

// Camera Initialization
void initialize_camera() {
    camera_load_config("blockz.conf");
    camera_open();
}

// Search Snapshot: Detects objects using the camera
bool search_snapshot(int channel) {
    camera_update();
    msleep(10);
    int object_count = get_object_count(channel);
    return object_count > 0; // Return true if any object is detected
}

// Spin Search: Spins in a slight arc to search for objects
void spin_search() {
    drive(-0.07, 0.07, 0.1); // Drive in an arc to cover a search area
}

// Approach Object: Drives forward until the object is no longer visible, then closes gripper
void approach_object() {
    
    stop(); // Stop once the object is no longer visible
    
    set_servo_position(GRIPPER_PIN, GRIPPER_OPEN_POSITION); // Open the gripper

    msleep(1000); // Wait for the gripper to open

    while (search_snapshot(0)) {
        drive(0.2, 0.2, 0.1); // Drive forward while object is visible
    }

    msleep(2000); // Wait for the gripper to open

    set_servo_position(GRIPPER_PIN, GRIPPER_CLOSED_POSITION); // Close the gripper
    
    msleep(1000); // Wait for the gripper to open


    

}


// Stop: Stops the robot
void stop() {
    drive(0.0, 0.0, 0.25);
}

// Drive Function: Controls motor speeds
void drive(float left, float right, float delay_seconds) {
    float left_speed = map(left, -1.0, 1.0, 0, 2047);
    float right_speed = map(right, -1.0, 1.0, 2047, 0);

    timer_duration = (int)(delay_seconds * 1000);
    start_time = systime();

    set_servo_position(LEFT_MOTOR_PIN, left_speed);
    set_servo_position(RIGHT_MOTOR_PIN, right_speed);
}

// Timer Elapsed: Checks if specified duration has passed
bool timer_elapsed() {
    return (systime() > (start_time + timer_duration));
}

// Map Function: Maps one range to another
float map(float value, float start_range_low, float start_range_high, float target_range_low, float target_range_high) {
    return target_range_low + ((value - start_range_low) / (start_range_high - start_range_low)) * (target_range_high - target_range_low);
}
 