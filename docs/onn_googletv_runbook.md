# OpenCE Onn GoogleTV: what to do when the workflow asks

The workflow (`.github/workflows/onn-googletv.yml`) keeps the app up to date
by itself. Every hour it looks for a new release of the main project
(OpenCommunityEdition/OpenCE), merges it, builds the 32-bit app, signs it and
publishes it. The boxes' app offers each new release when it starts.

It needs a person only in the cases below. Each one arrives as an email from
GitHub (a failed run) or as an issue in this repository.

## Merging the main project's build-N conflicts

**What it means:** the main project changed code that this fork also
changed, and git cannot combine the two on its own. Nothing was pushed; the
boxes stay on the last release. Online play keeps working, unless that
release of the main project raised the network version (the release notes
say "Network version"): then the boxes cannot join the main project's games
until the merge is done.

GitHub mails the first failed sync only: while the issue is open, the
hourly syncs that meet the same conflict only warn. A later release of the
main project that still conflicts opens its own issue (and mails once).

**What to do:** the merge needs someone who reads code. Open a Claude Code
session on this repository and ask it to "merge the main project's build-N
into armeabi-v7a and resolve the conflicts" (the issue names N). The next
sync closes the issue (and any other open conflict issue) once
armeabi-v7a has the main project's newest release.

## A build failed / The build of armeabi-v7a keeps failing

**What it means:** a build of armeabi-v7a failed, so no release was
published. Most failures are passing ones (a download that timed out), and
the next syncs build the same commit again, twice. GitHub mails you only
for the first failure (the push's); the sync's own tries mail nobody. After
three failures the sync opens the issue "The build of armeabi-v7a keeps
failing", goes on trying once a day, and closes the issue itself when a
release appears.

**What to do:** nothing, for an email alone. When the issue comes, look at
the run it links: if a download or GitHub itself failed, wait for the daily
try. Otherwise the failure is real: the main project's release does not
build in this fork as it is. As for a conflict, ask a Claude Code session to
"fix the failed build of armeabi-v7a" and give it the run's link.

## A warning that a workflow of the main project's was disabled

**What it means:** a merge brought a workflow file of the main project's
that this repository had not seen (a new one, or one it renamed). Only this
fork's workflow should run here, so the sync disabled it and stopped its
runs. One run of it may have started before that; it has none of the fork's
secrets.

**What to do:** nothing. If the warning says it could not be disabled:
**Actions**, the workflow, **Disable workflow**.

## The sync token expires soon / no longer works

**What it means:** the `OPENCE_ONN_GOOGLETV_SYNC_TOKEN` secret is a token
with an expiry date. The issue comes two weeks before it, and again if it
has expired. Until it is renewed, the sync still works with GitHub's own
token, except for main-project releases that change their workflow files.

**What to do:**

1. On GitHub: your picture, **Settings**, **Developer settings**, **Personal
   access tokens**, **Fine-grained tokens**, the sync token.
2. **Regenerate token**, choose the longest expiry, copy the new value.
3. In this repository: **Settings**, **Secrets and variables**, **Actions**,
   `OPENCE_ONN_GOOGLETV_SYNC_TOKEN`, **Update**, paste it.
4. Put the new value and date in your secrets backup, and close the issue.

## GitHub says the scheduled workflow was disabled

**What it means:** GitHub turns a public repository's schedules off after 60
days without a commit. The sync writes a small note
(`docs/onn_googletv_keepalive.md`) after 45 quiet days to prevent this, so
this should not happen; if it does:

**What to do:** **Actions**, **OpenCE Onn GoogleTV (armeabi-v7a)**,
**Enable workflow**.

## A release misbehaves on the boxes

**What it means:** a release built and published, but something is wrong
when it plays. Android does not let an app go back to an older version, so
the fix is a new release that holds the older code: a rollback.

**What to do:**

1. Find the last good release on the **Releases** page, and its tag, such as
   `build-36502` (only releases from 359.1 on can be rolled back to).
2. **Actions**, **OpenCE Onn GoogleTV (armeabi-v7a)**, **Run workflow**.
   Leave "Use workflow from" on `armeabi-v7a`, put the tag in the **Roll
   back** field, and run it.
3. A few minutes later a new release appears (for example 365.4) whose notes
   say "Rollback" and which OpenCE build it contains. The boxes update to it
   as usual.

The next release of the main project resumes ordinary releases. If the
problem was in the fork's code, or is still in the main project's, it comes
back with that release: roll back again, and ask a Claude Code session to
find and fix it.

## Installing by hand

Any release can be installed over the app with adb, keeping its data (the
key is the same):

```bash
adb install -r app-release.apk
```

An app from build 127 or earlier no longer updates by itself, and needs this
once.

## Things never to do

- **Never make a new signing key.** Apps signed with another key do not
  install over the existing app, and uninstalling it deletes the game data.
  The workflow refuses any key whose certificate is not SHA-256
  `dc9787ca36279161fbcfdb2a5042afbc7427a37945b4ef0657554b9a222fec58`.
- **Never put secrets in the repository.** They live only in **Settings**,
  **Secrets and variables**, **Actions**: `OPENCE_ONN_GOOGLETV_KEYSTORE_BASE64`,
  `OPENCE_ONN_GOOGLETV_KEYSTORE_PASSWORD`, `OPENCE_ONN_GOOGLETV_SYNC_TOKEN`,
  and in your own backup.
- **Never enable the main project's workflows** (`build.yml`,
  `server-image.yml`) in this repository's Actions settings: they are for the
  main project, and stay disabled here.

The technical notes for whoever works on the code are in
[onn_googletv_handoff.md](onn_googletv_handoff.md).
