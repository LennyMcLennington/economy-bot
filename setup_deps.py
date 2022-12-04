#!/usr/bin/python3
import subprocess
from pathlib import Path

root_dir = Path(__file__).parent
manual_deps_dir = root_dir / 'manual_deps'

for dep in manual_deps_dir.iterdir():
    subprocess.run(['conan', 'export', dep], check=True)

subprocess.run(['conan', 'install', '-if', root_dir / 'conan_deps', '-b', 'missing', root_dir], check=True)
