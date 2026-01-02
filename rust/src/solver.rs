use std::ffi::c_void;
use std::ptr::NonNull;

use crate::nshogi::NSHOGI_SOLVER_API;
use crate::state::State;
use crate::types::Move;

/// Depth-First Proof-Number (*df-pn*) checkmate solver.
pub struct DfPnSolver {
    handle: NonNull<c_void>,
}

impl DfPnSolver {
    /// Creates a new solver that may allocate up to `memory_mb` MB.
    pub fn new(memory_mb: usize) -> Self {
        Self {
            handle: NSHOGI_SOLVER_API.create_dfpn_solver(memory_mb),
        }
    }

    /// Searches for a checkmate sequence from `state`.
    ///
    /// The search examines up to `max_node_count` nodes and
    /// `max_depth` plies.
    /// It returns the first move of a proven mating line, or
    /// none move if no checkmate is found within the given limits.
    ///
    /// Passing `0` for either limit disables that limit.
    ///
    /// ```
    /// // No checkmate at the initial position.
    /// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// let mut solver = nshogi::solver::DfPnSolver::new(64);
    /// let checkmate_move = solver.solve(&mut state, 0, 0, false);
    /// assert!(checkmate_move.is_none());
    /// ```
    ///
    /// ```
    /// // Chakmte by five plies.
    /// use nshogi::io::ToSfen;
    /// let sfen = "9/9/4k4/9/4P4/9/9/9/K8 b 3G2r2bg4s4n4l17p 1";
    /// let mut state = nshogi::state::State::from_sfen(sfen).unwrap();
    /// let mut solver = nshogi::solver::DfPnSolver::new(64);
    /// let checkmate_move = solver.solve(&mut state, 0, 0, false);
    /// assert_eq!(
    ///     "G*5d",
    ///     checkmate_move.to_sfen()
    /// );
    /// ```
    pub fn solve(&mut self, state: &mut State, max_node_count: u64, max_depth: i32, strict: bool) -> Move {
        NSHOGI_SOLVER_API.solve_by_dfpn(
            self.handle.as_ptr(),
            state.handle.as_ptr(),
            max_node_count,
            max_depth,
            strict,
        )
    }

    /// Searches for a checkmate sequence from `state`.
    ///
    /// The search examines up to `max_node_count` nodes and
    /// `max_depth` plies.
    /// It returns the full checkmate sequence as a vector of moves,
    /// or an empty vector if no checkmate is found within the given limits.
    ///
    /// Passing `0` for either limit disables that limit.
    ///
    /// ```
    /// // No checkmate at the initial position.
    /// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// let mut solver = nshogi::solver::DfPnSolver::new(64);
    /// let checkmate_sequence = solver.solve_with_pv(&mut state, 0, 0, false);
    /// assert!(checkmate_sequence.is_empty());
    /// ```
    /// ```
    /// // Chakmte by one ply.
    /// use nshogi::io::ToSfen;
    /// let sfen = "4k4/9/4P4/9/9/9/9/9/K8 b G2r2b3g4s4n4l17p 1";
    /// let mut state = nshogi::state::State::from_sfen(sfen).unwrap();
    /// let mut solver = nshogi::solver::DfPnSolver::new(64);
    /// let checkmate_sequence = solver.solve_with_pv(&mut state, 0, 0, false);
    /// let sfen_sequence: Vec<String> = checkmate_sequence.iter().map(|m| m.to_sfen()).collect();
    /// assert_eq!(
    ///     vec!["G*5b"],
    ///     sfen_sequence
    /// );
    /// ```
    pub fn solve_with_pv(&mut self, state: &mut State, max_node_count: u64, max_depth: i32, strict: bool) -> Vec<Move> {
        NSHOGI_SOLVER_API.solve_with_pv_by_dfpn(
            self.handle.as_ptr(),
            state.handle.as_ptr(),
            max_node_count,
            max_depth,
            strict,
        )
    }
}

impl Drop for DfPnSolver {
    fn drop(&mut self) {
        NSHOGI_SOLVER_API.destroy_dfpn_solver(self.handle.as_ptr());
    }
}

/// Searches for a checkmate sequence in from `state` using
/// depth-first search (DFS) up to `depth` plies, and returns
/// the first move of the checkmate sequence, or none move if no
/// checkmate sequence is found.
///
/// ```
/// // No checkmate at the initial position.
/// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
/// let checkmate_move = nshogi::solver::dfs(&mut state, 5);
/// assert!(checkmate_move.is_none());
/// ```
///
/// ```
/// // Chakmte by five plies.
/// use nshogi::io::ToSfen;
/// let sfen = "9/9/4k4/9/4P4/9/9/9/K8 b 3G2r2bg4s4n4l17p 1";
/// let mut state = nshogi::state::State::from_sfen(sfen).unwrap();
/// let checkmate_move = nshogi::solver::dfs(&mut state, 5);
/// assert_eq!(
///     "G*5d",
///     checkmate_move.to_sfen()
/// );
/// ```
pub fn dfs(state: &mut State, depth: i32) -> Move {
    NSHOGI_SOLVER_API.solve_by_dfs(state.handle.as_ptr(), depth)
}

#[test]
fn dfs_mate_1_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let file = File::open("../res/test/mate-1-ply.txt").expect("mate-1-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = dfs(&mut state, 1);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfs_mate_3_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let file = File::open("../res/test/mate-3-ply.txt").expect("mate-3-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = dfs(&mut state, 3);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfs_mate_5_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let file = File::open("../res/test/mate-5-ply.txt").expect("mate-5-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = dfs(&mut state, 5);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_mate_1_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-1-ply.txt").expect("mate-1-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, false);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_mate_3_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-3-ply.txt").expect("mate-3-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, false);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_mate_5_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-5-ply.txt").expect("mate-5-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, false);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_mate_7_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-7-ply.txt").expect("mate-7-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, false);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_mate_9_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-9-ply.txt").expect("mate-9-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, false);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_mate_11_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-11-ply.txt").expect("mate-11-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, false);
        assert!(!checkmate_move.is_none());
    }
}

#[test]
fn dfpn_strict_mate_11_ply() {
    use std::fs::File;
    use std::io::{BufRead, BufReader};

    let mut solver = DfPnSolver::new(64);

    let file = File::open("../res/test/mate-11-ply.txt").expect("mate-11-ply.txt not found.");
    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.expect("Couldn't read a line.");
        let mut state =
            State::from_sfen(&line).expect("Couldn't create state object from a sfen line.");

        let checkmate_move = solver.solve(&mut state, 100000, 0, true);
        assert!(!checkmate_move.is_none());
    }
}
