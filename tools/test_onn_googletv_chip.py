"""OpenCE Onn GoogleTV's changes to the render chip's family
(port/android/host/host_render_policy_onn_googletv.h): the boxes it names,
and the debug.halo.render_chip override, with Android's property lookup and
the log stood in for."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[1]


@pytest.fixture(scope="module")
def chip(tmp_path_factory):
    compiler = shutil.which("clang")
    if not compiler:
        pytest.skip("clang required")
    work = tmp_path_factory.mktemp("onn-googletv-chip")
    source = work / "chip.c"
    source.write_text(r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "android_render_policy.h"
#define PROP_VALUE_MAX 92
#define HOST_LOG_INFO 4
#define HOST_LOG_WARN 5
static const char *forced = "";
static void host_logf(int priority, const char *format, ...) { (void)priority; (void)format; }
static int __system_property_get(const char *key, char *value)
{
    if (strcmp(key, "debug.halo.render_chip")) abort();
    strcpy(value, forced);
    return (int)strlen(value);
}
#include "host_render_policy_onn_googletv.h"
int main(int argc, char **argv)
{
    if (argc != 5) return 2;
    forced = argv[4];
    printf("%d", onn_googletv_chip_family(atoi(argv[1]), argv[2], argv[3]));
    return 0;
}
''')
    executable = work / "chip"
    result = subprocess.run([compiler, "-m32", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                             "-I", str(ROOT / "port/android/include"), "-I", str(ROOT / "port/android/host"),
                             str(source), "-o", str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.mark.parametrize("family,hardware,platform,forced,expected", [
    # the Technicolor UIW4059MIL (issue 23): unknown -> other
    (0, "uiw4059mil", "uiw4059mil", "", 3),
    (0, "UIW4059MIL", "", "", 3),
    (0, "", "uiw4059mil", "", 3),
    # its family as android_chip_classify found it, where it found one
    (1, "uiw4059mil", "uiw4059mil", "", 1),
    # another board, or one whose name only starts so: as it was
    (0, "uiw4059milx", "", "", 0),
    (0, "bcm72180", "", "", 0),
    (0, "", "", "", 0),
    # the onn boxes (Amlogic: other) and any recognised chip: as they were
    (3, "amlogic", "s905x5m", "", 3),
    (2, "mt8696", "", "", 2),
    # debug.halo.render_chip wins, and only a single digit 0 to 3
    (0, "uiw4059mil", "", "0", 0),
    (3, "amlogic", "", "1", 1),
    (0, "", "", "3", 3),
    (3, "amlogic", "", "4", 3),
    (3, "amlogic", "", "12", 3),
])
def test_chip_family(chip, family, hardware, platform, forced, expected):
    result = subprocess.run([str(chip), str(family), hardware, platform, forced], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    assert int(result.stdout) == expected
