use std::ffi::CString;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;

extern crate libc;

use super::pty;

#[repr(C)]
#[allow(dead_code)]
enum idtype_t {
    P_ALL,
    P_PID,
    P_PGID,
}

#[repr(C)]
#[allow(dead_code)]
enum si_code {
    CLD_NONE,
    CLD_EXITED,    // Child has exited.
    CLD_KILLED,    // Child was killed.
    CLD_DUMPED,    // Child terminated abnormally.
    CLD_TRAPPED,   // Traced child has trapped.
    CLD_STOPPED,   // Child has stopped.
    CLD_CONTINUED, // Stopped child has continued.
}


#[repr(C)]
struct siginfo_t {
    si_signo: libc::c_int,
    si_errno: libc::c_int,
    si_code: si_code,
    _pad: [libc::c_int; 29],
}

extern {
    fn waitid(idtype: idtype_t,
              id: libc::id_t,
              infop: *mut siginfo_t,
              options: libc::c_int);

    pub fn sp_forkexec(argv: *const *const libc::c_char,
                    child_pid: *mut libc::pid_t,
                    cldin: libc::c_int,
                    cldout: libc::c_int,
                    clderr: libc::c_int,
                    ptm: libc::c_int) -> libc::c_int;
}

pub struct Child {
    pid: libc::pid_t,
}

pub struct Command {
    args: Vec<CString>,
    argv: Vec<*const libc::c_char>,
    stdin: libc::c_int,
    stdout: libc::c_int,
    stderr: libc::c_int,
    ptm: libc::c_int,
}

pub enum Status {
    Running,
    Exited(libc::c_int),
    Killed,
}

impl Child {
    pub fn wait_noblock(&self) -> Status {
        let mut status = siginfo_t {
            si_signo: 0,
            si_errno: 0,
            si_code: si_code::CLD_NONE,
            _pad: [0; 29],
        };
        unsafe {
            waitid(idtype_t::P_PID,
                   self.pid as libc::id_t,
                   &mut status,
                   libc::WNOHANG | libc::WEXITED);
        }
        if status.si_signo == 0 {
            Status::Running
        } else {
            match status.si_code {
                si_code::CLD_EXITED => Status::Exited(status.si_errno),
                si_code::CLD_KILLED => Status::Killed,
                si_code::CLD_DUMPED => Status::Killed,
                _ => Status::Running,
            }
        }
    }
}

impl Command {
    pub fn new() -> Command {
        Command {
            args: Vec::new(),
            argv: Vec::new(),
            stdin: -1,
            stdout: -1,
            stderr: -1,
            ptm: -1,
        }
    }

    pub fn arg<S: AsRef<OsStr>>(&mut self, arg: S) -> &mut Command {
        let arg = CString::new(arg.as_ref().as_bytes()).unwrap();
        self.argv.push(arg.as_ptr());
        self.args.push(arg);
        self
    }

    pub fn stdio_from_pty(&mut self, term: &mut pty::Pty) -> &mut Command {
        let (ptm, pts) = term.get_fds();
        self.stdin = pts;
        self.stdout = pts;
        self.stderr = pts;
        self.ptm = ptm;
        term.disown_pts();
        self
    }

    pub fn run(&mut self) -> Result<Child, libc::c_int> {
        self.argv.push(0 as *const _);

        let mut ret = Child { pid: -1 };
        let rc = unsafe {
            sp_forkexec(self.argv.as_ptr(),
                                 &mut ret.pid,
                                 self.stdin,
                                 self.stdout,
                                 self.stderr,
                                 self.ptm)
        };

        match rc {
            0 => Ok(ret),
            e => Err(e),
        }
    }
}
