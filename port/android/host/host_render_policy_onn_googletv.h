/*
HOST_RENDER_POLICY_ONN_GOOGLETV.H

OpenCE Onn GoogleTV's changes to the render chip's family
(host_android_chip_family, host_render_policy.c, which includes this file):
kept in a file of the fork's own, so that the main project's edits of
host_render_policy.c merge with them, and its test (a copy of the function
without this file) is as the main project's.
*/

#define ONN_GOOGLETV_CHIP_FAMILY

static int onn_googletv_chip_word(const char *value, const char *word)
{
	while (*word)
	{
		int c = (unsigned char)*value++;

		if ((c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c) != *word++)
			return 0;
	}
	return !*value;
}

/* family: android_chip_classify's, of the SoC properties hardware and
platform (ro.hardware, ro.board.platform) */
static int onn_googletv_chip_family(int family, const char *hardware, const char *platform)
{
	/* The Technicolor UIW4059MIL (Broadcom BCM72180, VideoCore VI, Android
	TV 10: GitHub issue 23) names no SoC, only its board. With the unknown
	chip's compatibility paths it plays at some 15 fps; with "other"'s (no
	geometry streamed every draw, no CE texture conversion) it draws
	correctly and plays at 30 where the scene is simple. Only this board:
	other Broadcom boxes are untested */
	if (family == _android_chip_unknown &&
		(onn_googletv_chip_word(hardware, "uiw4059mil") || onn_googletv_chip_word(platform, "uiw4059mil")))
	{
		family = _android_chip_other;
		host_logf(HOST_LOG_INFO, "render chip: family %d (the Technicolor UIW4059MIL, Broadcom)", family);
	}
	/* adb shell setprop debug.halo.render_chip <family>: the policy of
	another family, for finding which a device's rendering needs (0 unknown:
	every compatibility path; 1 Snapdragon; 2 MediaTek; 3 other); ""
	(adb shell setprop debug.halo.render_chip "") goes back to the device's.
	(A debug. property lasts until the box restarts.) */
	{
		char forced[PROP_VALUE_MAX] = { 0 };

		if (__system_property_get("debug.halo.render_chip", forced) > 0 && forced[0] >= '0' && forced[0] <= '3' &&
			!forced[1])
		{
			family = forced[0] - '0';
			host_logf(HOST_LOG_WARN, "render chip: family %d (debug.halo.render_chip)", family);
		}
	}
	return family;
}
