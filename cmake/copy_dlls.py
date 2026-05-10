import subprocess
import sys
import os
import re
import shutil

def main():
    target_file = sys.argv[1].replace("\\", "/")
    target_dir  = sys.argv[2].replace("\\", "/")

    bash = "C:/msys64/usr/bin/bash.exe"
    result = subprocess.run(
        [bash, "-c", f"ldd '{target_file}'"],
        capture_output=True, text=True
    )

    for line in result.stdout.splitlines():
        m = re.search(r"=> (/[a-z]/[^ ]+\.dll)", line)
        if m and ("msys64" in m.group(1) or "mingw64" in m.group(1)):
            msys_path = m.group(1)
            win_path = re.sub(
                r"^/([a-z])/",
                lambda x: x.group(1).upper() + ":/",
                msys_path
            )
            if os.path.isfile(win_path):
                dest = os.path.join(target_dir, os.path.basename(win_path))
                if not os.path.exists(dest):
                    shutil.copy2(win_path, dest)

if __name__ == "__main__":
    main()
