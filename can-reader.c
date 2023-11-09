#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>     // sockets local interfaces
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>  // use select sys call to check for keyboard input
// select allows you to monitor multiple file descriptors, waiting until on or more 
// of the FDs become ready 

#include <linux/can.h>
#include <linux/can/raw.h>


int main(int argc, char **argv){
    int s;  // socket file descriptor
    int i; 

    fd_set rfds; // read file descriptor set
    struct timeval tv;
    int retval;

    int nbytes;
    struct ifreq ifr;   // struct used for ioctl about an interface
    struct sockaddr_can addr;
    struct can_frame frame;

    printf("\nCAN Sockets Receive Demo:\n");
    // Setup CAN socket
    if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("Socket Failed");
        return 1;
    }

    // Copy ifr name to match 
    strcpy(ifr.ifr_name, "vcan0");

    // Get the interface ID
    ioctl(s, SIOCGIFINDEX, &ifr);

    // Zero out struct mem for saftey
    memset(&addr, 0 , sizeof(addr));

    // Config addr
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the CAN interface
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		return 1;
	}

    // Main loop
    while (1) {
        // Clear the set ahead of time
        FD_ZERO(&rfds);
        // Add your descriptors to the set
        FD_SET(s, &rfds); // CAN socket
        FD_SET(STDIN_FILENO, &rfds); // STDIN, usually file descriptor 0

        // Set timeout val 5 sec
        tv.tv_sec = 0;
        tv.tv_usec = 5000000;

        // Use the larger of STDIN_FILENO or s plus 1
        int maxfdp1 = s > STDIN_FILENO ? s : STDIN_FILENO;
        maxfdp1 += 1;

        // Don't care about writefds and exceptfds:
        retval = select(maxfdp1, &rfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");
            return 1;
        } else if (retval) {
            // Check if there was an input on stdin (keyboard input)
            if (FD_ISSET(STDIN_FILENO, &rfds)) {
                // A key was pressed
                break;
            }

            // Check if there is a CAN message to read
            if (FD_ISSET(s, &rfds)) {
                //printf("Reading");
                nbytes = read(s, &frame, sizeof(struct can_frame));

                if (nbytes < 0 ){
                    perror("READ FAIL");
                    return 1;
                }

                // Print CAN id and length of payload, 03X - hexa should be zero padded to a width of at least 3 digits
                printf("vcan0  0x%03X  [%d]  ", frame.can_id, frame.can_dlc);

                // Print the payload
                for (i = 0; i < frame.can_dlc; i++){
                    printf("%02X ", frame.data[i]);
                }
                printf("\n");
                
            }
        } else {
            printf("No data within five seconds\n");
        }

        // Handle delay if you want
    }

    // Close the socket and clean up
    if (close(s) < 0 ){
        perror("Close");
        return 1;
    }
    printf("KEYBOARD EXIT\n");
    return 0;
}