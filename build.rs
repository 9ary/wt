extern crate gcc;

fn main() {
    gcc::Config::new()
        .file("c/pty.c")
        .file("c/subprocess.c")
        .flag("-std=c11")
        .flag("-D_XOPEN_SOURCE=700")
        .flag("-D_DEFAULT_SOURCE")
        .compile("libwt_c.a")
}
