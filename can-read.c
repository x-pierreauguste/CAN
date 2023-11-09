/*
    This function is intended to read and listen to an 
    open virtual CAN bus and print the data we are interested in.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>     // sockets local interfaces
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>


int main (int argc, char **argv){

    // Initialize variables we will use
    int s;  // socket file descriptor
    int i; 
    int nbytes;
    struct ifreq ifr;   // struct used for ioctl about an interface
    struct sockaddr_can addr;
    struct can_frame frame;


    if((s = socket(PF_CAN,SOCK_RAW, CAN_RAW) ) < 0){
        perror("Socket Failed");
        return 1;
    }

    // Copy ifr name to match inteface
    strcpy(ifr.ifr_name, "vcan0");
    // Get the interface index of the socket
    ioctl(s, SIOCGIFINDEX, &ifr);

    // Zero out the structure mem for safety
    memset(&addr, 0 , sizeof(addr));

    // Config addr
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Now bind the socket to a local address
    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("BINDING");
        return 1;
    }

    // Now that we have binded the socket to a local address, we can start reading

    // We want to extract the frame.id, frame.can_dlc and frame.data[i]
    nbytes = read(s, &frame, sizeof(struct can_frame));

    if (nbytes < 0 ){
        perror("READ FAIL");
        return 1;
    }

    // Print CAN id and length of payload, 03X - hexa should be zero padded to a width of at least 3 digits
    printf("0x%03X | [%d] | ", frame.can_id, frame.can_dlc);

    // Print the payload
    for (i = 0; i < frame.can_dlc; i++){
        printf("%02X ", frame.data[i]);
    }

    printf("\r\n\n");

    // close the socket
    if (close(s) < 0 ){
        perror("Close");
        return 1;
    }
    return 0;

}



