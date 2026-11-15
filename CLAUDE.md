# OpenCE Onn GoogleTV

A fork of OpenCE (OpenCommunityEdition/OpenCE) that builds the game as a
32-bit Android app (armeabi-v7a) for the onn 4K Pro and onn 4K Plus (2026)
GoogleTV boxes, whose Android runs only 32-bit apps. `AGENTS.md` is the main
project's and its rules apply here too. This file adds the fork's.

Read as needed:

- `docs/onn_googletv_android.md`: what the fork adds (the 32-bit app, its
  settings and defaults, its build options).
- `docs/onn_googletv_handoff.md`: the work's history, profiles, decisions,
  builds and releases, signing, next steps.
- `docs/onn_googletv_runbook.md`: what to do for each email and issue the
  workflow sends (conflicts, failed builds, the token, rollbacks).

## The fork's rules

- **Stay close to the main project.** `.github/workflows/onn-googletv.yml`
  merges each of its releases every six hours, and a conflict needs a
  person. Put the fork's changes in files of its own (`*_onn_googletv*`,
  `docs/onn_googletv_*`, new source files) with the smallest hooks in shared
  files. Never edit the main project's documents (`AGENTS.md`,
  `port/*/README.md` and the like): the fork's documentation goes in
  `docs/onn_googletv_*.md`. `README.md` is the one exception: only its
  title, the onn download badge, the hardware section and the onn download
  row are the fork's.
- **Online play with the main project must keep working.** Change nothing
  the machines of a network game send or must agree on, and keep
  `HALO_PORT_NETWORK_VERSION` as the main project's.
- **Branches:** `armeabi-v7a` is the default branch, and each push to it
  publishes a release the boxes' app installs; `main` mirrors the main
  project and is never edited. Work on a `claude/` branch and open a pull
  request; the user merges it. Test a branch with the workflow's Run
  workflow on that branch: it builds and signs, and publishes nothing.
- **Releases:** the workflow makes them (`tools/onn_googletv_version.sh`).
  Version codes only rise (Android installs no lower one); a bad release is
  undone with the workflow's rollback input, not by deleting it.
- **Signing:** never make a new key, and never put a secret in the
  repository. The key's certificate is SHA-256
  `dc9787ca36279161fbcfdb2a5042afbc7427a37945b4ef0657554b9a222fec58`.
  Only the sign job sees the key: keep the build job free of secrets.
- **The main project's workflows** (`build.yml`, `server-image.yml`) stay in
  the tree, disabled in the repository's Actions settings.
- **Hardware:** nothing here can run the app. Build with
  `python configure.py --release --android-abis armeabi-v7a` and
  `ninja android`, run `tools/test_host_alignment.py`, and say what was not
  run on a box: the user tests on the onn boxes.
- **Out of scope** (decided with the user): busy co-op games with many
  players, controller rumble over Bluetooth, and offering the fork's fixes
  to the main project.
