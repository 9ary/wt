# wt
A clean and functional terminal emulator.  
Still WIP, contributions welcome.

## Building
Install [Shogun](https://github.com/Streetwalrus/shogun) and [Ninja](https://ninja-build.org/).  
Clang is the preferred compiler, modify `build.ninja` (and replace `-Weverything` in `configure.py`) if you want to use
GCC instead.  
```bash
./configure.py
ninja
./build/wt
```
