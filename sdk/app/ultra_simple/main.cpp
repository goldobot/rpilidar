/*
 *  RPLIDAR
 *  Ultra Simple Data Grabber Demo App
 *
 *  Copyright (c) 2009 - 2014 RoboPeak Team
 *  http://www.robopeak.com
 *  Copyright (c) 2014 - 2016 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if 1 /* FIXME : DEBUG : HACK GOLDO ++ */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif /* FIXME : DEBUG : HACK GOLDO -- */

#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x)
#else
#include <unistd.h>
static inline void delay(_word_size_t ms){
    while (ms>=1000){
        usleep(1000*1000);
        ms-=1000;
    };
    if (ms!=0)
        usleep(ms*1000);
}
#endif


#if 1 /* FIXME : DEBUG : HACK GOLDO ++ */
float my_x[360];
float my_y[360];
char send_buf[360*4*2];

typedef struct _my_point_t {
  float x;
  float y;
} my_point_t;

int my_sock;
struct sockaddr_in viewer_saddr;

int init_sock(char *viewer_address_str)
{
  unsigned int ab0, ab1, ab2, ab3;

  if (sscanf (viewer_address_str, "%d.%d.%d.%d", 
	      &ab3, &ab2, &ab1, &ab0) != 4) {
    printf(" error : cannot parse viewer addr (%s)\n", viewer_address_str);
    return -1;
  }

  my_sock = socket (AF_INET, SOCK_DGRAM, 0);

  viewer_saddr.sin_family= AF_INET;
  viewer_saddr.sin_port= htons(1412);
  viewer_saddr.sin_addr.s_addr= htonl((ab3<<24)|(ab2<<16)|(ab1<<8)|(ab0));

  return 0;
}

int send_to_viewer()
{
  my_point_t *my_p;

  my_p = (my_point_t *)((void *)(&send_buf[0]));

  for (int i=0; i<360; i++) {
    my_p->x = my_x[i];
    my_p->y = my_y[i];
    my_p++;
  }

  int ret = sendto (my_sock, (void *)send_buf, sizeof (send_buf), 0, 
		    (const sockaddr *) &viewer_saddr, 
		    sizeof (struct sockaddr_in));
  if (ret!=sizeof(send_buf)) {
    printf ("sendto() failed\n");
    return -1;
  }

  return 0;
}
#endif /* FIXME : DEBUG : HACK GOLDO -- */


using namespace rp::standalone::rplidar;

bool checkRPLIDARHealth(RPlidarDriver * drv)
{
    u_result     op_result;
    rplidar_response_device_health_t healthinfo;


    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
#if 0 /* FIXME : DEBUG : HACK GOLDO ++ */
        printf("RPLidar health status : %d\n", healthinfo.status);
#endif /* FIXME : DEBUG : HACK GOLDO -- */
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return false;
        } else {
            return true;
        }

    } else {
        fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
        return false;
    }
}

#include <signal.h>
bool ctrl_c_pressed;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}

int main(int argc, const char * argv[]) {
    const char * opt_com_path = NULL;
    _u32         opt_com_baudrate = 115200;
    u_result     op_result;

    /* FIXME : DEBUG : HACK GOLDO ++ */
    char viewer_addr_str[40];
    //int kill_switch = 0;
    /* FIXME : DEBUG : HACK GOLDO -- */

#if 0 /* FIXME : DEBUG : HACK GOLDO ++ */
    printf("Ultra simple LIDAR data grabber for RPLIDAR.\n"
           "Version: "RPLIDAR_SDK_VERSION"\n");
#endif /* FIXME : DEBUG : HACK GOLDO -- */

#if 1 /* FIXME : DEBUG : HACK GOLDO ++ */
    // read serial port from the command line...
    if (argc>1) strncpy(viewer_addr_str,argv[1],40);
    else strncpy(viewer_addr_str,"127.0.0.1",40);

    // read serial port from the command line...
    if (argc>2) opt_com_path = argv[2]; // or set to a fixed value: e.g. "com3" 

    // read baud rate from the command line if specified...
    if (argc>3) opt_com_baudrate = strtoul(argv[3], NULL, 10);

    if (init_sock(viewer_addr_str)<0)
      return -1;
#else
    // read serial port from the command line...
    if (argc>1) opt_com_path = argv[1]; // or set to a fixed value: e.g. "com3" 

    // read baud rate from the command line if specified...
    if (argc>2) opt_com_baudrate = strtoul(argv[2], NULL, 10);
#endif /* FIXME : DEBUG : HACK GOLDO -- */



    if (!opt_com_path) {
#ifdef _WIN32
        // use default com port
        opt_com_path = "\\\\.\\com3";
#else
        opt_com_path = "/dev/ttyUSB0";
#endif
    }

    // create the driver instance
    RPlidarDriver * drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
    
    if (!drv) {
        fprintf(stderr, "insufficent memory, exit\n");
        exit(-2);
    }


    // make connection...
    if (IS_FAIL(drv->connect(opt_com_path, opt_com_baudrate))) {
        fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
            , opt_com_path);
        goto on_finished;
    }

    rplidar_response_device_info_t devinfo;

	// retrieving the device info
    ////////////////////////////////////////
    op_result = drv->getDeviceInfo(devinfo);

    if (IS_FAIL(op_result)) {
        fprintf(stderr, "Error, cannot get device info.\n");
        goto on_finished;
    }

    // print out the device serial number, firmware and hardware version number..
#if 0 /* FIXME : DEBUG : HACK GOLDO ++ */
    printf("RPLIDAR S/N: ");
    for (int pos = 0; pos < 16 ;++pos) {
        printf("%02X", devinfo.serialnum[pos]);
    }

    printf("\n"
            "Firmware Ver: %d.%02d\n"
            "Hardware Rev: %d\n"
            , devinfo.firmware_version>>8
            , devinfo.firmware_version & 0xFF
            , (int)devinfo.hardware_version);
#endif /* FIXME : DEBUG : HACK GOLDO -- */



    // check health...
    if (!checkRPLIDARHealth(drv)) {
        goto on_finished;
    }

	signal(SIGINT, ctrlc);
    
	drv->startMotor();
    // start scan...
    drv->startScan();

    // fetech result and print it out...
    while (1) {
        rplidar_response_measurement_node_t nodes[360*2];
        size_t   count = _countof(nodes);

        op_result = drv->grabScanData(nodes, count);

        if (IS_OK(op_result)) {
            drv->ascendScanData(nodes, count);
#if 0 /* FIXME : DEBUG : HACK GOLDO ++ */
            for (int pos = 0; pos < (int)count ; ++pos) {
                printf("%s theta: %03.2f Dist: %08.2f Q: %d \n", 
                    (nodes[pos].sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?"S ":"  ", 
                    (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f,
                    nodes[pos].distance_q2/4.0f,
                    nodes[pos].sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
            }
#endif /* FIXME : DEBUG : HACK GOLDO -- */
#if 0 /* FIXME : DEBUG : HACK GOLDO ++ */
	    if ((kill_switch>15) && (kill_switch<25)) {
	      for (int pos = 0; pos < (int)count ; ++pos) {
		double my_theta = ((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f)*(2.0f*M_PI/360.0f);
		double my_R = nodes[pos].distance_q2/4.0f;
		double my_x = my_R * cos (my_theta);
		double my_y = my_R * sin (my_theta);
		if ((my_x>-500.0) && (my_x<500.0) && 
		    (my_y>-500.0) && (my_y<500.0)) {
		  printf("%f %f\n", my_x, my_y);
		}
	      }
	    }
#endif /* FIXME : DEBUG : HACK GOLDO -- */


#if 1 /* FIXME : DEBUG : HACK GOLDO ++ */
	    for (int pos = 0; pos < 360 ; ++pos) {
	      my_x[pos] = 0.0;
	      my_y[pos] = 0.0;
            }

	    for (int pos = 0; pos < (int)count ; ++pos) {
              double my_theta = ((nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f)*(2.0f*M_PI/360.0f);
	      double my_R = nodes[pos].distance_q2/4.0f;
	      my_x[pos] = my_R * cos (my_theta);
	      my_y[pos] = my_R * sin (my_theta);
            }

	    send_to_viewer();
#endif /* FIXME : DEBUG : HACK GOLDO -- */


        }

#if 0 /* FIXME : DEBUG : HACK GOLDO ++ */
	kill_switch++;
	if (kill_switch>40) break;
#endif /* FIXME : DEBUG : HACK GOLDO -- */

        if (ctrl_c_pressed){ 
	  break;
	}
    }

    drv->stop();
    drv->stopMotor();
    // done!
on_finished:
    RPlidarDriver::DisposeDriver(drv);
    return 0;
}

