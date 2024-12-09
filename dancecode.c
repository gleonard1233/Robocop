//global variables
unsigned long start_time = 0;
unsigned long no_pollen_timer = 0; // Timer to track time spent without finding pollen

//function declarations
bool timer_elapsed();
void dance(); // Function for the dance

// Dance: Perform a dance by alternating motor movements
void dance() {
    unsigned long dance_start = systime(); // Track the start time of the dance
    while (systime() - dance_start < 10000) { // Dance for 10 seconds
        // Move left wheel forward, right wheel backward (spin in place)
        drive(0.2, -0.2, 0.5);  // 0.5 seconds of movement
        stop();
        msleep(500);  // Pause for 0.5 seconds

        // Move right wheel forward, left wheel backward (spin in place)
        drive(-0.2, 0.2, 0.5);  // 0.5 seconds of movement
        stop();
        msleep(500);  // Pause for 0.5 seconds
    }
}


 // Check if 30 seconds have passed without detecting pollen (add within main function after spincount)
                    if (systime() - no_pollen_timer > 30000) { // 30 seconds
                        stop();
                        dance(); // Perform the dance
                        no_pollen_timer = systime(); // Reset timer after dance
                    }