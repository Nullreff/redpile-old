#![feature(libc)]
#![feature(exit_status)]

extern crate libc;
use libc::{c_char, c_int};
use std::env::args;
use std::ffi::CString;
use std::env::set_exit_status;

#[link(name = "lua", kind= "static")]
#[link(name = "linenoise", kind= "static")]
#[link(name = "redpile-core", kind= "static")]
extern {
    fn redpile_run(argc: c_int, argv: *const *const c_char) -> c_int;
}

fn main() {
    let arg_strs = args().map(|arg| CString::new(arg).unwrap()).collect::<Vec<_>>();
    let arg_ptrs = arg_strs.iter().map(|arg| arg.as_ptr()).collect::<Vec<_>>();
    let result = unsafe {
        redpile_run(arg_ptrs.len() as c_int, arg_ptrs.as_ptr())
    };

    set_exit_status(result);
}
