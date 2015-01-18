extern crate libc;
use libc::{c_char, c_int};
use std::os;
use std::ffi::CString;
use std::iter;

#[link(name = "lua", kind= "static")]
#[link(name = "linenoise", kind= "static")]
#[link(name = "redpile-core", kind= "static")]
extern {
    fn redpile_run(argc: c_int, argv: *const CString) -> c_int;
}

fn main() {
    let args = os::args().iter().map(|&:arg| CString::from_slice(arg.as_bytes())).collect::<Vec<CString>>();
    unsafe {
        redpile_run(args.len() as c_int, args.as_ptr());
    }
}
