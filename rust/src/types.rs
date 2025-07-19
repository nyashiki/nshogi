use crate::io::ToSfen;
use crate::nshogi::{NSHOGI_IO_API, NSHOGI_MOVE_API};
use crate::state::State;

/// Side (color) of a player in shogi.
///
/// Internally `0` stands for Black (Sente), `1` for White (Gote),
/// and `2` for none.
/// Any other value is undefined behaviour.
#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct Color(u8);

impl Color {
    pub const BLACK: Color = Color(0);
    pub const WHITE: Color = Color(1);
    pub const NO_COLOR: Color = Color(2);

    pub(crate) fn new(value: u8) -> Self {
        Self(value)
    }

    #[inline]
    pub fn value(&self) -> u8 {
        self.0
    }
}

/// Kind of a shogi piece **without** its color.
#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct PieceType(u8);

impl PieceType {
    pub const EMPTY: PieceType = PieceType(0);

    pub const PAWN: PieceType = PieceType(1);
    pub const LANCE: PieceType = PieceType(2);
    pub const KNIGHT: PieceType = PieceType(3);
    pub const SILVER: PieceType = PieceType(4);
    pub const BISHOP: PieceType = PieceType(5);
    pub const ROOK: PieceType = PieceType(6);
    pub const GOLD: PieceType = PieceType(7);
    pub const KING: PieceType = PieceType(8);
    pub const PRO_PAWN: PieceType = PieceType(9);
    pub const PRO_LANCE: PieceType = PieceType(10);
    pub const PRO_KNIGHT: PieceType = PieceType(11);
    pub const PRO_SILVER: PieceType = PieceType(12);
    pub const PRO_BISHOP: PieceType = PieceType(13);
    pub const PRO_ROOK: PieceType = PieceType(14);

    pub(crate) fn new(value: u8) -> Self {
        Self(value)
    }

    #[inline]
    pub fn value(&self) -> u8 {
        self.0
    }
}

/// Kind of a shogi piece **with** its color.
#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct Piece(u16);

impl Piece {
    pub const EMPTY: Piece = Piece(0);

    pub const BLACK_PAWN: Piece = Piece(1);
    pub const BLACK_LANCE: Piece = Piece(2);
    pub const BLACK_KNIGHT: Piece = Piece(3);
    pub const BLACK_SILVER: Piece = Piece(4);
    pub const BLACK_BISHOP: Piece = Piece(5);
    pub const BLACK_ROOK: Piece = Piece(6);
    pub const BLACK_GOLD: Piece = Piece(7);
    pub const BLACK_KING: Piece = Piece(8);
    pub const BLACK_PRO_PAWN: Piece = Piece(9);
    pub const BLACK_PRO_LANCE: Piece = Piece(10);
    pub const BLACK_PRO_KNIGHT: Piece = Piece(11);
    pub const BLACK_PRO_SILVER: Piece = Piece(12);
    pub const BLACK_PRO_BISHOP: Piece = Piece(13);
    pub const BLACK_PRO_ROOK: Piece = Piece(14);

    pub const WHITE_PAWN: Piece = Piece(17);
    pub const WHITE_LANCE: Piece = Piece(18);
    pub const WHITE_KNIGHT: Piece = Piece(19);
    pub const WHITE_SILVER: Piece = Piece(20);
    pub const WHITE_BISHOP: Piece = Piece(21);
    pub const WHITE_ROOK: Piece = Piece(22);
    pub const WHITE_GOLD: Piece = Piece(23);
    pub const WHITE_KING: Piece = Piece(24);
    pub const WHITE_PRO_PAWN: Piece = Piece(25);
    pub const WHITE_PRO_LANCE: Piece = Piece(26);
    pub const WHITE_PRO_KNIGHT: Piece = Piece(27);
    pub const WHITE_PRO_SILVER: Piece = Piece(28);
    pub const WHITE_PRO_BISHOP: Piece = Piece(29);
    pub const WHITE_PRO_ROOK: Piece = Piece(30);

    pub(crate) fn new(value: u16) -> Self {
        Self(value)
    }

    #[inline]
    pub fn value(&self) -> u16 {
        self.0
    }
}

/// Squares in shogi.
#[derive(Clone, Copy, Debug)]
pub struct Square(i8);

impl Square {
    pub const SQ_1I: Square = Square(0);
    pub const SQ_1H: Square = Square(1);
    pub const SQ_1G: Square = Square(2);
    pub const SQ_1F: Square = Square(3);
    pub const SQ_1E: Square = Square(4);
    pub const SQ_1D: Square = Square(5);
    pub const SQ_1C: Square = Square(6);
    pub const SQ_1B: Square = Square(7);
    pub const SQ_1A: Square = Square(8);
    pub const SQ_2I: Square = Square(9);
    pub const SQ_2H: Square = Square(10);
    pub const SQ_2G: Square = Square(11);
    pub const SQ_2F: Square = Square(12);
    pub const SQ_2E: Square = Square(13);
    pub const SQ_2D: Square = Square(14);
    pub const SQ_2C: Square = Square(15);
    pub const SQ_2B: Square = Square(16);
    pub const SQ_2A: Square = Square(17);
    pub const SQ_3I: Square = Square(18);
    pub const SQ_3H: Square = Square(19);
    pub const SQ_3G: Square = Square(20);
    pub const SQ_3F: Square = Square(21);
    pub const SQ_3E: Square = Square(22);
    pub const SQ_3D: Square = Square(23);
    pub const SQ_3C: Square = Square(24);
    pub const SQ_3B: Square = Square(25);
    pub const SQ_3A: Square = Square(26);
    pub const SQ_4I: Square = Square(27);
    pub const SQ_4H: Square = Square(28);
    pub const SQ_4G: Square = Square(29);
    pub const SQ_4F: Square = Square(30);
    pub const SQ_4E: Square = Square(31);
    pub const SQ_4D: Square = Square(32);
    pub const SQ_4C: Square = Square(33);
    pub const SQ_4B: Square = Square(34);
    pub const SQ_4A: Square = Square(35);
    pub const SQ_5I: Square = Square(36);
    pub const SQ_5H: Square = Square(37);
    pub const SQ_5G: Square = Square(38);
    pub const SQ_5F: Square = Square(39);
    pub const SQ_5E: Square = Square(40);
    pub const SQ_5D: Square = Square(41);
    pub const SQ_5C: Square = Square(42);
    pub const SQ_5B: Square = Square(43);
    pub const SQ_5A: Square = Square(44);
    pub const SQ_6I: Square = Square(45);
    pub const SQ_6H: Square = Square(46);
    pub const SQ_6G: Square = Square(47);
    pub const SQ_6F: Square = Square(48);
    pub const SQ_6E: Square = Square(49);
    pub const SQ_6D: Square = Square(50);
    pub const SQ_6C: Square = Square(51);
    pub const SQ_6B: Square = Square(52);
    pub const SQ_6A: Square = Square(53);
    pub const SQ_7I: Square = Square(54);
    pub const SQ_7H: Square = Square(55);
    pub const SQ_7G: Square = Square(56);
    pub const SQ_7F: Square = Square(57);
    pub const SQ_7E: Square = Square(58);
    pub const SQ_7D: Square = Square(59);
    pub const SQ_7C: Square = Square(60);
    pub const SQ_7B: Square = Square(61);
    pub const SQ_7A: Square = Square(62);
    pub const SQ_8I: Square = Square(63);
    pub const SQ_8H: Square = Square(64);
    pub const SQ_8G: Square = Square(65);
    pub const SQ_8F: Square = Square(66);
    pub const SQ_8E: Square = Square(67);
    pub const SQ_8D: Square = Square(68);
    pub const SQ_8C: Square = Square(69);
    pub const SQ_8B: Square = Square(70);
    pub const SQ_8A: Square = Square(71);
    pub const SQ_9I: Square = Square(72);
    pub const SQ_9H: Square = Square(73);
    pub const SQ_9G: Square = Square(74);
    pub const SQ_9F: Square = Square(75);
    pub const SQ_9E: Square = Square(76);
    pub const SQ_9D: Square = Square(77);
    pub const SQ_9C: Square = Square(78);
    pub const SQ_9B: Square = Square(79);
    pub const SQ_9A: Square = Square(80);

    pub const SQ_INVALID: Square = Square(82);

    pub(crate) fn new(value: i8) -> Self {
        Self(value)
    }

    #[inline]
    pub fn value(&self) -> i8 {
        self.0
    }

    pub fn all() -> Vec<Square> {
        (0i8..81i8).map(|i| Square(i)).collect()
    }
}

/// Result of a *sennichite* (repetition) decision in shogi.
#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct Repetition(u8);

impl Repetition {
    pub const NO_REPETITION: Repetition = Repetition(0);
    pub const REPETITION: Repetition = Repetition(1);
    pub const WIN_REPETITION: Repetition = Repetition(2);
    pub const LOSS_REPETITION: Repetition = Repetition(3);
    pub const SUPERIOR_REPETITION: Repetition = Repetition(4);
    pub const INFERIOR_REPETITION: Repetition = Repetition(5);

    pub(crate) fn new(value: u8) -> Self {
        Self(value)
    }

    pub fn value(&self) -> u8 {
        self.0
    }
}

#[derive(Debug)]
pub enum SfenParseError {
    RuntimeError(&'static str),
    InternalError(&'static str),
}

#[derive(Debug)]
pub enum CSAParseError {
    RuntimeError(&'static str),
    InternalError(&'static str),
}

#[derive(Debug)]
pub enum MoveGetError {
    OutOfIndex,
}

/// A single ply (*move*) in shogi.
#[derive(Copy, Clone, Debug)]
pub struct Move(u32);

impl Move {
    /// Creates a raw Move from a bit-field without validation (internal).
    pub(crate) fn new(value: u32) -> Self {
        Self(value)
    }

    /// Returns the underlying 32-bit code.
    #[inline]
    pub fn value(&self) -> u32 {
        self.0
    }

    /// Parses a sfen move string (e.g. "7g7f").
    pub fn from_sfen(state: &State, sfen: &str) -> Result<Self, SfenParseError> {
        NSHOGI_IO_API.create_move_from_sfen(state.handle.as_ptr(), sfen)
    }

    pub fn from_csa(state: &State, csa: &str) -> Result<Self, CSAParseError> {
        NSHOGI_IO_API.create_move_from_csa(state.handle.as_ptr(), csa)
    }

    /// Decodes the target square (0–80).
    #[inline]
    pub fn to(&self) -> Square {
        NSHOGI_MOVE_API.to(self.0)
    }

    /// Decodes the source square (0–80).
    #[inline]
    pub fn from(&self) -> Square {
        NSHOGI_MOVE_API.from(self.0)
    }

    /// `true` if the move drops a piece from stand.
    #[inline]
    pub fn is_dropping(&self) -> bool {
        NSHOGI_MOVE_API.is_dropping(self.0)
    }

    /// `true` if the move includes promotion.
    #[inline]
    pub fn is_promoting(&self) -> bool {
        NSHOGI_MOVE_API.is_promoting(self.0)
    }

    /// Piece being moved.
    #[inline]
    pub fn piece_type(&self) -> PieceType {
        NSHOGI_MOVE_API.piece_type(self.0)
    }

    /// Piece type captured on the to square.
    #[inline]
    pub fn capture_piece_type(&self) -> PieceType {
        NSHOGI_MOVE_API.capture_piece_type(self.0)
    }

    /// `true` if this value is none move.
    #[inline]
    pub fn is_none(&self) -> bool {
        NSHOGI_MOVE_API.is_none(self.0)
    }

    /// Returns the special win move for *declaration*.
    pub fn win() -> Move {
        NSHOGI_MOVE_API.win()
    }

    /// Returns the csa notattion of the state.
    ///
    /// This method is **not** provided by the `ToCSA` trait because
    /// converting a move to CSA notation requires access to the current
    /// `State`.  That context is **not** available inside the trait itself.
    pub fn to_csa(&self, state: &State) -> String {
        NSHOGI_IO_API.move_to_csa(state.handle.as_ptr(), self.value())
    }
}

impl PartialEq for Move {
    #[inline]
    fn eq(&self, other: &Move) -> bool {
        self.0 == other.0
    }

    #[inline]
    fn ne(&self, other: &Move) -> bool {
        self.0 != other.0
    }
}

impl ToSfen for Move {
    /// Returns the sfen notattion of the state.
    fn to_sfen(&self) -> String {
        NSHOGI_IO_API.move_to_sfen(self.value())
    }
}
