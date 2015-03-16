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

use getopts::Options;
use libc::{c_void, c_char, c_int, c_uint, c_ushort, EXIT_FAILURE, EXIT_SUCCESS};
use std::ffi::CString;
use std::env::args;
use std::env::set_exit_status;
use std::old_io::stderr;

static REDPILE_VERSION: &'static str = "0.5.0";
static DEFAULT_WORLD_SIZE: u32 = 1024;

#[link(name = "lua", kind= "static")]
#[link(name = "linenoise", kind= "static")]
#[link(name = "redpile-core", kind= "static")]
extern {
    fn setup_signals();
    fn set_globals(config: &ReplConfig, state: *mut c_void, world: *mut c_void);
    fn redpile_cleanup();
    fn bench_run(world: *mut c_void, count: c_uint);
    fn repl_run();
    fn script_state_allocate() -> *mut c_void;
    fn script_state_free(state: *mut c_void);
    fn script_state_load_config(state: *mut c_void, file: *const c_char) -> *mut c_void;
    fn world_allocate(size: c_uint, type_data: *mut c_void) -> *mut c_void;
}

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
struct ReplConfig {
    interactive: c_int,
    port: c_ushort,
}

struct ScriptState {
    ptr: *mut c_void,
}

struct TypeData {
    ptr: *mut c_void,
}

struct World {
    pub ptr: *mut c_void,
}

impl ScriptState {
    pub fn new() -> ScriptState {
        ScriptState {
            ptr: unsafe { script_state_allocate() },
        }
    }

    pub fn free(&self) {
        unsafe {
            script_state_free(self.ptr);
        }
    }

    pub fn load(&self, file: &str) -> Option<TypeData> {
        let file_string = CString::new(file).unwrap();
        let data = unsafe { script_state_load_config(self.ptr, file_string.as_ptr()) };
        if !data.is_null() {
            Some(TypeData { ptr: data })
        } else {
            None
        }
    }
}

impl World {
    pub fn new(size: u32, types: TypeData) -> World {
        World {
            ptr: unsafe { world_allocate(size, types.ptr) },
        }
    }
}

fn is_power_of_two(x: u32) -> bool {
    x & (x - 1) == 0
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

    let state = ScriptState::new();
    let types = match state.load(&config.file) {
        Some(t) => t,
        None => {
            state.free();
            set_exit_status(EXIT_FAILURE);
            return;
        },
    };

    let world = World::new(config.world_size, types);

    let repl_config = ReplConfig {
        port: config.port,
        interactive: config.interactive as c_int,
    };

    unsafe { set_globals(&repl_config, state.ptr, world.ptr); }

    if config.benchmark > 0 {
        unsafe { bench_run(world.ptr, config.benchmark); }
    } else {
        unsafe { repl_run(); }
    }

    unsafe { redpile_cleanup(); }
    set_exit_status(EXIT_SUCCESS);
}
