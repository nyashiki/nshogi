use std::ffi::c_void;
use std::ptr::NonNull;

use crate::nshogi::{NSHOGI_IO_API, NSHOGI_STATE_API};
use crate::types::CSAParseError;
use crate::types::SfenParseError;

use crate::io::ToCSA;
use crate::io::ToSfen;
use crate::position::Position;
use crate::types::{Color, Move, MoveGetError, Repetition};

/// `State` struct is responsible for controlling and maintaining
/// the game state in Shogi. It includes functionality for tracking
/// the side to move, the current piece locations (refer to the
/// `Position` class for more details), applying a legal move,
/// maintaining the game history, and handling the declaration rule.
pub struct State {
    pub(crate) handle: NonNull<c_void>,
}

impl State {
    /// Returns a `State` which is specified by the `sfen` argument.
    pub fn from_sfen(sfen: &str) -> Result<Self, SfenParseError> {
        NSHOGI_IO_API
            .create_state_from_sfen(sfen)
            .map(|handle| Self { handle: handle })
    }

    pub fn from_csa(csa: &str) -> Result<Self, CSAParseError> {
        NSHOGI_IO_API
            .create_state_from_csa(csa)
            .map(|handle| Self { handle: handle })
    }

    /// Returns the side to move.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert_eq!(nshogi::types::Color::BLACK, state.get_side_to_move());
    /// ```
    #[inline]
    pub fn get_side_to_move(&self) -> Color {
        NSHOGI_STATE_API.get_side_to_move(self.handle.as_ptr())
    }

    /// Returns the ply of the state.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert_eq!(0, state.get_ply());
    /// ```
    #[inline]
    pub fn get_ply(&self) -> i32 {
        NSHOGI_STATE_API.get_ply(self.handle.as_ptr())
    }

    /// Returns the current position.
    pub fn get_position(&self) -> Position<'_> {
        Position::new(NSHOGI_STATE_API.get_position(self.handle.as_ptr()))
    }

    /// Returns the initial position.
    pub fn get_initial_position(&self) -> Position<'_> {
        Position::new(NSHOGI_STATE_API.get_initial_position(self.handle.as_ptr()))
    }

    /// Returns the repetition status of the state.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert_eq!(nshogi::types::Repetition::NO_REPETITION, state.get_repetition());
    /// ```
    #[inline]
    pub fn get_repetition(&self) -> Repetition {
        NSHOGI_STATE_API.get_repetition(self.handle.as_ptr())
    }

    /// Returns `true` if the player of the side to move
    /// can declare at the state.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert!(!state.can_declare());
    /// ```
    #[inline]
    pub fn can_declare(&self) -> bool {
        NSHOGI_STATE_API.can_declare(self.handle.as_ptr())
    }

    /// Returns a history ply specified by the `ply` argument.
    /// If `ply` is invalid, returns Err(MoveGetError).
    ///
    /// ```
    /// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert!(state.get_history_move(0).is_err());
    /// let next_move = nshogi::types::Move::from_sfen(&state, "2g2f").unwrap();
    /// state.do_move(next_move);
    /// assert_eq!(next_move, state.get_history_move(0).unwrap());
    /// ```
    pub fn get_history_move(&self, ply: u16) -> Result<Move, MoveGetError> {
        NSHOGI_STATE_API.get_history_move(self.handle.as_ptr(), ply)
    }

    /// Returns the last ply.
    /// Returns Err(MoveGetError) at the initial state.
    ///
    /// ```
    /// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert!(state.get_last_move().is_err());
    /// let next_move = nshogi::types::Move::from_sfen(&state, "2g2f").unwrap();
    /// state.do_move(next_move);
    /// assert_eq!(next_move, state.get_last_move().unwrap());
    /// ```
    #[inline]
    pub fn get_last_move(&self) -> Result<Move, MoveGetError> {
        NSHOGI_STATE_API.get_last_move(self.handle.as_ptr())
    }

    /// Returns the hash value of the state.
    #[inline]
    pub fn get_hash(&self) -> u64 {
        NSHOGI_STATE_API.get_hash(self.handle.as_ptr())
    }

    /// Returns `true` if the king of the side to move is in check.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert!(!state.is_in_check());
    /// ```
    #[inline]
    pub fn is_in_check(&self) -> bool {
        NSHOGI_STATE_API.is_in_check(self.handle.as_ptr())
    }

    /// Generate legal moves wilily, which means some "inefficient" moves are filtered out,
    /// such as moving lance at the second rank without promotion.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert_eq!(30, state.generate_legal_moves().len());
    /// ```
    #[inline]
    pub fn generate_legal_moves(&self) -> Vec<Move> {
        NSHOGI_STATE_API.generate_moves(self.handle.as_ptr(), true)
    }

    /// Generate all legal moves.
    ///
    /// ```
    /// let state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert_eq!(30, state.generate_legal_moves_nowily().len());
    /// ```
    #[inline]
    pub fn generate_legal_moves_nowily(&self) -> Vec<Move> {
        NSHOGI_STATE_API.generate_moves(self.handle.as_ptr(), false)
    }

    /// Applies a move, which must be one of the legal moves.
    ///
    /// ```
    /// use nshogi::types::{Piece, Square};
    /// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert!(state.get_history_move(0).is_err());
    /// let next_move = nshogi::types::Move::from_sfen(&state, "2g2f").unwrap();
    /// state.do_move(next_move);
    /// assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_2G));
    /// assert_eq!(Piece::BLACK_PAWN, state.get_position().piece_on(Square::SQ_2F));
    /// ```
    #[inline]
    pub fn do_move(&mut self, m: Move) {
        NSHOGI_STATE_API.do_move(self.handle.as_ptr(), m)
    }

    /// Reverts the last move. Must not call this method at the initial state.
    ///
    /// ```
    /// use nshogi::types::{Piece, Square};
    /// let mut state = nshogi::state::State::from_sfen("startpos").unwrap();
    /// assert!(state.get_history_move(0).is_err());
    /// let next_move = nshogi::types::Move::from_sfen(&state, "2g2f").unwrap();
    /// state.do_move(next_move);
    /// state.undo_move();
    /// assert_eq!(Piece::BLACK_PAWN, state.get_position().piece_on(Square::SQ_2G));
    /// assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_2F));
    /// ```
    #[inline]
    pub fn undo_move(&mut self) {
        NSHOGI_STATE_API.undo_move(self.handle.as_ptr())
    }
}

impl Drop for State {
    fn drop(&mut self) {
        NSHOGI_STATE_API.destroy_state(self.handle.as_ptr());
    }
}

impl Clone for State {
    fn clone(&self) -> Self {
        Self {
            handle: NSHOGI_STATE_API.clone_state(self.handle.as_ptr()),
        }
    }
}

impl ToSfen for State {
    /// Returns the sfen notation of the state.
    ///
    /// ```
    /// use nshogi::io::ToSfen;
    /// let state = nshogi::state::State::from_sfen("startpos moves 2g2f").unwrap();
    /// assert_eq!(
    ///     "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f",
    ///     state.to_sfen()
    /// );
    /// ```
    fn to_sfen(&self) -> String {
        NSHOGI_IO_API.state_to_sfen(self.handle.as_ptr())
    }
}

impl ToCSA for State {
    /// Returns the csa notation of the state.
    ///
    /// ```
    /// use nshogi::io::ToCSA;
    /// let state = nshogi::state::State::from_sfen("startpos moves 2g2f").unwrap();
    /// assert_eq!(
    ///     concat!(
    ///         "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n",
    ///         "P2 * -HI *  *  *  *  * -KA * \n",
    ///         "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n",
    ///         "P4 *  *  *  *  *  *  *  *  * \n",
    ///         "P5 *  *  *  *  *  *  *  *  * \n",
    ///         "P6 *  *  *  *  *  *  *  *  * \n",
    ///         "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n",
    ///         "P8 * +KA *  *  *  *  * +HI * \n",
    ///         "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n",
    ///         "+\n",
    ///         "+2726FU\n"
    ///     ),
    ///     state.to_csa()
    /// );
    /// ```
    fn to_csa(&self) -> String {
        NSHOGI_IO_API.state_to_csa(self.handle.as_ptr())
    }
}

#[test]
fn get_initial_state() {
    let result = State::from_sfen("startpos");
    assert!(result.is_ok());
}

#[test]
fn move_from_sfen() {
    use crate::types::{Piece, Square};

    let mut state = State::from_sfen("startpos").unwrap();
    let m = Move::from_sfen(&state, "2g2f").unwrap();

    // Check do_move is completed successfully.
    state.do_move(m);

    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_2F)
    );
}

#[test]
fn initial_state_sfen_moves() {
    let state = State::from_sfen("startpos").unwrap();

    let moves = state.generate_legal_moves();

    let sfen_moves = vec![
        "1g1f", "2g2f", "3g3f", "4g4f", "5g5f", "6g6f", "7g7f", "8g8f", "9g9f", "1i1h", "2h1h",
        "2h3h", "2h4h", "2h5h", "2h6h", "2h7h", "3i3h", "3i4h", "4i3h", "4i4h", "4i5h", "5i4h",
        "5i5h", "5i6h", "6i5h", "6i6h", "6i7h", "7i6h", "7i7h", "9i9h",
    ]
    .into_iter()
    .map(|sfen| {
        let result = Move::from_sfen(&state, sfen);
        assert!(result.is_ok());
        result.unwrap()
    });

    for m in sfen_moves {
        assert!(moves.contains(&m));
    }
}

#[test]
fn initial_position() {
    use crate::types::{Piece, Square};

    let state = State::from_sfen("startpos").unwrap();

    assert_eq!(Color::BLACK, state.get_position().get_side_to_move());

    assert_eq!(30, state.generate_legal_moves().len());

    assert_eq!(
        Piece::WHITE_LANCE,
        state.get_position().piece_on(Square::SQ_9A)
    );
    assert_eq!(
        Piece::WHITE_KNIGHT,
        state.get_position().piece_on(Square::SQ_8A)
    );
    assert_eq!(
        Piece::WHITE_SILVER,
        state.get_position().piece_on(Square::SQ_7A)
    );
    assert_eq!(
        Piece::WHITE_GOLD,
        state.get_position().piece_on(Square::SQ_6A)
    );
    assert_eq!(
        Piece::WHITE_KING,
        state.get_position().piece_on(Square::SQ_5A)
    );
    assert_eq!(
        Piece::WHITE_GOLD,
        state.get_position().piece_on(Square::SQ_4A)
    );
    assert_eq!(
        Piece::WHITE_SILVER,
        state.get_position().piece_on(Square::SQ_3A)
    );
    assert_eq!(
        Piece::WHITE_KNIGHT,
        state.get_position().piece_on(Square::SQ_2A)
    );
    assert_eq!(
        Piece::WHITE_LANCE,
        state.get_position().piece_on(Square::SQ_1A)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_9B));
    assert_eq!(
        Piece::WHITE_ROOK,
        state.get_position().piece_on(Square::SQ_8B)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_7B));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_6B));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_5B));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_4B));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_3B));
    assert_eq!(
        Piece::WHITE_BISHOP,
        state.get_position().piece_on(Square::SQ_2B)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_1B));
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_9C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_8C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_7C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_6C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_5C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_4C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_3C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_2C)
    );
    assert_eq!(
        Piece::WHITE_PAWN,
        state.get_position().piece_on(Square::SQ_1C)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_9D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_8D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_7D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_6D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_5D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_4D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_3D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_2D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_1D));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_9E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_8E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_7E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_6E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_5E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_4E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_3E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_2E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_1E));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_9F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_8F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_7F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_6F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_5F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_4F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_3F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_2F));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_1F));
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_9G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_8G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_7G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_6G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_5G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_4G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_3G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_2G)
    );
    assert_eq!(
        Piece::BLACK_PAWN,
        state.get_position().piece_on(Square::SQ_1G)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_9H));
    assert_eq!(
        Piece::BLACK_BISHOP,
        state.get_position().piece_on(Square::SQ_8H)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_7H));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_6H));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_5H));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_4H));
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_3H));
    assert_eq!(
        Piece::BLACK_ROOK,
        state.get_position().piece_on(Square::SQ_2H)
    );
    assert_eq!(Piece::EMPTY, state.get_position().piece_on(Square::SQ_1H));
    assert_eq!(
        Piece::BLACK_LANCE,
        state.get_position().piece_on(Square::SQ_9I)
    );
    assert_eq!(
        Piece::BLACK_KNIGHT,
        state.get_position().piece_on(Square::SQ_8I)
    );
    assert_eq!(
        Piece::BLACK_SILVER,
        state.get_position().piece_on(Square::SQ_7I)
    );
    assert_eq!(
        Piece::BLACK_GOLD,
        state.get_position().piece_on(Square::SQ_6I)
    );
    assert_eq!(
        Piece::BLACK_KING,
        state.get_position().piece_on(Square::SQ_5I)
    );
    assert_eq!(
        Piece::BLACK_GOLD,
        state.get_position().piece_on(Square::SQ_4I)
    );
    assert_eq!(
        Piece::BLACK_SILVER,
        state.get_position().piece_on(Square::SQ_3I)
    );
    assert_eq!(
        Piece::BLACK_KNIGHT,
        state.get_position().piece_on(Square::SQ_2I)
    );
    assert_eq!(
        Piece::BLACK_LANCE,
        state.get_position().piece_on(Square::SQ_1I)
    );
}

#[test]
fn invalid_state() {
    let result = State::from_sfen("invalid");
    assert!(result.is_err());
}

#[test]
fn do_and_undo() {
    let result = State::from_sfen("startpos");
    assert!(result.is_ok());

    let mut state = result.unwrap();

    let moves = state.generate_legal_moves();

    for m in moves {
        // Check do and undo work correctly.
        state.do_move(m);
        state.undo_move();
    }
}

#[test]
fn perft() {
    fn brute_force(state: &mut State, limit: u32) -> u64 {
        if limit == 0 {
            1
        } else {
            let moves = state.generate_legal_moves_nowily();

            if limit == 1 {
                moves.len() as u64
            } else {
                let mut sum = 0u64;

                for m in moves {
                    state.do_move(m);
                    sum += brute_force(state, limit - 1);
                    state.undo_move();
                }

                sum
            }
        }
    }

    let mut state = State::from_sfen("startpos").unwrap();

    assert_eq!(30, brute_force(&mut state, 1));
    assert_eq!(900, brute_force(&mut state, 2));
    assert_eq!(25470, brute_force(&mut state, 3));
    assert_eq!(719731, brute_force(&mut state, 4));
    assert_eq!(19861490, brute_force(&mut state, 5));
}
