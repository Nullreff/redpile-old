#![feature(libc)]
#![feature(exit_status)]
#![feature(collections)]

extern crate libc;
extern crate getopts;

use libc::c_int;
use getopts::Options;
use std::env::args;
use std::env::set_exit_status;

static REDPILE_VERSION: &'static str = "0.5.0";

#[repr(C)]
struct Config {
    world_size: i32,
    interactive: bool,
    port: u16,
    benchmark: u32,
    file: String,
}

enum ParseResult {
    Success(Config),
    Fail(String),
    Exit,
}

#[link(name = "lua", kind= "static")]
#[link(name = "linenoise", kind= "static")]
#[link(name = "redpile-core", kind= "static")]
extern {
    fn setup_signals();
    fn redpile_run(config: &Config) -> c_int;
}

fn print_help(program: &str, opts: Options) {
    println!("Redpile - A Voxel Logic Simulator");
    let brief = format!("Usage: {} [options] <config>", program);
    print!("{}", opts.usage(&brief));
}

fn print_version() {
    println!("Redpile {}", REDPILE_VERSION);
}

fn parse_options() -> ParseResult {
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
        Err(f) => panic!(f.to_string())
    };

    if matches.opt_present("h") {
        print_help(&name, options);
        return ParseResult::Exit;
    }
    if matches.opt_present("v") {
        print_version();
        return ParseResult::Exit;
    }

    ParseResult::Exit
}

fn main() {
    unsafe { setup_signals(); }

    let config = match parse_options() {
        ParseResult::Success(s) => s,
        ParseResult::Fail(f)    => panic!(f),
        ParseResult::Exit       => return,
    };

    let result = unsafe { redpile_run(&config) };
    set_exit_status(result);
}
