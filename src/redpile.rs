#![feature(libc)]
#![feature(env)]
#![feature(std_misc)]

extern crate libc;
use libc::{c_char, c_int};
use std::env::args;
use std::ffi::CString;

#[link(name = "lua", kind= "static")]
#[link(name = "linenoise", kind= "static")]
#[link(name = "redpile-core", kind= "static")]
extern {
    fn redpile_run(argc: c_int, argv: *const *const c_char) -> c_int;
}

fn main() {
    let args = args().map(|&:arg| CString::from_slice(arg.as_bytes()).as_ptr())
                     .collect::<Vec<*const c_char>>();
    unsafe {
        redpile_run(args.len() as c_int, args.as_ptr());
    }
}
