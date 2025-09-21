Colossal Cave Adventure
=======================
[![License Badge][]][License] [![GitHub Status][]][GitHub] [![Coverity Status][]][Coverity Scan]

This is the real, full-blown Colossal Cave Adventure game, written by
Will Crowther and Don Woods at Stanford AI Lab in the early '70s.  It is
the first interactive fiction game where the computer simulates and
describes a situation, and the user types in what to do next, in simple
English.

```
You are standing at the end of a road before a small brick building.
Around you is a forest.  A small stream flows out of the building and
down a gully.

> █
```


Quick start
-----------

A tiny Docker container image is available from GitHub:

    docker pull ghcr.io/troglobit/adventure:latest

The save game feature work only if you map your `$HOME` to the
container's `/root` and run:

    docker run -it --rm -v $HOME:/root ghcr.io/troglobit/adventure:latest

This saves the game state to `~/.adventure` when you issue the `suspend`
command.  To resume the game, append the `-r` option:

    docker run -it --rm -v $HOME:/root ghcr.io/troglobit/adventure:latest -r


Tips
----

Before you begin, a few suggestions:

1. Make a [map][].  There are two common ways to handle this.  You can
   use a piece of butcher paper and a pencil and sketch in all the rooms
   as circles or whatever with directions marked for the lines between
   rooms.  The other way to make a map is to make a word table with the
   room names down one side along with numbers you've assigned each
   room.  Along the top of the table you put N, NW, W, SW, S, SE, E and
   so on.  (Remember up and down).  Then you can tell at a glance which
   directions you have tried and haven't tried.

2. Save the game every half hour or so, using the `suspend` command,
   unless you have spent a lot of moves getting nowhere, of course.
   Start with `advent -r` next time to resume from where you were.

3. Take your time.  Gamers have been known to spend months on this one.
   They were spending 2 or 3 hours a day and they weren't beginners!

Try everything!

**Note:** sometimes you will have to go back to a previous location to
be able to do something.  The game parser uses only simple two word
(verb-noun) commands, after all, this is the first text adventure game.


Build & Install
---------------

The project now uses CMake for builds.  A typical build on Unix-like
systems looks like this:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

To install into the default prefix (`/usr/local`) run:

```
sudo cmake --install build --strip
```

Use `-DCMAKE_INSTALL_PREFIX=/usr` (or another path) to install
elsewhere.  Additional configuration toggles include:

* `-DADVENTURE_ENABLE_GETTEXT=OFF` to build without external gettext
  and rely on the internal string tables.
* `-DADVENTURE_STRICT_2WORD_INPUT=OFF` to relax the parser's
  two-word limit.
* `-DADVENTURE_WARN_AS_ERROR=ON` to treat compiler warnings as errors.

The legacy Autotools build system has been retired; `autogen.sh` and
`configure` now only emit a hint to use CMake.


Origin & References
-------------------

The original Fortran code was by William Crowther, with major features
added by Don Woods.  Conversion to BDS C by J. R. Jaeger and UNIX<TM>
standardization by Jerry D. Pohl. The last port to QNX 4 and bug fixes
by James Lummel.  This last version should work without any change on
HP-UX, Sun Solaris, *BSD and GNU/Linux systems.

The actual license of this program is unknown, although most people
agree that it is likely to be considered to be under the public domain.

See the files in the `doc/` directory for the original documentation,
including [cheat][] codes, some history, and a [map][].

Relevant Resources:

 - https://rickadams.org/adventure/
 - http://www.spitenet.com/cave/
 - http://groups.google.com/group/rec.arts.int-fiction/browse_thread/thread/607acaf1a279d4dd/9181d75148500b30 (original code, uncovered)
 - http://en.wikipedia.org/wiki/Colossal_Cave_Adventure

Other writings on this cult classic:

 - http://www.wurb.com/if/game/1
 - http://www.spitenet.com/cave/
 - http://advent.jenandcal.familyds.org/
 - https://github.com/Quuxplusone/Advent

 -- Compiled on July 26th, 2009 and updated August 8th 2023
    by Joachim Wiberg <troglobit@gmail.com>

[cheat]:           doc/cheat.txt
[map]:             doc/map.jpg
[License]:         https://unlicense.org/
[License Badge]:   https://img.shields.io/badge/License-Unlicense-blue.svg
[GitHub]:          https://github.com/troglobit/adventure/actions/workflows/build.yml
[GitHub Status]:   https://github.com/troglobit/adventure/actions/workflows/build.yml/badge.svg
[Coverity Scan]:   https://scan.coverity.com/projects/28918
[Coverity Status]: https://scan.coverity.com/projects/28918/badge.svg
