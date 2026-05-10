"""
Collects the inventory.exe, all MinGW runtime DLLs and the GTK4
data directories into a self-contained dist/ folder that can be
packaged into a Windows installer.

Usage (called by CMake post-build or manually):
    python collect_dist.py <exe_path> <dist_dir> [msys2_root]
"""

import subprocess
import sys
import os
import re
import shutil

MSYS2_ROOT = "C:/msys64/mingw64"


def copy_dlls(exe_path, dist_dir, msys2_root):
    bash = "C:/msys64/usr/bin/bash.exe"
    result = subprocess.run(
        [bash, "-c", f"ldd '{exe_path}'"],
        capture_output=True, text=True
    )
    for line in result.stdout.splitlines():
        m = re.search(r"=> (/[a-z]/[^ ]+\.dll)", line)
        if m and ("msys64" in m.group(1) or "mingw64" in m.group(1)):
            msys_path = m.group(1)
            win_path = re.sub(r"^/([a-z])/", lambda x: x.group(1).upper() + ":/", msys_path)
            if os.path.isfile(win_path):
                dest = os.path.join(dist_dir, os.path.basename(win_path))
                if not os.path.exists(dest):
                    shutil.copy2(win_path, dest)


def copy_gtk4_data(dist_dir, msys2_root):
    items = [
        # GLib settings schemas — required for GTK4 startup
        (os.path.join(msys2_root, "share/glib-2.0/schemas"),
         os.path.join(dist_dir, "share/glib-2.0/schemas")),

        # GTK4 assets (emoji, builder schema)
        (os.path.join(msys2_root, "share/gtk-4.0"),
         os.path.join(dist_dir, "share/gtk-4.0")),

        # Fontconfig — required for Pango text rendering
        (os.path.join(msys2_root, "etc/fonts"),
         os.path.join(dist_dir, "etc/fonts")),

        # GdkPixbuf loaders — required for image support
        (os.path.join(msys2_root, "lib/gdk-pixbuf-2.0"),
         os.path.join(dist_dir, "lib/gdk-pixbuf-2.0")),

        # Icons — Adwaita is GTK4's default theme
        (os.path.join(msys2_root, "share/icons/Adwaita"),
         os.path.join(dist_dir, "share/icons/Adwaita")),
        (os.path.join(msys2_root, "share/icons/hicolor"),
         os.path.join(dist_dir, "share/icons/hicolor")),
    ]

    for src, dst in items:
        if os.path.exists(src):
            if os.path.isdir(src):
                if os.path.exists(dst):
                    shutil.rmtree(dst)
                shutil.copytree(src, dst)
            else:
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                shutil.copy2(src, dst)


def main():
    exe_path  = sys.argv[1].replace("\\", "/")
    dist_dir  = sys.argv[2].replace("\\", "/")
    msys2_root = sys.argv[3].replace("\\", "/") if len(sys.argv) > 3 else MSYS2_ROOT

    os.makedirs(dist_dir, exist_ok=True)

    print("Copying exe...")
    shutil.copy2(exe_path, dist_dir)

    print("Copying DLLs...")
    copy_dlls(exe_path, dist_dir, msys2_root)

    print("Copying GTK4 runtime data...")
    copy_gtk4_data(dist_dir, msys2_root)

    print(f"Done. Distribution collected in: {dist_dir}")


if __name__ == "__main__":
    main()
