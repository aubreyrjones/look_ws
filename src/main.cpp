//
// Created by thejackimonster on 29.03.23.
//
// Copyright (c) 2023 thejackimonster. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "device3.h"
#include "device4.h"

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <math.h>

#include <iostream>
#include <cstdlib>
#include <string>

// Function to switch workspace in i3 window manager
void switchWorkspace(const std::string& workspace) {
    // Build the command string
    std::string command = "i3-msg -q workspace " + workspace;

    // Execute the command using system() function
    int result = std::system(command.c_str());

    // Check the result of the command execution
    if (result != 0) {
        std::cerr << "Failed to switch workspace." << std::endl;
    }
}

int zone = 0;

void test3(uint64_t timestamp,
		   device3_event_type event,
		   const device3_ahrs_type* ahrs) {
	static device3_quat_type old;
	static float dmax = -1.0f;
	
	if (event != DEVICE3_EVENT_UPDATE) {
		return;
	}
	
	device3_quat_type q = device3_get_orientation(ahrs);
	
	device3_euler_type e = device3_get_euler(q);
	
	float d = fabs(e.yaw - device3_get_euler(old).yaw);

	if (e.yaw < -15 && zone != -1) {
		std::cout << "Switching to workspace 1" << std::endl;
		switchWorkspace("1:web");
		zone = -1;
	}
	else if (e.yaw > 15 && zone != 1) {
		std::cout << "Switching to workspace 3" << std::endl;
		switchWorkspace("3:code");
		zone = 1;
	}
	else if (e.pitch > 15 && zone != 2) {
		std::cout << "Switching to workspace 2" << std::endl;
		switchWorkspace("2:term");
		zone = 2;
	}
	else if (e.yaw > -15 && e.yaw < 15 && e.pitch < 15) {
		zone = 0;
	}
	
	old = q;
}

void test4(uint64_t timestamp,
		   device4_event_type event,
		   uint8_t brightness,
		   const char* msg) {
	switch (event) {
		case DEVICE4_EVENT_MESSAGE:
			printf("Message: `%s`\n", msg);
			break;
		case DEVICE4_EVENT_BRIGHTNESS_UP:
			printf("Increase Brightness: %u\n", brightness);
			break;
		case DEVICE4_EVENT_BRIGHTNESS_DOWN:
			printf("Decrease Brightness: %u\n", brightness);
			break;
		default:
			break;
	}
}

int main(int argc, const char** argv) {
	pid_t pid = fork();
	
	if (pid == -1) {
		perror("Could not fork!\n");
		return 1;
	}
	
	if (pid == 0) {
		device3_type dev3;
		if (DEVICE3_ERROR_NO_ERROR != device3_open(&dev3, test3)) {
			printf("It's not type 3.\n");
			return 1;
		}

		device3_clear(&dev3);
		device3_calibrate(&dev3, 1000, true, true, false);
		while (DEVICE3_ERROR_NO_ERROR == device3_read(&dev3, 0));
		device3_close(&dev3);
		return 0;
	} else {
		int status = 0;

		device4_type dev4;
		if (DEVICE4_ERROR_NO_ERROR != device4_open(&dev4, test4)) {
			status = 1;
			printf("It's not type 4.\n");
			goto exit;
		}

		device4_clear(&dev4);
		while (DEVICE4_ERROR_NO_ERROR == device4_read(&dev4, 0));
		device4_close(&dev4);
		
	exit:
		if (pid != waitpid(pid, &status, 0)) {
			return 1;
		}
		
		return status;
	}
}
