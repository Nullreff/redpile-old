Getting Started
===============

*Note: Redpile is currently only suppored on [Debian](https://www.debian.org/) as that's what my development machine runs.
Official support for other operating systems will come once it hits 1.0.0*

Installation
------------

```bash
sudo apt-get install git gcc build-essential cmake bison flex
git clone https://github.com/Nullreff/redpile.git
cd redpile
make
```

There should now be a `redpile` executable in the `build` directory.
The `master` branch should always build and pass all tests.
If it doesn't, please open an issue.

Usage
-----

For playing around, you can run redpile in `--interactive` (`-i`) mode.
See `--help` for a list of all options.

```bash
$ redpile -i
>
```

You can set blocks, get blocks and run ticks.

```bash
$ redpile -i
> set 0 0 0 torch up
> get 0 0 0
(0,0,0) TORCH 0 UP
> set 0 0 1 wire
> set 0 0 2 wire
> tick
(0,0,0) POWER 0
(0,0,1) POWER 0
(0,0,2) POWER 0
> tick
(0,0,0) POWER 0
(0,0,2) POWER 14
(0,0,1) POWER 15
>
```

For more commands, check out the [commands](commands) and [blocks](blocks) pages.

