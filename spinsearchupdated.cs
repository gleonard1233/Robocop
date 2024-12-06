int spiral_length = 1; // Length of the forward movement, increases over time
int spin_count = 0; // Global variable to track the number of spins

void spin_search() {
    static float total_angle = 0.0; // Tracks cumulative spin angle
    drive(-0.07, 0.07, 0.1);       // Slight arc for searching
    total_angle += 0.1 * 360;      // Approximate angle based on motion

    if (total_angle >= 360.0) { // One full spin completed
        total_angle = 0.0;     // Reset for next spin
        spin_count++;          // Increment spin count
    }
}
while (true) {
    if (!have_pollen) {
        if (timer_elapsed()) {
            if (search_snapshot(0)) {
                // Object detected, approach it
                approach_object(0);
            } else {
                spin_search();

                // If the robot spins 2 times, drive forward and reset
                if (spin_count >= 5) {
                    stop();
                    drive(0.3, 0.3, spiral_length); // Drive forward
                    spiral_length++;               // Increase spiral search area
                    spin_count = 0;                // Reset spin counter
                }
            }
        }
    } else {
        if (search_snapshot(1)) {
            // Object detected, approach it
            approach_drop();
        } else {
            spin_search();
        }
    }
}
