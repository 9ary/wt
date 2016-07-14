extern crate libc;

extern {
    pub fn openpty(ptm: *mut libc::c_int, pts: *mut libc::c_int) -> libc::c_int;
}

pub struct Pty {
    ptm: libc::c_int,
    pts: libc::c_int,
}

impl Pty {
    pub fn new() -> Result<Pty, libc::c_int> {
        let mut ret = Pty { ptm: -1, pts: -1 };
        let rc = unsafe { openpty(&mut ret.ptm, &mut ret.pts) };
        match rc {
            0 => Ok(ret),
            e => Err(e),
        }
    }

    pub fn get_fds(&self) -> (libc::c_int, libc::c_int) {
        (self.ptm, self.pts)
    }

    pub fn disown_pts(&mut self) {
        if self.pts >= 0 {
            self.pts = -1;
        }
    }
}

impl Drop for Pty {
    fn drop(&mut self) {
        if self.ptm >= 0 {
            unsafe { libc::close(self.ptm); }
        }

        if self.pts >= 0 {
            unsafe { libc::close(self.pts); }
        }
    }
}
