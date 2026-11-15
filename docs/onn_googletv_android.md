# OpenCE Onn GoogleTV: the 32-bit app

What this fork adds to the main project's Android port (`port/android/README.md`,
which stays as the main project has it): the 32-bit app (armeabi-v7a) for
Android TV boxes whose Android runs only 32-bit apps, such as the onn 4K Pro
and onn 4K Plus. The main project's documents describe everything else, and
their settings tables give each setting's meaning; this one gives what is
different here. What to do when the workflow asks is in
[onn_googletv_runbook.md](onn_googletv_runbook.md), the work's history in
[onn_googletv_handoff.md](onn_googletv_handoff.md).

## Building it

`python configure.py --android-abis armeabi-v7a` builds the 32-bit app
(`--android-abis arm64-v8a,armeabi-v7a`: one app with both). It needs an
ARMv7-A processor with NEON, and runs ARMv8 code on an ARMv8 one. The guest
is then 32-bit ARM code in a 32-bit app, with the same AArch64 system call
interface, which the host translates (`port/android/host/host_syscall.c`).
`--android-guest-lto` optimises the guest at the link (below).

Its defines, beside the main project's (`AGENTS.md`, "Defines"):

| Define | Set by | Meaning |
| --- | --- | --- |
| `HALO_ANDROID_GUEST` | Android (both ABIs) | The guest: its C library, its SDL bridge, its memory window |
| `HALO_ARM64_GUEST` | Android, arm64-v8a | The guest's ABI (ILP32 AArch64, compiled through Mach-O) |
| `HALO_ARM32_GUEST` | Android, armeabi-v7a | The guest's ABI (32-bit ARM, ELF) |

Its own test: `python -m pytest -q tools/test_host_alignment.py`, the fixups
of the guest's unaligned accesses (the workflow runs it in every build).

## Settings that differ in the 32-bit app

The main project's settings tables (`port/linux/README.md`,
`port/android/README.md`) describe each setting; in the 32-bit app these
have other defaults, or are its own:

| Setting | Default here | Why |
| --- | --- | --- |
| `display.vsync`, `display.max_fps` | `false` and `30` (elsewhere `true` and `0`) | The TV boxes draw a network game at 30 to 55 frames a second, and a steady 30 (the game's tick rate), paced by `max_fps`, plays better than the display's rate with dips. Android composites every frame, so vsync off does not tear. |
| `display.object_shadows` | `false` (elsewhere `true`) | The objects' shadows on the ground. Each one takes several changes of render target, which on a tiled GPU (a Mali-G310) cost about 130 us of the CPU each, some 7 ms of a frame with a game's players in view. |
| `display.shader_cache` | `true` (`false` in the 64-bit app) | The GPU programs the game links are kept in `shader_cache/` in the save root and loaded from there the next time, instead of linked again (50-90 ms each on a Mali-G310: a stalled frame the first time an effect is drawn in a run). The file is the driver's own: a system update starts another. `debug.gpu_stats` logs how many came from it. |
| `audio.hrtf` | `false` (elsewhere `true`) | The Xbox's directional filters for 3D sounds are about 40% of the mixer's work on a Cortex-A55; with many sounds playing the mixer then takes most of a core, and the game waits for it. 3D sounds are still panned and attenuated. `audio.resampler` should be `xbox` (the default) or `linear`: `sinc`, the default of older versions that an old `config.toml` keeps, is a quarter of the mixer's work there. |
| `input.quick_turn_rate` | `400.0` (elsewhere `0.0`, off) | Degrees a second the view turns while a gamepad's turn button (a back paddle) is held: what the look stick held over does at a high sensitivity, so that its own sensitivity can stay low for aiming. 720 at most; `0` turns the buttons off. The stun and the aim's magnetism slow it as they slow the stick's; it is the player's own input, as a mouse's turn is. |
| `input.quick_turn_left`, `input.quick_turn_right` | `""` | The buttons that turn, separated by commas: `paddle1` to `paddle4` (SDL's: the right and left upper paddles, then the lower), `misc1`, or `buttonN`, the gamepad's Nth button as its driver numbers them. Empty: SDL's paddles, and in the Android app Android's BUTTON_Z (left, `button18`) and BUTTON_C (right, `button17`), which the 8BitDo Ultimate 2 sends for its paddles over Bluetooth. With the turn on, the log names each unnamed button the first time it is pressed (`gamepad: button17 pressed`). |
| `debug.gl_profile` | `false` | `true`: every 120 frames the log has the OpenGL ES functions that took the most time in the driver, with their calls and time a frame, and the app's own GL helpers and the buffer swap. For finding where a frame's CPU time goes, which a profiler cannot name in drivers without symbols. |
| `debug.gl_refresh_state` | `false` | `true`: every OpenGL value is set again for each draw, whatever the renderer remembers having set. For finding whether a driver and the renderer disagree on what is set (black or missing surfaces). |
| `debug.sample_seconds` | `0` | As in the main project (a hang's sampler), and a fraction of a second (`0.02`, a hundredth at least) makes it a profile with each sample's callers, which simpleperf cannot give of the 32-bit guest: record `adb logcat -s halo` to a file while playing, and count the functions of the `sample tid` lines' addresses (the first is where the thread was, the rest its callers, innermost first; "same N" repeats a thread's last sample). |

Android properties for finding problems (`adb shell setprop ...`, then start
the game; `""` goes back):

- `debug.halo.render_chip` `0` (unknown: both compatibility paths), `1`
  (Snapdragon), `2` (MediaTek) or `3` (other): another chip family's render
  policy. The log says `render chip: family <n> (debug.halo.render_chip)`.
- `debug.halo.guest_image` `armv7` or `armv8`: that game image, whatever the
  processor (below).

The window surface is opaque in the Android app (no alpha): a Broadcom
set-top box composes its screen from a surface's alpha, and showed the
game's scratch values in destination alpha as a black screen. The log says
`window: N bits of alpha`; where a driver gives alpha all the same, it is
set to 1 before each swap.

## How the 32-bit app works

The 64-bit app runs the game as ILP32 AArch64 code. A 32-bit Android cannot
run AArch64 code at all, so the 32-bit app (armeabi-v7a) runs the game as
32-bit ARM code (ARMv7-A with NEON, in ARM state, Android's softfp calling
convention), in an ordinary 32-bit app:

- the guest image is `halo_guest_armv7.elf`, compiled for ELF directly
  (`--target=armv7a-unknown-linux-musleabi`), without the Mach-O
  conversion; it is linked at the same address, 0x88000000, after the same
  Xbox window at 0x80000000;
- its C library is the same musl, built against the same system call
  interface (`guest/libc/arch/arm64_32`: AArch64's numbers and structures),
  which the host translates to 32-bit ARM's calls through bionic
  (`host/host_syscall.c`, `host/host_guest_syscalls.h`);
- its import stubs (`tools/android_imports.py --arch arm`) load the host's
  function from the same 8-byte table entries, and enter it with `ldr pc`,
  which goes to Thumb or ARM code as the address says;
- `HALO_ARM32_GUEST` and `HALO_ANDROID_GUEST` are defined for it, not
  `HALO_ARM64_GUEST`.

The 32-bit app has a second image of the game, `halo_guest_armv8.elf`
(`build/android/armv8`): the same sources compiled for ARMv8's 32-bit
state (`-march=armv8-a -mfpu=neon-fp-armv8 -mtune=cortex-a55`). Most 32-bit
Android TV boxes have a 64-bit processor under a 32-bit Android (the Onn 4K
boxes' Cortex-A55, for example), and there the compiler's code divides with
the processor's instruction and rounds with VRINT, where ARMv7 code calls a
routine for each (some 900 places in the game). The host starts the ARMv8
image where the processor says it is ARMv8 (the kernel's platform name
`v8l`, or ARMv8's CRC32 instructions in its capabilities; never under an
ARM translator), and the ARMv7 image everywhere else, so the app runs on
every armeabi-v7a device with NEON. Both compute the same results: every
instruction the ARMv8 image gains is exact, and neither fuses a multiply
and an add. The log (tag `halo`) names the processor and the image:
`guest image: halo_guest_armv8.elf`; addresses in a crash report are
symbolized against the build's file of that name.
`adb shell setprop debug.halo.guest_image armv7` (or `armv8`) starts that
image whatever the processor, for comparing the two; `""` goes back to the
processor's.

Both images take `memcpy` and `memmove` from the guest runtime
(`guest/runtime/guest_string.c`: sixteen bytes at a time through NEON
registers, at any alignment) in place of musl's generic C ones, and the
ARMv7 image rounds real numbers to integers in line
(`halo_linux_prefix.h`, "rounding on ARMv7") in place of a call to `rint`.

In a 32-bit process the game, ART, the graphics driver and the app share
the 4 GB (or 3 GB) of address space, so the fixed ranges at 0x80000000 are
more often taken by the time the game loads. The app then says so and
writes `memory_map.txt` (refer to "Game data" in `port/android/README.md`).

The fork that maintains the 32-bit app builds and publishes it with
`.github/workflows/onn-googletv.yml`: each push to its `armeabi-v7a` branch is
built with the clang of the profiles in `pgo/` (so that they optimise it),
signed in a job of its own, which runs none of the repository's code, with
the `OPENCE_ONN_GOOGLETV_KEYSTORE_BASE64` and
`OPENCE_ONN_GOOGLETV_KEYSTORE_PASSWORD` secrets' key, and published as a release `build-<n>` with `opence-onn-googletv-release.zip`
and `opence-onn-googletv-debug.zip` (`HALO_RELEASE_ASSET_PREFIX`), which the app's self-updater of those builds
installs (each its own kind); every six hours the main
project's newest release is merged in, to keep the network version the same
as the main project's, and built. CI_ANDROID_ABIS and CI_ANDROID_GUEST_CC
give `tools/ci_build.py` the ABIs and the guest's compiler.

`python configure.py --android-guest-lto` optimises the 32-bit guest at the
link (`CI_ANDROID_GUEST_LTO` for `tools/ci_build.py`'s release builds, which
the fork's workflow sets): the game, the platform layer, the maths and the
vendored libraries are compiled to LLVM bitcode and optimised as one module,
as the Linux and Windows builds are, so that the game's small functions are
inlined across its files. The guest's C library and runtime stay ordinary
objects. The link is done by the guest compiler's own `ld.lld` (`clang
-print-prog-name=ld.lld`), which reads its bitcode, and fails if the image
has a thread-local segment (the guest's thread-local storage is emulated).
The 64-bit guest, whose objects are converted from Mach-O assembly, is built
without.

32-bit ARM, unlike AArch64, stops a program at some unaligned accesses
(floating-point and double-word loads and stores of addresses that are not
a multiple of 4). Such an access is a SIGBUS in logcat (tag `halo`), with
the guest's address: `llvm-addr2line -e build/android/armv7/halo_guest_armv7.elf <address>`
(or `build/android/armv8/halo_guest_armv8.elf`, as the log's `guest image:` line says)
names the line.
