struct GROUNDED_HORIZONTAL_KINEMATICS {
    static constexpr double MIN_WALK_SPEED           = 0.296875;
    static constexpr double MAX_WALK_SPEED           = 6.25;
    static constexpr double MAX_RUN_SPEED            = 10.25;
    static constexpr double SKID_TURNAROUND_SPEED    = 2.25;
    static constexpr double WALK_ACC                 = 0.1484375;
    static constexpr double RUN_ACC                  = 0.22265625;
    static constexpr double RELEASE_DEC              = 0.203125;
    static constexpr double SKID_DEC                 = 0.40625;
};

struct AIRBORNE_HORIZONTAL_KINEMATICS {
    static constexpr double CURRENT_SPEED_THRESHOLD_FOR_ACC = 6.25; // CST
    static constexpr double INITIAL_SPEED_THRESHOLD_FOR_ACC = 7.25; // IST

    static constexpr double BELOW_CST_ACC = 0.1484375; // Acceleration when current speed < CST
    static constexpr double ABOVE_CST_ACC = 0.22265625; // Acceleration when current speed >= CST

    static constexpr double ABOVE_CST_DEC = 0.22265625;  // Deceleration when current speed >= CST
    static constexpr double ABOVE_IST_DEC = 0.203125; // Deceleration when IST <= current speed < CST
    static constexpr double BELOW_IST_DEC = 0.1484375; // Deceleration when current speed < IST

    static constexpr double INITIAL_SPEED_THRESHOLD_FOR_VEL = 6.25;
    static constexpr double BELLOW_ISP_SPEED_LIMIT_VEL = 6.25; // Speed limit if mario started airborne at x speed bellow CST
    static constexpr double ABOVE_IST_SPEED_LIMIT_VEL = 10.25; // Speed limit if mario started airborne at x speed above CST
};

struct AIRBORNE_VERTICAL_KINEMATICS {
    static constexpr double SMALL_SPEED_THRESHOLD = 4; // SST
    static constexpr double MEDIUM_SPEED_THRESHOLD = 9.2490234375; // MST
    static constexpr double LARGE_SPEED_THRESHOLD = 9.25; // LST

    // < SST
    static constexpr double INITIAL_VELOCITY_S = 16;
    static constexpr double REDUCED_GRAVITY_S = 0.5;
    static constexpr double GRAVITY_S = 1.75;

    // >= SST AND <= MST
    static constexpr double INITIAL_VELOCITY_M = 16;
    static constexpr double REDUCED_GRAVITY_M = 0.46875;
    static constexpr double GRAVITY_M = 1.5;

    // >= LST
    static constexpr double INITIAL_VELOCITY_L = 20;
    static constexpr double REDUCED_GRAVITY_L = 0.625;
    static constexpr double GRAVITY_L = 2.25;

    static constexpr double MAX_DOWNWARD_SPEED = 18; // Max speed mario can move downward
    static constexpr double RESET_DOWNWARD_SPEED = 16; // Speed to reset to when mario exceeds max downward speed

    static constexpr double GOOMBA_STOMP_VELOCITY = 17.25;
};