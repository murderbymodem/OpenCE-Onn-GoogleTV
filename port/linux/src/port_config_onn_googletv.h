/*
PORT_CONFIG_ONN_GOOGLETV.H

OpenCE Onn GoogleTV's settings, in a file of the fork's own so that the main
project's edits of port_config.c merge with them: port_config.c includes it
above its table of settings, takes the defaults below for a few of its own
settings, and ends the table with the fork's settings
(ONN_GOOGLETV_CONFIG_SETTINGS; config.toml has each in its section,
wherever the table lists it). docs/onn_googletv_android.md documents them.
*/

#ifndef PORT_CONFIG_ONN_GOOGLETV_H
#define PORT_CONFIG_ONN_GOOGLETV_H

/* display.object_shadows: as on the Xbox, but off in the 32-bit Android app,
whose devices (TV boxes with a Mali GPU) spend some 7 ms of a frame on the
shadows' changes of render target (port/linux/game/object_shadows.c) */
#ifdef HALO_ARM32_GUEST
#define OBJECT_SHADOWS_DEFAULT "false"
#else
#define OBJECT_SHADOWS_DEFAULT "true"
#endif

/* display.shader_cache: the desktop's, and the 32-bit Android app's, whose
TV boxes take 50-90 ms to link a program; the 64-bit app links each run, as
before (OpenGL ES drivers' binaries are not all to be trusted) */
#if defined(HALO_ANDROID) && !defined(HALO_ARM32_GUEST)
#define SHADER_CACHE_DEFAULT "false"
#else
#define SHADER_CACHE_DEFAULT "true"
#endif

/* audio.hrtf: the Xbox's directional filters, but off in the 32-bit Android
app: on its TV boxes (a Cortex-A55) they are some 40% of the mixer's work,
which with many sounds playing takes most of a core and keeps the game's
thread waiting for the mixer (port/linux/src/dsound_sdl.c, mixer_gate) */
#ifdef HALO_ARM32_GUEST
#define AUDIO_HRTF_DEFAULT "false"
#else
#define AUDIO_HRTF_DEFAULT "true"
#endif

/* input.quick_turn_rate: off, but on in the 32-bit Android app, whose TV
boxes are played with a gamepad whose back paddles nothing else uses */
#ifdef HALO_ARM32_GUEST
#define QUICK_TURN_RATE_DEFAULT "400.0"
#else
#define QUICK_TURN_RATE_DEFAULT "0.0"
#endif

/* display.vsync and display.max_fps: the 32-bit Android app's TV boxes draw
a network game at 30 to 55 frames a second, and a steady 30 (the game's own
tick rate), paced by max_fps with vsync off, plays better there than the
display's refresh with dips; Android composites every frame, so it does not
tear. Elsewhere, vsync. */
#ifdef HALO_ARM32_GUEST
#define VSYNC_DEFAULT "false"
#define MAX_FPS_DEFAULT "30"
#else
#define VSYNC_DEFAULT "true"
#define MAX_FPS_DEFAULT "0"
#endif

/* the fork's own settings, the last of port_config.c's table */
#define ONN_GOOGLETV_CONFIG_SETTINGS \
	{ "display.object_shadows", _config_boolean, OBJECT_SHADOWS_DEFAULT, "HALO_OBJECT_SHADOWS", _environment_value, \
		_platform_all, \
		"The objects' shadows on the ground. false leaves them out, which saves\n" \
		"the most time of a frame on slow GPUs that draw in tiles (phones and\n" \
		"TV boxes): each object's shadow takes several changes of render\n" \
		"target. Taken up at once. Off by default in the 32-bit Android app,\n" \
		"for the TV boxes it runs on." }, \
	{ "input.quick_turn_rate", _config_real, QUICK_TURN_RATE_DEFAULT, "HALO_QUICK_TURN_RATE", _environment_value, _platform_all, \
		"Degrees a second the view turns while a gamepad's turn button (a back\n" \
		"paddle) is held, as the look stick held over at a high sensitivity,\n" \
		"720 at most; 0: the buttons do nothing." }, \
	{ "input.quick_turn_left", _config_string, "\"\"", "HALO_QUICK_TURN_LEFT", _environment_value, _platform_all, \
		"The gamepad buttons that turn left, separated by commas: paddle1 to\n" \
		"paddle4, misc1, or buttonN (the log names a button's number when it is\n" \
		"first pressed). Empty: the left paddles." }, \
	{ "input.quick_turn_right", _config_string, "\"\"", "HALO_QUICK_TURN_RIGHT", _environment_value, _platform_all, \
		"The gamepad buttons that turn right, as input.quick_turn_left. Empty:\n" \
		"the right paddles." }, \
	{ "debug.gl_profile", _config_boolean, "false", "HALO_GL_PROFILE", _environment_set_is_true, _platform_android, \
		"Every 120 frames, the OpenGL ES functions that took the most time in\n" \
		"the driver, with their calls and time a frame, in the log (the 32-bit\n" \
		"ARM app; slower while on)." }, \
	{ "debug.gl_refresh_state", _config_boolean, "false", "HALO_GL_REFRESH_STATE", _environment_set_is_true, \
		_platform_android, \
		"Set every OpenGL value again for each draw, whatever the renderer\n" \
		"remembers having set: slower, for finding whether a driver and the\n" \
		"renderer disagree on what is set (black or missing surfaces)." },

#endif
