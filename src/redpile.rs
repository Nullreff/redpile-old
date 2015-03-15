/* redpile.rs - Voxel logic simulator
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#![feature(collections)]
#![feature(exit_status)]
#![feature(libc)]
#![feature(old_io)]

extern crate getopts;
extern crate libc;
mod common;

use common::is_power_of_two;
use getopts::Options;
use libc::{c_char, c_int, c_uint, c_ushort, EXIT_FAILURE};
use std::env::args;
use std::env::set_exit_status;
use std::ffi::CString;
use std::old_io::stderr;

static REDPILE_VERSION: &'static str = "0.5.0";
static DEFAULT_WORLD_SIZE: u32 = 1024;

struct Config {
    world_size: u32,
    interactive: bool,
    port: u16,
    benchmark: u32,
    file: String,
}

enum ConfigStatus {
    Failed(String),
    Done,
}

#[repr(C)]
struct CConfig {
    world_size: c_uint,
    interactive: c_int,
    port: c_ushort,
    benchmark: c_uint,
    file: *const c_char,
}

#[link(name = "lua", kind= "static")]
#[link(name = "linenoise", kind= "static")]
#[link(name = "redpile-core", kind= "static")]
extern {
    fn setup_signals();
    fn redpile_run(config: &CConfig) -> c_int;
}

fn print_help(program: &str, opts: Options) {
    println!("Redpile - A Voxel Logic Simulator");
    let brief = format!("Usage: {} [options] <config>", program);
    print!("{}", opts.usage(&brief));
}

fn print_version() {
    println!("Redpile {}", REDPILE_VERSION);
}

fn parse_world_size(string: &str) -> Result<u32, ConfigStatus> {
    match string.parse() {
        Ok(value) if value > 0 && is_power_of_two(value) => Ok(value),
        _ => Err(ConfigStatus::Failed(
            format!("You must pass a power of two between 1 and {} as the world size", std::u32::MAX))),
    }
}

fn parse_port_number(string: &str) -> Result<u16, ConfigStatus> {
    match string.parse() {
        Ok(value) if value > 0 => Ok(value),
        _ => Err(ConfigStatus::Failed(
            format!("You must pass a number between 1 and {} as the port number", std::u16::MAX))),
    }
}

fn parse_benchmark_size(string: &str) -> Result<u32, ConfigStatus> { 
    match string.parse() {
        Ok(value) if value > 0 => Ok(value),
        _ => Err(ConfigStatus::Failed(
            format!("You must pass a positive number of seconds to run each benchmark for"))),
    }
}

fn parse_options() -> Result<Config, ConfigStatus> {
    let args: Vec<_> = args().collect();
    let name = args[0].clone();

    let mut options = Options::new();
    options.optopt("w", "world-size",
                   "The number of nodes to allocate initially", "SIZE");
    options.optflag("i", "interactive",
                    "Run in interactive mode with a prompt for reading commands");
    options.optopt("p", "port",
                    "Listen on the specified port for commands", "PORT");
    options.optopt("b", "benchmark",
                   "Run each benchmark for the time specified", "MILLISECONDS");
    options.optflag("v", "version",
                    "Print the current version");
    options.optflag("h", "help",
                    "Print this message");

    let matches = match options.parse(args.tail()) {
        Ok(m)  => m,
        Err(f) => return Err(ConfigStatus::Failed(f.to_string()))
    };

    if matches.opt_present("h") {
        print_help(&name, options);
        return Err(ConfigStatus::Done);
    }

    if matches.opt_present("v") {
        print_version();
        return Err(ConfigStatus::Done);
    }

    let config = Config {
        world_size: match matches.opt_str("w") {
            Some(string) => try!(parse_world_size(&string)),
            None => DEFAULT_WORLD_SIZE,
        },

        interactive: matches.opt_present("i"),

        port: match matches.opt_str("p") {
            Some(string) => try!(parse_port_number(&string)),
            None => 0,
        },

        benchmark: match matches.opt_str("b") {
            Some(string) => try!(parse_benchmark_size(&string)),
            None => 0,
        },

        file: if !matches.free.is_empty() {
            matches.free[0].clone()
        } else {
            return Err(ConfigStatus::Failed("You must provide a configuration file".to_string()))
        },
    };

    Ok(config)
}

fn main() {
    unsafe { setup_signals(); }

    let config = match parse_options() {
        Ok(s) => s,
        Err(ConfigStatus::Done) => return,
        Err(ConfigStatus::Failed(f)) => {
            writeln!(&mut stderr(), "{}", f);
            set_exit_status(EXIT_FAILURE);
            return;
        },
    };

    let file = CString::new(config.file).unwrap();

    let c_config = CConfig {
        world_size: config.world_size,
        interactive: config.interactive as c_int,
        port: config.port,
        benchmark: config.benchmark,
        file: file.as_ptr(),
    };

    let result = unsafe { redpile_run(&c_config) };
    set_exit_status(result);
}
