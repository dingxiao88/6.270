#include <joyos.h>
#include <stdint.h>

#include "platform.h"

int16_t pastL_vel;
int16_t pastR_vel;

struct lock pause_lock;

void platform_init(void) {
    platform_reverse = 0;
    platform_pause = 0;
    pastL_vel = 0;
    pastR_vel = 0;
    
    // Calibrate gyro
    printf("Calibrating gyro...");
    pause(100);
    gyro_init(GYRO_PORT, LSB_US_PER_DEG, 1000L);
    printf("done.\n");
    
    init_lock(&pause_lock, "pause_lock");
    
    // Set initial servo positions
    triggerBack();
    leverUp();
}

void pauseMovement() {
    acquire(&pause_lock);
    platform_pause = 1;
    motor_set_vel(L_MOTOR_PORT, 0);
    motor_set_vel(R_MOTOR_PORT, 0);
    motor_brake(L_MOTOR_PORT);
    motor_brake(R_MOTOR_PORT);
    release(&pause_lock);
}

void unpauseMovement() {
    acquire(&pause_lock);
    platform_pause = 0;
    release(&pause_lock);
}

void setReversed(int reversed) {
    platform_reverse = reversed;
}

float getHeading(void) {
    if (platform_reverse) {
        return gyro_get_degrees() + 180;
    } else {
        return gyro_get_degrees();
    }
}

void setLRMotors(int16_t l_vel, int16_t r_vel) {
    int paused;
    acquire(&pause_lock);
    paused = platform_pause;
    release(&pause_lock);
    if (paused) {
        return;
    }
    
    if (l_vel > 255) {
        l_vel = 255;
    } else if (l_vel < -255) {
        l_vel = -255;
    }
    
    if (r_vel > 255) {
        r_vel = 255;
    } else if (r_vel < -255) {
        r_vel = -255;
    }

    if(abs(l_vel) < SETPOINT_MIN_VEL){
        if(l_vel < 0){
            l_vel = -SETPOINT_MIN_VEL;
	    } else {
            l_vel = SETPOINT_MIN_VEL;
        }
	}

    if(abs(r_vel) < SETPOINT_MIN_VEL) {
        if(r_vel < 0) {
            r_vel = -SETPOINT_MIN_VEL;
        } else {
            r_vel = SETPOINT_MIN_VEL;
        }
    }



    
  if (platform_reverse) {
    if (abs(-r_vel-pastL_vel) > SETPOINT_MAX_DERIV) {
      if (pastL_vel < -r_vel){
	      r_vel = -(pastL_vel + SETPOINT_MAX_DERIV);
      } else {
	      r_vel = -(pastL_vel - SETPOINT_MAX_DERIV);
      }
    }
    
    if (abs(-l_vel-pastR_vel) > SETPOINT_MAX_DERIV) {
      if (pastR_vel < -l_vel) {
	      l_vel = -(pastR_vel + SETPOINT_MAX_DERIV);
      } else {
	      l_vel = -(pastR_vel - SETPOINT_MAX_DERIV);
      }
    }
    motor_set_vel(L_MOTOR_PORT, -r_vel);
    motor_set_vel(R_MOTOR_PORT, -l_vel);
    pastR_vel = -l_vel;
    pastL_vel = -r_vel;
  
  } else {
    if (abs(l_vel-pastL_vel) > SETPOINT_MAX_DERIV) {
      if (pastL_vel < l_vel) {
	      l_vel = pastL_vel + SETPOINT_MAX_DERIV;
      } else {
	      l_vel = pastL_vel - SETPOINT_MAX_DERIV;
      }
    }
    
    if (abs(r_vel-pastR_vel) > SETPOINT_MAX_DERIV) {
      if (pastR_vel < r_vel) {
	      r_vel = pastR_vel + SETPOINT_MAX_DERIV;
      } else {
	      r_vel = pastR_vel - SETPOINT_MAX_DERIV;
      }
    }    
    
    motor_set_vel(L_MOTOR_PORT, l_vel);
    motor_set_vel(R_MOTOR_PORT, r_vel);
    pastR_vel = r_vel;
    pastL_vel = l_vel;
  }
}
  
void triggerForward(void) {
    setTriggerPosition(TRIGGER_LOWER_LIMIT);
}
void triggerBack(void) {
    setTriggerPosition(TRIGGER_UPPER_LIMIT);
}
void setTriggerPosition(uint16_t pos) {
    if (pos < TRIGGER_LOWER_LIMIT) {
        pos = TRIGGER_LOWER_LIMIT;
    } else if (pos > TRIGGER_UPPER_LIMIT) {
        pos = TRIGGER_UPPER_LIMIT;
    }
    
    servo_set_pos(TRIGGER_SERVO_PORT, pos);
}

void leverUp(void) {
    setLeverPosition(LEVER_LOWER_LIMIT);
}
void leverDown(void) {
    setLeverPosition(LEVER_UPPER_LIMIT);
}
void setLeverPosition(uint16_t pos) {
    if (pos < LEVER_LOWER_LIMIT) {
        pos = LEVER_LOWER_LIMIT;
    } else if (pos > LEVER_UPPER_LIMIT) {
        pos = LEVER_UPPER_LIMIT;
    }
    
    servo_set_pos(LEVER_SERVO_PORT, pos);
}
