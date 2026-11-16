/*
XINPUT_ORDER_ONN_GOOGLETV.H

OpenCE Onn GoogleTV's order of the Android app's gamepads (sdl_gamepads,
xinput_sdl.c, which includes this file): kept in a file of the fork's own,
so that the main project's edits of xinput_sdl.c merge with it.

The main project gives the ports to the gamepads SDL recognises first, then
to the rest in Android's order. Android also lists remote receivers and
virtual key devices as gamepads, which SDL does not recognise, and neither
does it every controller (one reporting Amazon's Fire TV vendor, a generic
Bluetooth one): on a Technicolor box (GitHub issue 23) the IR receiver came
first, so player 1 had it, and the controller (player 3) moved nothing once
the game began. Of the gamepads SDL does not recognise, those with a left
stick now come before those without; recognised ones come first as before,
and the order among each kind is Android's.
*/

#ifdef HALO_ANDROID_GUEST

#define ONN_GOOGLETV_GAMEPAD_ORDER

/* SDL_GetGamepadAxis of ONN_GOOGLETV_HAS_AXIS + an axis: whether the
gamepad has that axis (the app's host_sdl_gamepad_axis: the guest's SDL has
no SDL_GamepadHasAxis) */
#define ONN_GOOGLETV_HAS_AXIS 0x100

/* 0: recognised by SDL; 1: not, with a left stick; 2: neither */
static int onn_googletv_gamepad_rank(SDL_Gamepad *gamepad)
{
	SDL_GamepadType type = SDL_GetGamepadType(gamepad);

	if (type != SDL_GAMEPAD_TYPE_UNKNOWN && type != SDL_GAMEPAD_TYPE_STANDARD)
		return 0;
	return SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)(ONN_GOOGLETV_HAS_AXIS + SDL_GAMEPAD_AXIS_LEFTX)) &&
		SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)(ONN_GOOGLETV_HAS_AXIS + SDL_GAMEPAD_AXIS_LEFTY)) ? 1 : 2;
}

/* the gamepads of ids (count), in that order, at most PORT_COUNT: found */
static int onn_googletv_gamepads_order(const SDL_JoystickID *ids, int count, SDL_Gamepad *gamepads[PORT_COUNT])
{
	static SDL_JoystickID logged[PORT_COUNT];
	SDL_JoystickID players[PORT_COUNT];
	int rank, index, found = 0;

	memset(gamepads, 0, sizeof(SDL_Gamepad *) * PORT_COUNT);
	memset(players, 0, sizeof(players));
	for (rank = 0; rank < 3; rank++)
	{
		for (index = 0; index < count && found < PORT_COUNT; index++)
		{
			SDL_Gamepad *gamepad = SDL_GetGamepadFromID(ids[index]);

			if (gamepad && onn_googletv_gamepad_rank(gamepad) == rank)
			{
				players[found] = ids[index];
				gamepads[found++] = gamepad;
			}
		}
	}
	/* the log names the players' gamepads (the app's "gamepad <n>: <name>"
	lines) when they change */
	if (memcmp(logged, players, sizeof(logged)))
	{
		char text[128];
		size_t length = 0;

		memcpy(logged, players, sizeof(logged));
		text[0] = 0;
		for (index = 0; index < found && length < sizeof(text); index++)
		{
			int written = snprintf(text + length, sizeof(text) - length, "%splayer %d: gamepad %u",
				index ? ", " : "", index + 1, (unsigned)players[index]);

			if (written > 0)
				length += (size_t)written;
		}
		platform_log("gamepads: %s", found ? text : "none");
	}
	return found;
}

#endif
