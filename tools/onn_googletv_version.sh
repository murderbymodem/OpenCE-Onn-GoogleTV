#!/usr/bin/env bash
# The version of a build of this fork (.github/workflows/onn-googletv.yml):
# the number of the main project's newest release whose commit the build
# contains, and a revision of the fork's own on top of it.
#
#   tools/onn_googletv_version.sh UPSTREAM_URL EXISTING_TAGS_FILE
#
# prints "<code> <name> <based on>": based on is the main project's release
# the build contains (0: none found); name is the players' number (the app's version name,
# the release's title): the base alone for the first release on it, which
# is the main project's release itself (365), then <base>.<n> for the fork's
# own releases after it on the same base (365.1, 365.2), until the build
# contains the main project's next release (366). code is base * 100 +
# revision (the first release on a base is revision 1: 36501, then 36502),
# the app's version code and the release's tag (build-<code>),
# which the updater compares and Android requires to rise. EXISTING_TAGS_FILE
# lists this repository's release tags, one a line. The revision is one more
# than the last release on the same base; a code is always above every
# existing tag's, so a build whose base cannot be found (the main project
# keeps only its last five releases) takes the last base again. The
# repository is a clone that has HEAD and the main project's recent history.
set -euo pipefail
upstream="$1"
existing="$2"

highest=0
while read -r tag; do
	case "$tag" in
	build-*) number="${tag#build-}" ;;
	*) continue ;;
	esac
	[[ "$number" =~ ^[0-9]+$ ]] || continue
	(( number > highest )) && highest=$number
done < "$existing"

# the main project's newest release tag that HEAD contains
base=0
while read -r commit ref; do
	tag="${ref#refs/tags/}"
	number="${tag#build-}"
	[[ "$number" =~ ^[0-9]+$ ]] || continue
	if (( number > base )) && git cat-file -e "$commit^{commit}" 2>/dev/null &&
		git merge-base --is-ancestor "$commit" HEAD 2>/dev/null; then
		base=$number
	fi
done < <(git ls-remote --tags "$upstream" 'build-*' | grep -v '\^{}$')

# (none found: the last release's base; builds before this scheme, at most
# 999, have none)
contains=$base
if (( base == 0 )); then
	base=$(( highest >= 1000 ? highest / 100 : 1 ))
	echo "no release of the main project found in HEAD: base $base" >&2
fi
code=$(( base * 100 + 1 ))
if (( highest >= code )); then
	code=$(( highest + 1 ))
fi
revision=$(( code % 100 ))
if (( revision == 0 )); then
	echo "a hundredth release on base $(( code / 100 - 1 ))" >&2
	exit 1
fi
base=$(( code / 100 ))
# (the bases before this naming, up to 359, kept <base>.<revision>: 359.1 is
# already a release, so 359's next is 359.2)
if (( base <= 359 )); then
	name="$base.$revision"
elif (( revision == 1 )); then
	name="$base"
else
	name="$base.$(( revision - 1 ))"
fi
# (a rebuild of an earlier commit, a rollback, takes the next number on the
# newest base, so that the installed apps take it as an update, but names
# the release it contains)
echo "$code $name $contains"
