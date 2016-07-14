mod pty;
mod subprocess;

fn main() {
    let mut pty = pty::Pty::new().unwrap();
    let cld = subprocess::Command::new()
        .arg("bash")
        .stdio_from_pty(&mut pty)
        .run()
        .unwrap();

    while let subprocess::Status::Running = cld.wait_noblock() {
    }
}
