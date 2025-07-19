/// Converts a value into its sfen string.
///
/// Implement this trait for any type that can be losslessly represented
/// as an sfen fragment (e.g. a complete `State`, a single `Move`).
///
/// # Examples
///
/// ```rust
/// use nshogi::io::ToSfen;
///
/// let state = nshogi::state::State::from_sfen("startpos").unwrap();
/// assert_eq!(
///     "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
///     state.to_sfen());
/// ```
pub trait ToSfen {
    /// Returns the sfen string.
    fn to_sfen(&self) -> String;
}

pub trait ToCSA {
    /// Returns the CSA string.
    fn to_csa(&self) -> String;
}
