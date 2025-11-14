<<<<<<< HEAD
[![DDraceNetwork](docs/assets/TClient_Logo_Horizontal.svg)](https://tclient.app) 

[![Build status](https://github.com/TaterClient/TClient/workflows/Build/badge.svg)](https://github.com/TaterClient/TClient/actions/workflows/build.yaml)
<!-- [![Code coverage](https://github.com/TaterClient/TClient/branch/master/graph/badge.svg)](https://codecov.io/gh/TaterClient/TClient/branch/master) -->
<!-- [![Translation status](https://hosted.weblate.org/widget/ddnet/ddnet/svg-badge.svg)](https://hosted.weblate.org/engage/ddnet/) -->
=======
[![DDraceNetwork](https://ddnet.org/ddnet-small.png)](https://ddnet.org)

[![Build status](https://github.com/ddnet/ddnet/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/ddnet/ddnet/actions/workflows/build.yml?branch=master)
[![Code coverage](https://codecov.io/gh/ddnet/ddnet/branch/master/graph/badge.svg)](https://codecov.io/gh/ddnet/ddnet/branch/master)
[![Translation status](https://hosted.weblate.org/widget/ddnet/ddnet/svg-badge.svg)](https://hosted.weblate.org/engage/ddnet/)
>>>>>>> 693f1e28cbe90c8daf2cdb11a23c2b8a53435951

### Taters custom ddnet client with some modifications

Not guarenteed to be bug free, but I will try to fix them.

If ddnet devs are reading this and want to steal my changes please feel free.

Thanks to tela for the logo design, and solly for svg <3

### Links

[Discord](https://discord.gg/BgPSapKRkZ)
[Website](https://tclient.app)

### Installation

* Download the latest [release](https://github.com/sjrc6/TaterClient-ddnet/releases)
* Download a [nightly (dev/unstable) build](https://github.com/sjrc6/TaterClient-ddnet/actions/workflows/fast-build.yml?query=branch%3Amaster)
* [Clone](https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository) this repo and build using the [guide from DDNet](https://github.com/ddnet/ddnet?tab=readme-ov-file#cloning)

### Scripting

TClient supports the [ChaiScript](https://chaiscript.com/) language for simple tasks

Add scripts to your config dir then run them with `chai [scriptname] [args]`

> [!CAUTION]
> There are no runtime restrictions, you can easily `while (true) {}` yourself or run out of memory, be careful!

```js
var a // Declare a variable
a = 1 // Set it
var b = 2 // Do both at once
var c = "strings"
var d = ["lists", 2] // not strongly typed
// var e, f = d // no list deconstruction
print(d[0] + to_string(d[1])) // explicit to_string required for string concat
var bass = "ba" + "s" + "s"
var ass = bass.substr(1, -1) // both indecies required, use -1 for end
if (a == b) { // brackets required
	print("this will never happen") // output
} else if (c == "strings") { // string comparison
	exec("echo hello world") // run console stuff
}
var current_game_mode = state("game_mode") // Get the current game mode, all states you can get are listed below
def myfunc(a, b, c) { // yeah it uses def for function definition idk
	print(a, b, c)
	if (a == b) { return "early" }
	c // last statement returns like in rust
}
print(myfunc(1, 2, 3)) // prints "early"
for (var i = 0; i < 10; i++) { // for loops (c style)
	print(i) // auto converts to string, will throw if it cant
}
return "top level return"
```

Here is a list of states which are available:

| Return type | Call | Description |
| --- | -- | --- |
| `string` | `state("game_mode")` | Returns the current game mode name (e.g., “DM”, “TDM”, “CTF”). |
| `bool` | `state("game_mode_pvp")` | Whether the current mode is PvP. |
| `bool` | `state("game_mode_race")` | Whether the current mode is a race mode. |
| `bool` | `state("eye_wheel_allowed")` | Whether the “eye wheel” feature is allowed on this server. |
| `bool` | `state("zoom_allowed")` | Whether camera zoom is allowed. |
| `bool` | `state("dummy_allowed")` | Whether using a dummy client is allowed. |
| `bool` | `state("dummy_connected")` | Whether the dummy client is currently connected. |
| `bool` | `state("rcon_authed")` | Whether the client is authenticated with RCON (admin access). |
| `int` | `state("team")` | The player’s current team number. |
| `int` | `state("ddnet_team")` | The player’s DDNet team number. |
| `string` | `state("map")` | The name of the current or connecting map. |
| `string` | `state("server_ip")` | The IP address of the connected or connecting server. |
| `int` | `state("players_connected")` | Number of currently connected players. |
| `int` | `state("players_cap")` | Maximum number of players the server supports. |
| `string` | `state("server_name")` | The server’s name. |
| `string` | `state("community")` | The server’s community identifier. |
| `string` | `state("location")` | The player’s approximate map location (“NW”, “C”, “SE”, etc.). |
| `string` | `state("state")` | The client’s connection state (e.g., “online”, “offline”, “loading”, “demo”). |
| `int` | `state("id", string Name)` | Finds and returns a client ID by player name (exact or case-insensitive match). |
| `string` | `state("name", int Id)` | Returns the name of a player given their client ID. |
| `string` | `state("clan", int Id)` | Returns the clan name of a player given their client ID. |

```js
var wha = include("thatscript.chai") // you can include other scripts, they use absolute paths from config dir
print(what) // prints "top level return"
if (!file_exists("file")) { // check if a file exists, also absolute from config dir
	throw("why doesn't this file exist")
}
```

There is also `math` and `re` modules

```js
import("math")
math.pi
math.e
math.pow(1, 2)
math.sqrt(3)
math.sin(1)
math.cos(1)
math.tan(1)
math.asin(1)
math.acos(1)
math.atan(1)
math.atan2(1, 1)
math.log(1)
math.log10(1)
math.log2(1)
math.ceil(1)
math.floor(1)
math.round(1)
math.abs(1)
```

```js
import("re")

if(re.test(re.compile(".+?ello.+?"), "hello")) { // re.test(r, string)
	print("hi")
}
re.match(re.compile("\\d"), "h3ll0", false, fun[](str, match, group) { // re.match(r, string, global, callback)
	print("not global: " + to_string(match) + " " + str)
})
re.match(re.compile("\\d"), "h3ll0", true, fun[](str, match, group) {
	print("global: " + to_string(match) + " " + str)
})
re.match(re.compile("(h3)l(l0)"), "h3ll0", false, fun[](str, match, group) {
	print("groups: " + to_string(match) + " " + to_string(group) + " " + str)
})
print(re.replace(re.compile("\\d"), "h3ll0", true, fun[](str, match, group) { // re.replace(r, string, global, callback)
	if (str == "3") {
		return "e"
	} else if (str == "0") {
		return "o"
	}
	return str
}))
```

### Settings Page

> [!NOTE]
> This is out of date

![image](https://github.com/user-attachments/assets/a6ccb206-9fed-48be-a2d2-8fc50a6be882)
![image](https://github.com/user-attachments/assets/9251509a-d852-41ac-bf6b-9a610db08945)
![image](https://github.com/user-attachments/assets/47dab977-1311-4963-a11a-81b78005b12b)
![image](https://github.com/user-attachments/assets/29bddfd9-fcf1-420c-b7e0-958493051a3c)
![image](https://github.com/user-attachments/assets/efe3528f-a962-4dc0-aa8c-9ca963c246e5)
![image](https://github.com/user-attachments/assets/9f15023d-2a27-44ee-8157-e76da53c875a)

![image](https://user-images.githubusercontent.com/22122579/182528700-4c8238c3-836e-49c3-9996-68025e7f5d58.png)

### Features

> [!NOTE]
> This is out of date

```
<<<<<<< HEAD
tc_run_on_join_console
tc_run_on_join_delay
tc_nameplate_ping_circle
tc_hammer_rotates_with_cursor
tc_freeze_update_fix
tc_show_center
tc_skin_name
tc_color_freeze
tc_freeze_stars
tc_white_feet
tc_white_feet_skin
tc_mini_debug
tc_last_notify
tc_last_notify_text
tc_last_notify_color
tc_cursor_in_spec
tc_render_nameplate_spec
tc_fast_input
tc_fast_input_others
tc_improve_mouse_precision
tc_frozen_tees_hud
tc_frozen_tees_text
tc_frozen_tees_hud_skins
tc_frozen_tees_size
tc_frozen_tees_max_rows
tc_frozen_tees_only_inteam
tc_remove_anti
tc_remove_anti_ticks
tc_remove_anti_delay_ticks
tc_unpred_others_in_freeze
tc_pred_margin_in_freeze
tc_pred_margin_in_freeze_amount
tc_show_others_ghosts
tc_swap_ghosts
tc_hide_frozen_ghosts
tc_pred_ghosts_alpha
tc_unpred_ghosts_alpha
tc_render_ghost_as_circle
tc_outline
tc_outline_in_entities
tc_outline_freeze
tc_outline_unfreeze
tc_outline_tele
tc_outline_solid
tc_outline_width
tc_outline_alpha
tc_outline_alpha_solid
tc_outline_color_solid
tc_outline_color_freeze
tc_outline_color_tele
tc_outline_color_unfreeze
tc_player_indicator
tc_player_indicator_freeze
tc_indicator_alive
tc_indicator_freeze
tc_indicator_dead
tc_indicator_offset
tc_indicator_offset_max
tc_indicator_variable_distance
tc_indicator_variable_max_distance
tc_indicator_radius
tc_indicator_opacity
tc_indicator_inteam
tc_indicator_tees
tc_profile_skin
tc_profile_name
tc_profile_clan
tc_profile_flag
tc_profile_colors
tc_profile_emote
tc_auto_verify
tc_rainbow
tc_rainbow_others
tc_rainbow_mode
tc_reset_bindwheel_mouse
add_profile
add_bindwheel
remove_bindwheel
delete_all_bindwheel_binds
+bindwheel_execute_hover
+bindwheel
tc_regex_chat_ignore
tc_color_freeze_darken
tc_color_freeze_feet
tc_spec_menu_ID
tc_limit_mouse_to_screen
=======

To clone the libraries if you have previously cloned DDNet without them, or if you require the ddnet-libs history instead of a shallow clone:

```sh
git submodule update --init --recursive
```

## Dependencies on Linux / macOS

You can install the required libraries on your system, `touch CMakeLists.txt` and CMake will use the system-wide libraries by default. You can install all required dependencies and CMake on Debian or Ubuntu like this:

```sh
sudo apt install build-essential cargo cmake git glslang-tools google-mock libavcodec-extra libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libcurl4-openssl-dev libfreetype6-dev libglew-dev libnotify-dev libogg-dev libopus-dev libopusfile-dev libpng-dev libsdl2-dev libsqlite3-dev libssl-dev libvulkan-dev libwavpack-dev libx264-dev ninja-build python3 rustc spirv-tools
```

On older distributions like Ubuntu 18.04 don't install `google-mock`, but instead set `-DDOWNLOAD_GTEST=ON` when building to get a more recent gtest/gmock version.

On older distributions `rustc` version might be too old, to get an up-to-date Rust compiler you can use [rustup](https://rustup.rs/) with stable channel instead or try the `rustc-mozilla` package.

Or on CentOS, RedHat and AlmaLinux like this:

```sh
sudo yum install cargo cmake ffmpeg-devel freetype-devel gcc gcc-c++ git glew-devel glslang gmock-devel gtest-devel libcurl-devel libnotify-devel libogg-devel libpng-devel libx264-devel ninja-build openssl-devel opus-devel opusfile-devel python3 rust SDL2-devel spirv-tools sqlite-devel vulkan-devel wavpack-devel
```

Or on Fedora like this:

```sh
sudo dnf install cargo cmake ffmpeg-devel freetype-devel gcc gcc-c++ git glew-devel glslang gmock-devel gtest-devel libcurl-devel libnotify-devel libogg-devel libpng-devel make ninja-build openssl-devel opus-devel opusfile-devel python SDL2-devel spirv-tools sqlite-devel vulkan-devel wavpack-devel x264-devel
```

Or on Arch Linux like this:

```sh
sudo pacman -S --needed base-devel cmake curl ffmpeg freetype2 git glew glslang gmock libnotify libpng ninja opusfile python rust sdl2 spirv-tools sqlite vulkan-headers vulkan-icd-loader wavpack x264
```

Or on Gentoo like this:

```sh
emerge --ask dev-build/ninja dev-db/sqlite dev-lang/rust-bin dev-libs/glib dev-libs/openssl dev-util/glslang dev-util/spirv-headers dev-util/spirv-tools media-libs/freetype media-libs/glew media-libs/libglvnd media-libs/libogg media-libs/libpng media-libs/libsdl2 media-libs/libsdl2[vulkan] media-libs/opus media-libs/opusfile media-libs/pnglite media-libs/vulkan-loader[layers] media-sound/wavpack media-video/ffmpeg net-misc/curl x11-libs/gdk-pixbuf x11-libs/libnotify
```

On macOS you can use [homebrew](https://brew.sh/) to install build dependencies like this:

```sh
brew install cmake ffmpeg freetype glew glslang googletest libpng molten-vk ninja opusfile rust SDL2 spirv-tools vulkan-headers wavpack x264
```

If you don't want to use the system libraries, you can pass the `-DPREFER_BUNDLED_LIBS=ON` parameter to cmake.

## Building on Linux and macOS

To compile DDNet yourself, execute the following commands in the source root:

```sh
cmake -Bbuild -GNinja
cmake --build build
```

DDNet requires additional libraries, some of which are bundled for the most common platforms (Windows, Mac, Linux, all x86 and x86\_64) for convenience and the official builds. The bundled libraries for official builds are now in the ddnet-libs submodule. Note that when you build and develop locally, you should ideally use your system's package manager to install the dependencies, instead of relying on ddnet-libs submodule, which does not contain all dependencies anyway (e.g. openssl, vulkan). See the previous section for how to get the dependencies. Alternatively see the following build arguments for how to disable some features and their dependencies (`-DVULKAN=OFF` won't require Vulkan for example).

The following is a non-exhaustive list of build arguments that can be passed to the `cmake` command-line tool in order to enable or disable options in build time:

* **-DCMAKE_BUILD_TYPE=[Release|Debug|RelWithDebInfo|MinSizeRel]** <br>
	An optional CMake variable for setting the build type. If not set, defaults to "Release" if `-DDEV=ON` is **not** used, and "Debug" if `-DDEV=ON` is used. See `CMAKE_BUILD_TYPE` in CMake Documentation for more information.

* **-DPREFER_BUNDLED_LIBS=[ON|OFF]** <br>
	Whether to prefer bundled libraries over system libraries. Setting to ON will make DDNet use third party libraries available in the `ddnet-libs` folder, which is the git-submodule target of the [ddnet-libs](https://github.com/ddnet/ddnet-libs) repository mentioned above -- Useful if you do not have those libraries installed and want to avoid building them. If set to OFF, will only use bundled libraries when system libraries are not found. Default value is OFF.

* **-DWEBSOCKETS=[ON|OFF]** <br>
	Whether to enable WebSocket support for server. Setting to ON requires the `libwebsockets-dev` library installed. Default value is OFF.

* **-DMYSQL=[ON|OFF]** <br>
	Whether to enable MySQL/MariaDB support for server. Requires at least MySQL 8.0 or MariaDB 10.2. Setting to ON requires the `libmariadbclient-dev` library installed, which are also provided as bundled libraries for the common platforms. Default value is OFF.

	Note that the bundled MySQL libraries might not work properly on your system. If you run into connection problems with the MySQL server, for example that it connects as root while you chose another user, make sure to install your system libraries for the MySQL client. Make sure that the CMake configuration summary says that it found MySQL libs that were not bundled (no "using bundled libs").

* **-DTEST_MYSQL=[ON|OFF]** <br>
	Whether to test MySQL/MariaDB support in GTest based tests. Default value is OFF.

	Note that this requires a running MySQL/MariaDB database on localhost with this setup:

```sql
CREATE DATABASE ddnet;
CREATE USER 'ddnet'@'localhost' IDENTIFIED BY 'thebestpassword';
GRANT ALL PRIVILEGES ON ddnet.* TO 'ddnet'@'localhost';
FLUSH PRIVILEGES;
```

* **-DAUTOUPDATE=[ON|OFF]** <br>
	Whether to enable the autoupdater. Packagers may want to disable this for their packages. Default value is ON for Windows and Linux.

* **-DCLIENT=[ON|OFF]** <br>
	Whether to enable client compilation. If set to OFF, DDNet will not depend on Curl, Freetype, Ogg, Opus, Opusfile, and SDL2. Default value is ON.

* **-DVIDEORECORDER=[ON|OFF]** <br>
	Whether to add video recording support using FFmpeg to the client. Default value is ON.

* **-DDOWNLOAD_GTEST=[ON|OFF]** <br>
	Whether to download and compile GTest. Useful if GTest is not installed and, for Linux users, there is no suitable package providing it. Default value is OFF.

* **-DDEV=[ON|OFF]** <br>
	Whether to optimize for development, speeding up the compilation process a little. If enabled, don't generate stuff necessary for packaging. Setting to ON will set CMAKE\_BUILD\_TYPE to Debug by default. Default value is OFF.

* **-DUPNP=[ON|OFF]** <br>
	Whether to enable UPnP support for the server.
	You need to install `libminiupnpc-dev` on Debian, `miniupnpc` on Arch Linux.
	Default value is OFF.

* **-DVULKAN=[ON|OFF]** <br>
	Whether to enable the vulkan backend.
	On Windows you need to install the Vulkan SDK and set the `VULKAN_SDK` environment flag accordingly.
	Default value is ON for Windows x86\_64 and Linux, and OFF for Windows x86 and macOS.

* **-GNinja** <br>
	Use the Ninja build system instead of Make. This automatically parallelizes the build and is generally faster. Compile with `ninja` instead of `make`. Install Ninja with `sudo apt install ninja-build` on Debian, `sudo pacman -S --needed ninja` on Arch Linux.

* **-DCMAKE_CXX_LINK_FLAGS=[FLAGS]** <br>
	Custom flags to set for compiler when linking.

* **-DEXCEPTION_HANDLING=[ON|OFF]** <br>
	Enable exception handling (only works with Windows as of now, uses DrMingw there). Default value is OFF.

* **-DIPO=[ON|OFF]** <br>
	Enable interprocedural optimizations, also known as Link Time Optimization (LTO). Default value is OFF.

* **-DFUSE_LD=[OFF|LINKER]** <br>
	Linker to use. Default value is OFF to try mold, lld, gold.

* **-DSECURITY_COMPILER_FLAGS=[ON|OFF]** <br>
	Whether to set security-relevant compiler flags like `-D_FORTIFY_SOURCE=2` and `-fstack-protector-all`. Default Value is ON.

## Running tests (Debian/Ubuntu)

In order to run the tests, you need to install the following library `libgtest-dev`.

This library isn't compiled, so you have to do it:
```sh
sudo apt install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make

# copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp lib/*.a /usr/lib
```

To run the tests you can build the target `run_tests` using

```sh
cmake --build build --target run_tests`
```

## Code formatting

We use clang-format 20 to format the C++ code of this project. Execute `scripts/fix_style.py` after changing the code to ensure code is formatted properly, a GitHub central style checker will do the same and prevent your change from being submitted.

## Using AddressSanitizer + UndefinedBehaviourSanitizer or Valgrind's Memcheck

ASan+UBSan and Memcheck are useful to find code problems more easily. Please use them to test your changes if you can.

For ASan+UBSan compile with:

```sh
CC=clang CXX=clang++ CXXFLAGS="-fsanitize=address,undefined -fsanitize-recover=all -fno-omit-frame-pointer" CFLAGS="-fsanitize=address,undefined -fsanitize-recover=all -fno-omit-frame-pointer" cmake -DCMAKE_BUILD_TYPE=Debug -Bbuild -GNinja
cmake --build build
```

and run with:

```sh
UBSAN_OPTIONS=suppressions=./ubsan.supp:log_path=./SAN:print_stacktrace=1:halt_on_errors=0 ASAN_OPTIONS=log_path=./SAN:print_stacktrace=1:check_initialization_order=1:detect_leaks=1:halt_on_errors=0:handle_abort=1 LSAN_OPTIONS=suppressions=./lsan.supp ./DDNet
```

Check the SAN.\* files afterwards. This finds more problems than memcheck, runs faster, but requires a modern GCC/Clang compiler.

For valgrind's memcheck compile a normal Debug build and run with: `valgrind --tool=memcheck ./DDNet`
Expect a large slow down.

## Building on Windows with the Visual Studio IDE

Download and install some version of [Microsoft Visual Studio](https://www.visualstudio.com/) (At the time of writing, MSVS Community 2022) with **C++ support**.

You'll have to install both [Python 3](https://www.python.org/downloads/) and [Rust](https://rustup.rs/) as well.

Make sure the MSVC build tools, C++ CMake-Tools and the latest Windows SDK version appropriate to your windows version are selected in the installer.

Now open up your Project folder, Visual Studio should automatically detect and configure your project using CMake.

On your tools hotbar next to the triangular "Run" Button, you can now select what you want to start (e.g game-client or game-server) and build it.

## Building on Windows with standalone MSVC build tools

First off you will need to install the MSVC [Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/), [Python 3](https://www.python.org/downloads/) as well as [Rust](https://www.rust-lang.org/tools/install).

To compile and build DDNet on Windows, use your IDE of choice either with a CMake integration (e.g Visual Studio Code), or by ~~**deprecated**~~ using the CMake GUI.

Configure CMake to use the MSVC Build Tools appropriate to your System by your IDE's instructions.

If you're using Visual Studio Code, you can use the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extension to configure and build the project.

You can then open the project folder in VSC and press `Ctrl+Shift+P` to open the command palette, then search for `CMake: Configure`

This will open up a prompt for you to select a kit, select your `Visual Studio` version and save it. You can now use the GUI (bottom left) to compile and build your project.

## Cross-compiling on Linux to Windows x86/x86\_64

Install MinGW cross-compilers of the form `i686-w64-mingw32-gcc` (32 bit) or
`x86_64-w64-mingw32-gcc` (64 bit). This is probably the hard part. ;)

Then add `-DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/mingw64.toolchain` to the
**initial** CMake command line.

## Cross-compiling on Linux/Windows to Android

Cross-compilation to Android is explained in a [separate guide](scripts/android/README.md).

## Cross-compiling on Linux to WebAssembly via Emscripten

Install Emscripten cross-compilers on a modern linux distro. Follow the [instructions to install the emsdk](https://emscripten.org/docs/getting_started/downloads.html). Installing Emscripten from the package manager (e.g. with `sudo apt install emscripten`) may not work, as building some libraries requires a more recent version of Emscripten than available via package manager.

If you need to compile the ddnet-libs for WebAssembly, simply call

```sh
# <directory to build in> should be a directory outside of the project's source directory
scripts/compile_libs/gen_libs.sh "<directory to build in>" webasm
```

from the project's source directory. It will automatically create a directory called `ddnet-libs` in your build directory.
You can then manually merge this directory with the one in the ddnet source directory.

Run `rustup target add wasm32-unknown-emscripten` to install the WASM target for compiling Rust.

Then run `emcmake cmake .. -G "Unix Makefiles" -DVIDEORECORDER=OFF -DVULKAN=OFF -DSERVER=OFF -DTOOLS=OFF -DPREFER_BUNDLED_LIBS=ON` in your build directory to configure followed by `cmake --build . -j8` to build. For testing it is highly recommended to build in debug mode by also passing the argument `-DCMAKE_BUILD_TYPE=Debug` when invoking `emcmake cmake`, as this speeds up the build process and adds debug information as well as additional checks. Note that using the Ninja build system with Emscripten is not currently possible due to [CMake issue 16395](https://gitlab.kitware.com/cmake/cmake/-/issues/16395).

To test the compiled code locally, just use `emrun --browser firefox DDNet.html`

To host the compiled .html file copy all `.data`, `.html`, `.js`, `.wasm` files to the web server. See `other/emscripten/minimal.html` for a minimal HTML example. You can also run `other/emscripten/server.py` to host a minimal server for testing using Python without needing to install Emscripten.

Then enable cross origin policies. Example for apache2 on debian based distros:

```sh
sudo a2enmod header

# edit the apache2 config to allow .htaccess files
sudo nano /etc/apache2/apache2.conf

# set AllowOverride to All for your directory
# then create a .htaccess file on the web server (where the .html is)
# and add these lines
Header add Cross-Origin-Embedder-Policy "require-corp"
Header add Cross-Origin-Opener-Policy "same-origin"

# now restart apache2
sudo service apache2 restart
```

## Cross-compiling on Linux to macOS

Install [osxcross](https://github.com/tpoechtrager/osxcross), then add
`-DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/darwin.toolchain` and
`-DCMAKE_OSX_SYSROOT=/path/to/osxcross/target/SDK/MacOSX10.11.sdk/` to the
**initial** CMake command line.

Install `dmg` and `hfsplus` from
[libdmg-hfsplus](https://github.com/mozilla/libdmg-hfsplus) and `newfs_hfs`
from
[diskdev\_cmds](http://pkgs.fedoraproject.org/repo/pkgs/hfsplus-tools/diskdev_cmds-540.1.linux3.tar.gz/0435afc389b919027b69616ad1b05709/diskdev_cmds-540.1.linux3.tar.gz)
to unlock the `package_dmg` target that outputs a macOS disk image.

## Importing the official DDNet Database

```sh
$ wget https://ddnet.org/stats/ddnet-sql.zip # (~2.5 GiB)
$ unzip ddnet-sql.zip
$ yay -S mariadb mysql-connector-c++
$ sudo mysql_install_db --user=mysql --basedir=/usr --datadir=/var/lib/mysql
$ sudo systemctl start mariadb
$ sudo mysqladmin -u root password 'PW'
$ mysql -u root -p'PW'
MariaDB [(none)]> create database teeworlds; create user 'teeworlds'@'localhost' identified by 'PW2'; grant all privileges on teeworlds.* to 'teeworlds'@'localhost'; flush privileges;
# this takes a while, you can remove the KEYs in record_race.sql to trade performance in queries
$ mysql -u teeworlds -p'PW2' teeworlds < ddnet-sql/record_*.sql

$ cat mine.cfg
sv_use_sql 1
add_sqlserver r teeworlds record teeworlds "PW2" "localhost" "3306"
add_sqlserver w teeworlds record teeworlds "PW2" "localhost" "3306"

$ cmake -Bbuild -DMYSQL=ON -GNinja
$ cmake --build build --target DDNet-Server
$ build/DDNet-Server -f mine.cfg
```

<a href="https://repology.org/metapackage/ddnet/versions">
	<img src="https://repology.org/badge/vertical-allrepos/ddnet.svg?header=" alt="Packaging status" align="right">
</a>

## Installation from Repository

Debian/Ubuntu

```sh
sudo apt-get install ddnet
```

MacOS

```sh
brew install --cask ddnet
```

Fedora

```sh
sudo dnf install ddnet
```

Arch Linux

```sh
yay -S ddnet
```

FreeBSD

```sh
sudo pkg install DDNet
```

Windows (Scoop)
```cmd
scoop bucket add games
scoop install games/ddnet
```

## Benchmarking

DDNet is available in the [Phoronix Test Suite](https://openbenchmarking.org/test/pts/ddnet). If you have PTS installed you can easily benchmark DDNet on your own system like this:

```sh
phoronix-test-suite benchmark ddnet
```

## Better Git Blame

First, use a better tool than `git blame` itself, e.g. [`tig`](https://jonas.github.io/tig/). There's probably a good UI for Windows, too. Alternatively, use the GitHub UI, click "Blame" in any file view.

For `tig`, use `tig blame path/to/file.cpp` to open the blame view, you can navigate with arrow keys or kj, press comma to go to the previous revision of the current line, q to quit.

Only then you could also set up git to ignore specific formatting revisions:

```sh
git config blame.ignoreRevsFile formatting-revs.txt
```

## (Neo)Vim Syntax Highlighting for config files

Copy the file detection and syntax files to your vim config folder:

```sh
# vim
cp -R other/vim/* ~/.vim/

# neovim
cp -R other/vim/* ~/.config/nvim/
>>>>>>> 693f1e28cbe90c8daf2cdb11a23c2b8a53435951
```
