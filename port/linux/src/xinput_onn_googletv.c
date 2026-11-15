/*
XINPUT_ONN_GOOGLETV.C

OpenCE Onn GoogleTV's turn buttons, in a file of the fork's own so that the
main project's edits of xinput_sdl.c merge with them: xinput_sdl.c gives
only the gamepad of a port (halo_port_gamepad), and player_control.c adds
the turn to the look (halo_linux_quick_turn).
*/

#include "platform.h"
#include "port_config.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SDL_Gamepad *halo_port_gamepad(int port);
extern unsigned char network_chat_keyboard_open(void);
extern short network_chat_controller(void);

/* ---------- the turn buttons (input.quick_turn_rate)

A gamepad's spare buttons (its back paddles) turn the view left or right
while they are held, at input.quick_turn_rate degrees a second: what holding
the look stick over does at a high sensitivity, with the stick's own
sensitivity left low for aiming. player_control.c adds the turn to the
stick's rate, so the stun and the aim's magnetism slow it as they slow the
stick's. It is this machine's input only: other machines see the player
turn, as they do a mouse's.

input.quick_turn_left and input.quick_turn_right name the buttons, separated
by commas: paddle1 to paddle4 (SDL's: the right and left upper paddles, then
the lower), misc1, or buttonN, the gamepad's Nth button as its driver numbers
them, for one SDL has no name for. Empty: the left and right paddles, and in
the Android app buttons 18 (left) and 17 (right) too, which are Android's
BUTTON_Z and BUTTON_C, where the 8BitDo Ultimate 2 puts its left and right
paddles over Bluetooth. With
the turn on, the first press of each of a gamepad's unnamed buttons is
logged with its number, for finding which a paddle is. */

#define QUICK_TURN_RAW_FIRST 15
#define QUICK_TURN_RAW_COUNT 21
#define QUICK_TURN_MAXIMUM_RATE 720.0f

/* a gamepad's button by its driver's number */
static BOOL gamepad_raw_button(SDL_Gamepad *gamepad, int button)
{
#ifdef HALO_ANDROID
	/* (the host reads buttons from 0x100 on from the joystick:
	port/android/host/host_sdl.c) */
	return SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)(0x100 + button));
#else
	SDL_Joystick *joystick = SDL_GetGamepadJoystick(gamepad);

	return joystick && button < SDL_GetNumJoystickButtons(joystick) && SDL_GetJoystickButton(joystick, button);
#endif
}

/* whether one of the buttons a setting names is held */
static BOOL quick_turn_held(SDL_Gamepad *gamepad, const char *names)
{
	static const struct
	{
		const char *name;
		SDL_GamepadButton button;
	} named[] =
	{
		{ "paddle1", SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1 }, { "paddle2", SDL_GAMEPAD_BUTTON_LEFT_PADDLE1 },
		{ "paddle3", SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2 }, { "paddle4", SDL_GAMEPAD_BUTTON_LEFT_PADDLE2 },
		{ "misc1", SDL_GAMEPAD_BUTTON_MISC1 },
	};

	while (*names)
	{
		size_t length = strcspn(names, ", ");
		unsigned long index;

		for (index = 0; index < sizeof(named) / sizeof(named[0]); index++)
		{
			if (length == strlen(named[index].name) && !strncmp(names, named[index].name, length) &&
				SDL_GetGamepadButton(gamepad, named[index].button))
			{
				return TRUE;
			}
		}
		if (length > 6 && length < 10 && !strncmp(names, "button", 6))
		{
			int button = atoi(names + 6);

			if (button >= 0 && button < 64 && gamepad_raw_button(gamepad, button))
				return TRUE;
		}
		names += length;
		names += strspn(names, ", ");
	}
	return FALSE;
}

/* radians the port's player turns in these seconds for the turn buttons
held (to the left positive, as the look's yaw); player_control.c adds it to
the stick's look */
float halo_linux_quick_turn(short gamepad_index, float seconds)
{
	static unsigned long read_at = ~0UL;
	static float rate;
	static char left_names[64], right_names[64];
	static BOOL logged[QUICK_TURN_RAW_COUNT];
	SDL_Gamepad *gamepad;
	int button;
	float turn = 0.0f;

	if (read_at != config_changes())
	{
		const char *left = config_string("input.quick_turn_left"), *right = config_string("input.quick_turn_right");

		read_at = config_changes();
		rate = (float)config_real("input.quick_turn_rate");
		if (!(rate > 0.0f))
			rate = 0.0f;
		if (rate > QUICK_TURN_MAXIMUM_RATE)
			rate = QUICK_TURN_MAXIMUM_RATE;
#ifdef HALO_ANDROID
		snprintf(left_names, sizeof(left_names), "%s", left && *left ? left : "paddle2,paddle4,button18");
		snprintf(right_names, sizeof(right_names), "%s", right && *right ? right : "paddle1,paddle3,button17");
#else
		snprintf(left_names, sizeof(left_names), "%s", left && *left ? left : "paddle2,paddle4");
		snprintf(right_names, sizeof(right_names), "%s", right && *right ? right : "paddle1,paddle3");
#endif
	}
	if (rate <= 0.0f || !(seconds > 0.0f))
		return 0.0f;
	/* (none while the chat's on-screen keyboard is up) */
	if (network_chat_keyboard_open() && network_chat_controller() == gamepad_index)
		return 0.0f;
	gamepad = halo_port_gamepad(gamepad_index);
	if (!gamepad)
		return 0.0f;
	for (button = 0; button < QUICK_TURN_RAW_COUNT; button++)
	{
		if (!logged[button] && gamepad_raw_button(gamepad, QUICK_TURN_RAW_FIRST + button))
		{
			logged[button] = TRUE;
			platform_log("gamepad: button%d pressed (input.quick_turn_left and input.quick_turn_right take \"button%d\")",
				QUICK_TURN_RAW_FIRST + button, QUICK_TURN_RAW_FIRST + button);
		}
	}
	/* (a frame's worth at most a tenth of a second's: a stalled frame does
	not spin the player round) */
	if (seconds > 0.1f)
		seconds = 0.1f;
	if (quick_turn_held(gamepad, left_names))
		turn += rate;
	if (quick_turn_held(gamepad, right_names))
		turn -= rate;
	return turn * (3.14159265f / 180.0f) * seconds;
}
