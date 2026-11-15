# OpenCE - Halo: Combat Evolved for Walmart Onn GoogleTV boxes

<p align="center">
  <img src="../port/assets/icon/opence-icon.png" alt="OpenCE" width="128">
</p>


<p align="center">
  <a href="https://github.com/murderbymodem/OpenCE-Onn-GoogleTV/releases/latest"><img src="https://img.shields.io/github/v/release/murderbymodem/OpenCE-Onn-GoogleTV?label=Download%20Onn%20GoogleTV&display_name=release&style=for-the-badge" alt="Download for Onn GoogleTV"></a>
</p>

**OpenCE** (Open Community Edition) is an open source port of the Halo: Combat Evolved decompilation (Halo CE) based on an early Xbox development build (2342) and forked from
[bnunu/halo-1](https://github.com/bnunu/halo-1). The project aims to enhance the existing game and adds many features, such as 128-player multiplayer, online campaign co-op, mod packs, field-of-view settings, a server browser, peer-to-peer hosting, automatic map downloads, voice chat, support for Custom Edition maps and much more! 

The game runs natively on Windows, Linux, Android **(now including 32-bit armeabi-v7a, this fork)** and in the web browser with nothing to install. A macOS port is in progress. Worth noting that it is not an emulator, **the game's own code is compiled for each platform** with a new layer in place of the Xbox's hardware. Runs on OpenGL (Android is on OpenGL ES and WebGL 2 in the browser). Sound, input, gamepads are all handled through SDL3. The game currently draws a frame every refresh of your display with Smooth Motion enabled, while the simulation runs at 30 ticks just like the original Xbox. 

<img width="1284" height="1007" alt="The game running on Windows" src="https://github.com/user-attachments/assets/0a4412e1-f6d3-48ed-8e17-0a374c2122f1" />
<p align="center"><em>OpenCE running on Windows (release build)</em></p>

## A copy of Halo: Combat Evolved is required

OpenCE does not include the game data. You **must provide your own Xbox disc image** (`.xiso` or `.iso`) of Halo: Combat Evolved. Every version of the game works. The maps of the European (PAL) version were made for a slower console - the game changes them to play as the North American (NTSC) maps do, so players of the two versions can play together.

## Hardware

The 32-bit Onn GoogleTV app has been tested on:

| Device | CPU | GPU | RAM |
| --- | --- | --- | --- |
| onn 4K Pro Streaming Device Gen 2 (2026), Android 14 (32-bit) | Amlogic S905X5M, 4x Cortex-A55 | Arm Mali-G310 (OpenGL ES 3.2) | 3 GB |
| onn 4K Plus Streaming Device (2026), Android 14 (32-bit) | Amlogic S905X5M, 4x Cortex-A55 | Arm Mali-G310 (OpenGL ES 3.2) | 2 GB |

Both hold about 30 fps in small multiplayer matches, with occasional dips below
that. Some of the main project's features, such as high player counts and
modded co-op campaigns, may not be playable on this hardware.

Older onn boxes and other 32-bit AndroidTV and GoogleTV boxes may work, but are
untested.

## Download

GitHub Actions builds the app for each commit to `armeabi-v7a`
([onn-googletv.yml](workflows/onn-googletv.yml)) and publishes it as a release:

| Platform | Release | Debug | Guide |
| --- | --- | --- | --- |
| **Onn GoogleTV 32-bit** (armeabi-v7a) | [opence-onn-googletv-release.zip](https://github.com/murderbymodem/OpenCE-Onn-GoogleTV/releases/latest/download/opence-onn-googletv-release.zip) | [opence-onn-googletv-debug.zip](https://github.com/murderbymodem/OpenCE-Onn-GoogleTV/releases/latest/download/opence-onn-googletv-debug.zip) | [The 32-bit app](../docs/onn_googletv_android.md) |

Release build is recommended for regular play.

The debug build stops at the first failed check (assertion) and writes it to the log, so use it when you want to find and report problems.

The app updates itself. At start-up it looks for a newer release, and asks
if you want to install it. The [Releases](https://github.com/murderbymodem/OpenCE-Onn-GoogleTV/releases)
page keeps the last five releases.

## Credits

The decompilation is of the Xbox build 2342 (`cachebeta.exe`, SHA-256
`4cc87b45f721270392a96f1674ed2b5cd4a7bb4355faeab4531d1cf1884d9520`).
OpenCE starts from the decompilation of
[bnunu/halo-1](https://github.com/bnunu/halo-1), which is a fork of
[punpckhdq/halo](https://github.com/punpckhdq/halo). The artists of the
high-res redraws are in [AUTHORS.md](../AUTHORS.md). The main contributors are Jonas Volman (@bnunu) and MrBruh (@cybersecurity). Everyone else who has contributed is on the
[contributors page](https://github.com/OpenCommunityEdition/OpenCE/graphs/contributors).

## Licence

OpenCE is free software: you can give it to others and change it under the
terms of the GNU Affero General Public License as published by the Free
Software Foundation, version 3 of the License or (at your option) any later
version (`AGPL-3.0-or-later`). It comes with no warranty. The full text is in
[LICENSE](../LICENSE).

- **From the change.** The AGPL applies from the commit that made the change,
  and to each release built from that commit or a later one. Earlier
  releases, and the source at every earlier commit, were released under
  [CC0 1.0](../LICENSES/CC0-1.0.txt) and stay that way.
- **What OpenCE builds on.** The decompilation of
  [punpckhdq/halo](https://github.com/punpckhdq/halo) is under the AGPL 3.0
  from 17 October 2026, and under CC0 1.0 before that date.
  [bnunu/halo-1](https://github.com/bnunu/halo-1) is under CC0 1.0.
- **Contributions** are under `AGPL-3.0-or-later`. A change taken from a
  project under GPL 3.0 keeps that licence and its notices; the GPL 3.0 and
  the AGPL 3.0 let the two be combined (section 13 of each).
- **Hosts and servers.** If you change the game and let other persons play
  with your version over a network (for example, as the host of an internet
  game, or on a web site), the AGPL asks you to offer them the source of
  your version (section 13). The game as released has its source here, and
  each release's source is its tag (`build-<n>`).
- **Third-party code, fonts and art** keep their own licences, which are
  next to them (`port/third_party`, `port/assets/fonts`,
  `port/assets/icons`, `port/assets/text_filter`). The release downloads and
  the web site's `licenses.txt` carry their notices.
- **Halo is not covered.** The licence covers the work of OpenCE's authors.
  It gives no rights in Halo, its maps or anything else that Microsoft owns.


## Disclaimer
OpenCE (Open Community Edition) is an unofficial fan project. It is not endorsed by or affiliated with Microsoft, Xbox Game Studios, Halo Studios, Activision or Bungie. Halo and Halo: Combat Evolved are trademarks of Microsoft Corporation; Halo © Microsoft Corporation.
No game data is included. You must provide your own copy of the game.
