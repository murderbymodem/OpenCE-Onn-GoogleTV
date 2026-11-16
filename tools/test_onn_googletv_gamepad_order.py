"""OpenCE Onn GoogleTV's order of the Android app's gamepads
(port/linux/src/xinput_order_onn_googletv.h), with SDL's gamepads stood in
for: which device each player gets."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[1]


@pytest.fixture(scope="module")
def order(tmp_path_factory):
    compiler = shutil.which("clang")
    if not compiler:
        pytest.skip("clang required")
    work = tmp_path_factory.mktemp("onn-googletv-gamepad-order")
    source = work / "order.c"
    source.write_text(r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HALO_ANDROID_GUEST 1
#define PORT_COUNT 4
typedef unsigned int SDL_JoystickID;
typedef enum { SDL_GAMEPAD_TYPE_UNKNOWN, SDL_GAMEPAD_TYPE_STANDARD, SDL_GAMEPAD_TYPE_XBOXONE } SDL_GamepadType;
typedef enum { SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY } SDL_GamepadAxis;
typedef short Sint16;
/* a device: its name, whether SDL recognises it, whether it has a left stick */
typedef struct SDL_Gamepad { char name[32]; int recognised; int stick; } SDL_Gamepad;
static SDL_Gamepad devices[8];
static int logs;
/* (SDL numbers gamepads from 1: 0 is none) */
static SDL_Gamepad *SDL_GetGamepadFromID(SDL_JoystickID id) { return &devices[id - 1]; }
static SDL_GamepadType SDL_GetGamepadType(SDL_Gamepad *g)
{ return g->recognised ? SDL_GAMEPAD_TYPE_XBOXONE : SDL_GAMEPAD_TYPE_STANDARD; }
/* the app's: from 0x100 on, whether it has the axis; below, the axis (at rest) */
static Sint16 SDL_GetGamepadAxis(SDL_Gamepad *g, SDL_GamepadAxis a) { return (int)a >= 0x100 ? g->stick : 0; }
static void platform_log(const char *format, ...) { (void)format; logs++; }
#include "xinput_order_onn_googletv.h"
/* argv: devices "name:recognised:stick" in Android's order; prints the
players' names, then the log lines written by two calls */
int main(int argc, char **argv)
{
    SDL_JoystickID ids[8];
    SDL_Gamepad *gamepads[PORT_COUNT];
    int count = argc - 1, i, found;
    for (i = 0; i < count; i++) {
        sscanf(argv[i + 1], "%31[^:]:%d:%d", devices[i].name, &devices[i].recognised, &devices[i].stick);
        ids[i] = (SDL_JoystickID)(i + 1);
    }
    found = onn_googletv_gamepads_order(ids, count, gamepads);
    found = onn_googletv_gamepads_order(ids, count, gamepads);
    for (i = 0; i < found; i++) printf("%s,", gamepads[i]->name);
    printf(" %d\n", logs);
    return 0;
}
''')
    executable = work / "order"
    result = subprocess.run([compiler, "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                             "-I", str(ROOT / "port/linux/src"), str(source), "-o", str(executable)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.mark.parametrize("devices,players", [
    # the Technicolor box (issue 23): the controller first, then the rest as Android listed them
    (["NexusIrHandler:0:0", "virtual-search:0:0", "PG-9021S:0:1", "virtual-remote:0:0"],
     "PG-9021S,NexusIrHandler,virtual-search,virtual-remote,"),
    # a recognised controller first, as the main project's order
    (["remote:0:0", "generic:0:1", "8BitDo:1:1"], "8BitDo,generic,remote,"),
    # only a controller (an onn box with one): as it was
    (["8BitDo:1:1"], "8BitDo,"),
    (["generic:0:1"], "generic,"),
    # every device with a stick, or none: Android's order, as it was
    (["a:0:1", "b:0:1", "c:0:1"], "a,b,c,"),
    (["a:0:0", "b:0:0"], "a,b,"),
    # more devices than ports: the controller listed fifth still gets one
    (["r1:0:0", "r2:0:0", "r3:0:0", "r4:0:0", "pad:0:1"], "pad,r1,r2,r3,"),
    # none
    ([], ""),
])
def test_gamepad_order(order, devices, players):
    result = subprocess.run([str(order), *devices], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    names, logs = result.stdout.split()[0] if devices else "", result.stdout.split()[-1]
    assert names == players
    # the players are logged when they change, not on every call (none from
    # the start: no change, no line)
    assert logs == ("1" if devices else "0")
