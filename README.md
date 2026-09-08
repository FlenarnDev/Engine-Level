# Engine-Level

Engine-Level is a native F4SE plugin project for Fallout 4 for Fallout Cascadia, built using CommonLibF4 and XMake.

## Requirements

- C++23 Compiler (MSVC or Clang-CL)  
- [XMake](https://xmake.io/) [3.0.0+]
- [CommonLibF4](https://github.com/libxse/commonlibf4)

## Building

This repo uses git submodules for its third-party dependencies. Clone with:

```bash
git clone --recurse-submodules <repo-url>
```

Or, if you already have a clone without them:

```bash
git submodule update --init --recursive
```

Then:

```bash
xmake
```

A properly configured environment is expected. 

## Structure
```bash
/src
/lib
/xmake.lua
```

## Notes
* Targets Fallout 4 runtime via F4SE
* Uses CommonLibF4 for engine interaction
* Intended for low-level engine work
* Targeting the latest Fallout 4 runtime, currently 1.11.240

## License

This project is licensed under the GNU General Public License v3.0.

See `LICENSE` for details.

## Contributions

Pull requests are accepted but not guaranteed to be reviewed or merged.

## Disclaimer

This project depends on internal game structures and may break with game updates or changes to the address library.
