/*
    Written by Sandy Maguire (Paamayim), 2010. Almost entirely based on wiiuse and wm2js.

    If the uinput buttons sent by this program seem odd (which they might - I don't have a good enough understanding 
    of uinput), the reason they are as they are is to (as closely as possible) emulate the default buttons used by
    mupen64plus when recognizing the joypad.

    Most of this has been hacked together. Seems to work better than the original wm2js though!
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uinput.h"
#include "wiiuse.h"
#include "wm7js.h"

#define MAX_WIIMOTES				7
#define PI                          3.14159265
//#define DPAD_TO_ANALOG_STICK      uncomment this line to use the dpad instead of the ljs as the main analog stick


// performs button press logic
#define BUTTON_PRESS(a, b)  if (IS_JUST_PRESSED(cc, CLASSIC_CTRL_BUTTON_##a)) { do_uinput(fd, BTN_##b, 1, EV_KEY); } \
                            else if (IS_RELEASED(cc, CLASSIC_CTRL_BUTTON_##a) ) { do_uinput(fd, BTN_##b, 0, EV_KEY); }

// emulates a joystick from button presses - originally to map dpad to analog stick
#define BUTTON_TO_STICK(a, b, c, downd) if (IS_JUST_PRESSED(cc, CLASSIC_CTRL_BUTTON_##a) && is##downd##d == 0) { \
                                        is##downd##d = 1; do_uinput(fd, ABS_##b, c * (0xFFFF), EV_ABS); } else \
                                        if (IS_RELEASED(cc, CLASSIC_CTRL_BUTTON_##a)) { is##downd##d = 0; } 

// maps the RJS to c buttons
#define RJS_TO_CSTICK(a, b, c)  if ((cc->rjs.ang >= (a) && cc->rjs.ang <= (b)) && cc->rjs.mag >= 0.4f && !isnan(cc->rjs.ang) \
                                && iscdead == 1)  { iscdead = 0; cdead = 1; do_uinput(fd, BTN_##c, 1, EV_KEY); \
                                usleep(20000); do_uinput(fd, BTN_##c, 0, EV_KEY); }



int *uinput;
wiimote** wiimotes;


#ifdef DPAD_TO_ANALOG_STICK
int isld = 0;
int isrd = 0;
int isud = 0;
int isdd = 0;
#endif

int isdead = 1;
int iscdead = 1;


int iszdead = 1;

void handle_event(int device) {
    struct wiimote_t* wm = wiimotes[device];
    int fd = uinput[device];

    if (wm->exp.type == EXP_CLASSIC) {
		/* classic controller */
		struct classic_ctrl_t* cc = (classic_ctrl_t*)&wm->exp.classic;

        // regular buttons
        BUTTON_PRESS(     A, TRIGGER);
        BUTTON_PRESS(     B, TOP);
        BUTTON_PRESS(  PLUS, BASE3);
        BUTTON_PRESS(FULL_R, PINKIE);

        // 
        if (IS_JUST_PRESSED(cc, CLASSIC_CTRL_BUTTON_ZL) || IS_JUST_PRESSED(cc, CLASSIC_CTRL_BUTTON_ZR)) { 
            iszdead = 0;
            do_uinput(fd, BTN_BASE, 1, EV_KEY); 
        } else if (!IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_ZL) && !IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_ZR) && iszdead == 0) { 
            iszdead = 1;
            do_uinput(fd, BTN_BASE, 0, EV_KEY); 
        }
        

        // map rjs stick to c buttons too
        int cdead = 0;

        // up requires a little different logic
        if ((cc->rjs.ang >= 360 - 23 || cc->rjs.ang <= 0 + 23) && cc->rjs.mag >= 0.4f && !isnan(cc->rjs.ang) && iscdead == 1)  { 
            iscdead = 0; 
            cdead = 1;
            do_uinput(fd, BTN_BASE2, 1, EV_KEY);
            usleep(20000); 
            do_uinput(fd, BTN_BASE2, 0, EV_KEY); 
        }
        RJS_TO_CSTICK( 90 - 23,  90 + 23, BASE4);
        RJS_TO_CSTICK(180 - 23, 180 + 23, THUMB);
        RJS_TO_CSTICK(270 - 23, 270 + 23, TOP2);

        if (cdead == 0) iscdead = 1;

#ifndef DPAD_TO_ANALOG_STICK
        // c buttons mapped to dpad
        BUTTON_PRESS(UP, BASE2);
        BUTTON_PRESS(RIGHT, BASE4);
        BUTTON_PRESS(DOWN, THUMB);
        BUTTON_PRESS(LEFT, TOP2);

        // ljs to analog stick
        if (cc->ljs.mag >= 0.04) {
            isdead = 0;
            double rad = cc->ljs.ang * PI / 180.0;
            int y = (int)(-cos(rad) * (double)0xFFFF * cc->ljs.mag);
            int x = (int)(sin(rad) * (double)0xFFFF * cc->ljs.mag);
            do_uinput(fd, ABS_X, x, EV_ABS);
            do_uinput(fd, ABS_Y, y, EV_ABS);
        } else if (isdead == 0) {
            isdead = 1;
            do_uinput(fd, ABS_X, 0, EV_ABS);
            do_uinput(fd, ABS_Y, 0, EV_ABS);
        }
#else
        // dpad to analog stick
        BUTTON_TO_STICK(LEFT, X, -1, l);
        BUTTON_TO_STICK(RIGHT, X, 1, r);
        BUTTON_TO_STICK(UP, Y, -1, u);
        BUTTON_TO_STICK(DOWN, Y, 1, d);

        if (isld == 0 && isrd == 0) { do_uinput(fd, ABS_X, 0, EV_ABS); }
        if (isud == 0 && isdd == 0) { do_uinput(fd, ABS_Y, 0, EV_ABS); }
#endif
	}
}

// just echos to the command line to keep you in the loop
void handle_ctrl_status(struct wiimote_t* wm) {
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);

	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}

int main(int argc, char** argv) {
	int found, connected, i;

	wiimotes =  wiiuse_init(MAX_WIIMOTES);

    // wiiuse has an ugly splash
    system("clear");
    // but Micahel Laforest still deserves our love
    printf("wm7js - Wiimote Classic Controller to Joypad\n");
    printf("Initialized Michael Laforest's wiiuse <thepara[at]gmail{dot}com>\n\n");
    printf("Press 1 + 2 on your Wiimotes now.\n");

	found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
	if (!found) {
		printf ("No wiimotes found.");
		return 1;
	}

	connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
	if (connected)
		printf("Connected to %i wiimotes (of %i found).\n", connected, found);
	else {
		printf("Failed to connect to any wiimote.\n");
		return 2;
	}

    for (i = 0; i < connected; i++) {
        switch (i) {
            case 0: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_1); break;
            case 1: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_2); break;
            case 2: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_3); break;
            case 3: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_4); break;
            case 4: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_1 | WIIMOTE_LED_2); break;
            case 5: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_2 | WIIMOTE_LED_3); break;
            case 6: wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_3 | WIIMOTE_LED_4); break;
        }

        wiiuse_rumble(wiimotes[i], 1);
    }

    usleep(200000);

    for (i = 0; i < connected; i++)
        wiiuse_rumble(wiimotes[i], 0);

    if(init_uinput_device(&uinput, connected) != 0)
		xxxdie("Registering the uinput device failed, aborting!");

	while (1) {
		if (wiiuse_poll(wiimotes, MAX_WIIMOTES)) {
			i = 0;
			for (; i < MAX_WIIMOTES; ++i) {
				switch (wiimotes[i]->event) {
					case WIIUSE_EVENT:
						/* a generic event occured */
						handle_event(i);
						break;

					case WIIUSE_STATUS:
						/* a status event occured */
						handle_ctrl_status(wiimotes[i]);
						break;

					case WIIUSE_DISCONNECT:
					case WIIUSE_UNEXPECTED_DISCONNECT:
						/* the wiimote disconnected */
						printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", i);
						break;

					case WIIUSE_READ_DATA:
						break;

					case WIIUSE_NUNCHUK_INSERTED:
						printf("Nunchuk is unsupported by wm7js. Use wm2js instead.\n");
						break;

					case WIIUSE_CLASSIC_CTRL_INSERTED:
						printf("Classic controller inserted.\n");
						break;

					case WIIUSE_GUITAR_HERO_3_CTRL_INSERTED:
						handle_ctrl_status(wiimotes[i]);
						printf("Guitar Hero 3 controller is unsupported.\n");
						break;

					case WIIUSE_NUNCHUK_REMOVED:
					case WIIUSE_CLASSIC_CTRL_REMOVED:
					case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
						/* some expansion was removed */
						handle_ctrl_status(wiimotes[i]);
						printf("An expansion was removed.\n");
						break;

					default:
						break;
				}
			}
		}
	}

	wiiuse_cleanup(wiimotes, MAX_WIIMOTES);

	return 0;
}

int xxxdie(char *msg) {
	printf("Error: %s\n",msg);
	exit(1);
}

int xxxdie_i(char *msg, int i) {
	printf("Error: %s 0x%x\n",msg,i);
	exit(1);
}

int xxxwarn(char *msg) {
	printf("Warning: %s\n",msg);
	return TRUE;
}


