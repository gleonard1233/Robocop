// Include Libraries
#include <kipr/wombat.h>  // KIPR Wombat native library
#include <stdlib.h>       // General-purpose functions
#include <stdbool.h>      // Boolean support
#include <math.h>

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

#define BACK_BUMP_LEFT_PIN 2
#define BACK_BUMP_CENTER_PIN 0
#define BACK_BUMP_RIGHT_PIN 1

#define RIGHT_MOTOR_PIN 0
#define LEFT_MOTOR_PIN 1
#define GRIPPER_PIN 2
#define LIFTER_PIN 3

// Define gripper positions
#define GRIPPER_OPEN_POSITION 0
#define GRIPPER_CLOSED_POSITION 1023

#define LIFTER_DOWN_POSITION 2030
#define LIFTER_UP_POSITION 0

// Global Variables
int hierarchy_length;
int timer_duration = 500;
unsigned long start_time = 0;
bool have_pollen = false;

// store all current sensor values accessible to all functions and updated by the "read_sensors" function
int right_ir_value, left_ir_value, back_bump_left_value, back_bump_center_value, back_bump_right_value;

// threshold values
int avoid_threshold = 6000;	   // the absolute difference between IR readings has to be above this for the avoid action

// Function Declarations
void initialize_camera();
bool search_snapshot(int channel);
void spin_search();
void approach_object();
void stop();
void drive(float left, float right, float delay_seconds);
bool timer_elapsed();
float map(float value, float start_range_low, float start_range_high, float target_range_low, float target_range_high);
bool is_pollinated();
void approach_drop();
void forward();
void dance(); // Function for the dance
void read_sensors();							 // read all sensor values and save to global variables
bool is_above_distance_threshold(int threshold); // return true if one and only one IR sensor is above the specified threshold
bool is_back_bump();							 // return true if one of the back bumpers was hit
void escape_back(); //initialize escape back function
void avoid(); //initialize avoid function

//Used for spin seach function
int spiral_length = 1; // Length of the forward movement, increases over time
int spin_count = 0; // Global variable to track the number of spins

//==================================//
//===============MAIN===============//
//==================================//

int main() {    
    enable_servo(LEFT_MOTOR_PIN);
    enable_servo(RIGHT_MOTOR_PIN);
    enable_servo(GRIPPER_PIN);
    enable_servo (LIFTER_PIN);

    unsigned long no_pollen_timer = systime(); // Timer to track time spent without finding pollen
   
    drive(0.0, 0.0, 1.0);
    
    initialize_camera();

while (true) {
    if (timer_elapsed()) {
        read_sensors(); // Read all sensors and set global variables of their readouts

        if (is_back_bump()) {
            escape_back();
            // continue;
        } else {
            if (is_above_distance_threshold(avoid_threshold)) {
                avoid();
                // continue;
            } else {
                if (systime() - no_pollen_timer > 30000) { // Check if 30 seconds have passed without detecting pollen
                    stop();
                    dance(); // Perform the dance
                    no_pollen_timer = systime(); // Reset timer after dance
                } else {
                    if (is_pollinated()) {
                        // Object detected, approach it
                        printf("pollinated!!");
                        spin_search(); // No object detected, continue spinning search
                         // If the robot spins 2 times, drive forward and reset
                if (spin_count >= 7) {
                    stop();
                    drive(0.3, 0.3, spiral_length); // Drive forward
                    spiral_length++;               // Increase spiral search area
                    spin_count = 0;                // Reset spin counter
                }
                    } else if (!have_pollen) {
                        if (search_snapshot(0)) {
                            // Object detected, approach it
                            approach_object(0);
                            no_pollen_timer = systime();
                            
                        } else {
                            // No object detected, continue spinning search
                            spin_search();
                             // If the robot spins 2 times, drive forward and reset
                if (spin_count >= 7) {
                    stop();
                    drive(0.3, 0.3, spiral_length); // Drive forward
                    spiral_length++;               // Increase spiral search area
                    spin_count = 0;                // Reset spin counter
                }
                        }
                    } else {
                        if (search_snapshot(1)) {
                            // Object detected, approach it
                            approach_drop();
                            no_pollen_timer = systime();

                        } else {
                            // No object detected, continue spinning search
                            spin_search();
                             // If the robot spins 2 times, drive forward and reset
                if (spin_count >= 7) {
                    stop();
                    drive(0.3, 0.3, spiral_length); // Drive forward
                    spiral_length++;               // Increase spiral search area
                    spin_count = 0;                // Reset spin counter
                }
                        }
                    }
                }
            }
        }
    }
}
return 0;
}

//=======================================//
//===============FUNCTIONS===============//
//=======================================//

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
    if (object_count > 0){
        printf("found");
    }
    return object_count > 0; // Return true if any object is detected
}

// Function to wait for the object to be centered in the camera's view
void wait_for_centered_object(int channel) {
    camera_update();
    msleep(10); // Small delay for camera update
    
    int object_count = get_object_count(channel);
    if (object_count == 0) {
        // No object detected, return
        return;
    }

    // Define the center of the camera's view (in pixels)
    int center_x = 80; // Assuming the camera resolution is 320x240 (center is 160 on x-axis)

    // Define a threshold for being "centered"
    int threshold = 35;  // Tolerance for being centered (±20 pixels)
    
    while (true) {
        camera_update();  // Continuously update the camera
		msleep(10);
        // Get the x-coordinate of the first detected object (assuming one object detected)
        int object_x = get_object_centroid(channel, 0).x;

        // Check if the object is within the centered threshold
        if (object_x >= (center_x - threshold) && object_x <= (center_x + threshold)) {
            // If the object is centered, break the loop and exit
            printf("x: %d", object_x);
            break;
        } else {
            // Otherwise, move the robot to center the object
            if (object_x < center_x - threshold) {
                // Move left to center the object
                drive(-0.07, 0.07, 0.1);
                msleep(10);
            } else if (object_x > center_x + threshold) {
                // Move right to center the object
                drive(0.07, -0.07, 0.1);
                msleep(10);
            }
            msleep(10); // Small delay to avoid excessive updates
        }
    }
}


// Is pollinated Boolean loop : checks if the flower the camera sees has pollen on it
bool is_pollinated() {
    camera_update();
    msleep(10);

    int red_count = get_object_count(0);
    int blue_count = get_object_count(1);

    if (red_count + blue_count < 1){
        return false;
    }

    int redx  = (get_object_centroid(0, 0)).x ;
    int redy  = (get_object_centroid(0, 0)).y ;
    int bluex  = (get_object_centroid(1, 0)).x ;
    int bluey  = (get_object_centroid(1, 0)).y ;
    
    int x = redx - bluex;
    int y = redy - bluey;
    
    float dist = sqrt( pow(x, 2) + pow(y, 2) );
    
 		if (dist < 30){
        printf("nearby");
        return true;
    }
    return false;
}

// Dance: Perform a dance by alternating motor movements
void dance() {
    unsigned long dance_start = systime(); // Track the start time of the dance
    while (systime() - dance_start < 10000) { // Dance for 10 seconds
        // Move left wheel forward, right wheel backward (spin in place)
        drive(0.2, -0.2, 0.5);  // 0.5 seconds of movement
        msleep(500);  // Pause for 0.5 seconds
        // Move right wheel forward, left wheel backward (spin in place)
        drive(-0.2, 0.2, 0.5);  // 0.5 seconds of movement
        msleep(500);  // Pause for 0.5 seconds
        drive(0.2, -0.2, 0.5);  // 0.5 seconds of movement
        msleep(500);  // Pause for 0.5 seconds
        // Move right wheel forward, left wheel backward (spin in place)
        drive(-0.2, 0.2, 0.5);  // 0.5 seconds of movement
        msleep(500);  // Pause for 0.5 seconds
        drive(0.2, -0.2, 0.5);  // 0.5 seconds of movement
        msleep(500);  // Pause for 0.5 seconds
        // Move right wheel forward, left wheel backward (spin in place)
        drive(-0.2, 0.2, 0.5);  // 0.5 seconds of movement
        msleep(500);  // Pause for 0.5 seconds
    }
}

// Spin Search
void spin_search() {
    static float total_angle = 0.0; // Tracks cumulative spin angle
    drive(-0.07, 0.07, 0.1);       // Slight arc for searching
    total_angle += 0.1 * 360;      // Approximate angle based on motion

    if (total_angle >= 360.0) { // One full spin completed
        total_angle = 0.0;     // Reset for next spin
        spin_count++;          // Increment spin count
    }
}

// Approach Object: Drives forward until the object is no longer visible, then closes gripper
void approach_object(channel) {
    stop(); // Stop once the object is no longer visible
    wait_for_centered_object(channel);  // This function will block until the object is centered
    stop();
    msleep(1000);
    set_servo_position(LIFTER_PIN,LIFTER_DOWN_POSITION);
    msleep(1000);
    set_servo_position(GRIPPER_PIN, GRIPPER_OPEN_POSITION); // Open the gripper
    msleep(1000); // Wait for the gripper to open
    while (search_snapshot(channel)) {
        forward(); // Drive forward while object is visible
        msleep(200);
    }
    stop();
    msleep(1000);
    set_servo_position(GRIPPER_PIN, GRIPPER_CLOSED_POSITION); // Close the gripper
    msleep(1000); // Wait for the gripper to close
    set_servo_position(LIFTER_PIN, LIFTER_UP_POSITION);
    msleep(1000); 
    have_pollen = true;

}

void approach_drop() {
        stop(); // Stop once the object is no longer visible
    
    wait_for_centered_object(1);
    stop();
    msleep(1000);
    while (search_snapshot(1)) {
        forward(); // Drive forward while object is visible
        msleep(200);
    }
        stop();
    msleep(1000); // Wait for the gripper to open
    set_servo_position(GRIPPER_PIN, GRIPPER_OPEN_POSITION); // Close the gripper
    have_pollen = false;  
    msleep(1000); // Wait for the gripper to close
    set_servo_position(LIFTER_PIN, LIFTER_UP_POSITION);
    msleep(1000);
    drive(-1.0, -1.0, 0.2);
}

// Stop: Stops the robot
void stop() {
    drive(0.0, 0.0, 0.25);
}

void stop_plain() {
    set_servo_position(LEFT_MOTOR_PIN, 0);
    set_servo_position(RIGHT_MOTOR_PIN, 0);
}
void forward() {
    set_servo_position(LEFT_MOTOR_PIN, 1500);
    set_servo_position(RIGHT_MOTOR_PIN, 880);
}

//Avoid function

void avoid()
{
	if (left_ir_value > avoid_threshold)
	{
		drive(0.5, -0.5, 0.1);
	}

	else if (right_ir_value > avoid_threshold)
	{
		drive(-0.5, 0.5, 0.1);
	}
}

//escape back function

void escape_back()
{
	drive(0.9, 0.9, 0.25); //drive forward a little
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

//reads all of the sensors

void read_sensors()
{
	right_ir_value = analog_et(RIGHT_IR_PIN);				// read the IR sensor at RIGHT_IR_PIN
	left_ir_value = analog_et(LEFT_IR_PIN);					// read the IR sensor at LEFT_IR_PIN
	// read the bumpers
	back_bump_left_value = digital(BACK_BUMP_LEFT_PIN);	// read the bumper at BACK_BUMP_LEFT_PIN
	back_bump_center_value = digital(BACK_BUMP_CENTER_PIN);  // read the bumper at BACK_BUMP_CENTER_PIN
	back_bump_right_value = digital(BACK_BUMP_RIGHT_PIN);	// read the bumper at BACK_BUMP_RIGHT_PIN	
}
/******************************************************/

// Used for avoid function

bool is_above_distance_threshold(int threshold)
{
	return (left_ir_value > threshold || right_ir_value > threshold) && !(left_ir_value > threshold && right_ir_value > threshold);
	// returns true if one (exclusive) IR value is above the threshold, otherwise false
}

// Used for Escape back function

bool is_back_bump()
{
	return (back_bump_left_value == 1 || back_bump_center_value == 1 || back_bump_right_value == 1); // return true if one of the back bump values is 1, otherwise false
}

// Map Function: Maps one range to another

float map(float value, float start_range_low, float start_range_high, float target_range_low, float target_range_high) {
    return target_range_low + ((value - start_range_low) / (start_range_high - start_range_low)) * (target_range_high - target_range_low);
}
 