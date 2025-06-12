use crate::state::State;
use crate::state_config::StateConfig;
use crate::types::Move;
use crate::nshogi::NSHOGI_ML_API;

/// Discrete feature IDs used when converting a `State` into
/// numerical (float) representation.
#[repr(transparent)]
#[derive(PartialEq, Clone, Copy)]
pub struct FeatureType(i32);

impl FeatureType {
    /// Side-to-move plane: **1.0** on every square when Black is to move.
    /// Otherwise the plane is filled with **0.0**.
    pub const BLACK: FeatureType = FeatureType(0);
    /// Side-to-move plane: **1.0** on every square when White is to move.
    /// Otherwise the plane is filled with **0.0**.
    pub const WHITE: FeatureType = FeatureType(1);

    /// Piece plane: **1.0** on every square that contains a pawn
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PAWN: FeatureType = FeatureType(2);
    /// Piece plane: **1.0** on every square that contains a lance
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_LANCE: FeatureType = FeatureType(3);
    /// Piece plane: **1.0** on every square that contains a knight
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_KNIGHT: FeatureType = FeatureType(4);
    /// Piece plane: **1.0** on every square that contains a silver
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_SILVER: FeatureType = FeatureType(5);
    /// Piece plane: **1.0** on every square that contains a gold
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_GOLD: FeatureType = FeatureType(6);
    /// Piece plane: **1.0** on every square that contains a king
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_KING: FeatureType = FeatureType(7);
    /// Piece plane: **1.0** on every square that contains a bishop
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_BISHOP: FeatureType = FeatureType(8);
    /// Piece plane: **1.0** on every square that contains a rook
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_ROOK: FeatureType = FeatureType(9);
    /// Piece plane: **1.0** on every square that contains a promoted pawn
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PRO_PAWN: FeatureType = FeatureType(10);
    /// Piece plane: **1.0** on every square that contains a promoted lance
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PRO_LANCE: FeatureType = FeatureType(11);
    /// Piece plane: **1.0** on every square that contains a promoted knight
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PRO_KNIGHT: FeatureType = FeatureType(12);
    /// Piece plane: **1.0** on every square that contains a promoted silver
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PRO_SILVER: FeatureType = FeatureType(13);
    /// Piece plane: **1.0** on every square that contains a promoted bishop
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PRO_BISHOP: FeatureType = FeatureType(14);
    /// Piece plane: **1.0** on every square that contains a promoted rook
    /// belonging to the side to move, **0.0** elsewhere.
    pub const MY_PRO_ROOK: FeatureType = FeatureType(15);
    /// Piece plane: **1.0** on every square that contains a bishop or
    /// a promoted bishop belonging to the side to move, **0.0** elsewhere.
    pub const MY_BISHOP_AND_PRO_BISHOP: FeatureType = FeatureType(16);
    /// Piece plane: **1.0** on every square that contains a rook or
    /// a promoted rook belonging to the side to move, **0.0** elsewhere.
    pub const MY_ROOK_AND_PRO_ROOK: FeatureType = FeatureType(17);

    /// Piece plane: **1.0** on every square that contains a pawn
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PAWN: FeatureType = FeatureType(18);
    /// Piece plane: **1.0** on every square that contains a lance
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_LANCE: FeatureType = FeatureType(19);
    /// Piece plane: **1.0** on every square that contains a knight
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_KNIGHT: FeatureType = FeatureType(20);
    /// Piece plane: **1.0** on every square that contains a silver
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_SILVER: FeatureType = FeatureType(21);
    /// Piece plane: **1.0** on every square that contains a gold
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_GOLD: FeatureType = FeatureType(22);
    /// Piece plane: **1.0** on every square that contains a king
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_KING: FeatureType = FeatureType(23);
    /// Piece plane: **1.0** on every square that contains a bishop
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_BISHOP: FeatureType = FeatureType(24);
    /// Piece plane: **1.0** on every square that contains a rook
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_ROOK: FeatureType = FeatureType(25);
    /// Piece plane: **1.0** on every square that contains a promoted pawn
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PRO_PAWN: FeatureType = FeatureType(26);
    /// Piece plane: **1.0** on every square that contains a promoted lance
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PRO_LANCE: FeatureType = FeatureType(27);
    /// Piece plane: **1.0** on every square that contains a promoted knight
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PRO_KNIGHT: FeatureType = FeatureType(28);
    /// Piece plane: **1.0** on every square that contains a promoted silver
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PRO_SILVER: FeatureType = FeatureType(29);
    /// Piece plane: **1.0** on every square that contains a promoted bishop
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PRO_BISHOP: FeatureType = FeatureType(30);
    /// Piece plane: **1.0** on every square that contains a promoted rook
    /// belonging to the side not to move, **0.0** elsewhere.
    pub const OP_PRO_ROOK: FeatureType = FeatureType(31);
    /// Piece plane: **1.0** on every square that contains a bishop or
    /// a promoted bishop belonging to the side not to move, **0.0** elsewhere.
    pub const OP_BISHOP_AND_PRO_BISHOP: FeatureType = FeatureType(32);
    /// Piece plane: **1.0** on every square that contains a rook or
    /// a promoted rook belonging to the side not to move, **0.0** elsewhere.
    pub const OP_ROOK_AND_PRO_ROOK: FeatureType = FeatureType(33);

    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one pawn on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN1: FeatureType = FeatureType(34);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN2: FeatureType = FeatureType(35);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least three pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN3: FeatureType = FeatureType(36);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least four pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN4: FeatureType = FeatureType(37);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least five pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN5: FeatureType = FeatureType(38);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least six pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN6: FeatureType = FeatureType(39);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least seven pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN7: FeatureType = FeatureType(40);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least eight pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN8: FeatureType = FeatureType(41);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least nine pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_PAWN9: FeatureType = FeatureType(42);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one lance on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_LANCE1: FeatureType = FeatureType(43);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two lances on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_LANCE2: FeatureType = FeatureType(44);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least three lances on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_LANCE3: FeatureType = FeatureType(45);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least four lances on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_LANCE4: FeatureType = FeatureType(46);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one knight on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_KNIGHT1: FeatureType = FeatureType(47);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two knights on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_KNIGHT2: FeatureType = FeatureType(48);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least three knights on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_KNIGHT3: FeatureType = FeatureType(49);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least four knights on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_KNIGHT4: FeatureType = FeatureType(50);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one silver on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_SILVER1: FeatureType = FeatureType(51);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two silvers on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_SILVER2: FeatureType = FeatureType(52);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least three silvers on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_SILVER3: FeatureType = FeatureType(53);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least four silvers on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_SILVER4: FeatureType = FeatureType(54);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one gold on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_GOLD1: FeatureType = FeatureType(55);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two golds on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_GOLD2: FeatureType = FeatureType(56);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least three golds on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_GOLD3: FeatureType = FeatureType(57);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least four golds on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_GOLD4: FeatureType = FeatureType(58);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one bishop on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_BISHOP1: FeatureType = FeatureType(59);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two bishops on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_BISHOP2: FeatureType = FeatureType(60);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least one rook on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_ROOK1: FeatureType = FeatureType(61);
    /// Stand plane: **1.0** on every square when the side to move
    /// holds at least two rooks on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const MY_STAND_ROOK2: FeatureType = FeatureType(62);

    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one pawn on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN1: FeatureType = FeatureType(63);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN2: FeatureType = FeatureType(64);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least three pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN3: FeatureType = FeatureType(65);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least four pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN4: FeatureType = FeatureType(66);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least five pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN5: FeatureType = FeatureType(67);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least six pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN6: FeatureType = FeatureType(68);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least seven pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN7: FeatureType = FeatureType(69);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least eight pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN8: FeatureType = FeatureType(70);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least nine pawns on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_PAWN9: FeatureType = FeatureType(71);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one lance on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_LANCE1: FeatureType = FeatureType(72);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two lances on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_LANCE2: FeatureType = FeatureType(73);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least three lances on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_LANCE3: FeatureType = FeatureType(74);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least four lances on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_LANCE4: FeatureType = FeatureType(75);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one knight on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_KNIGHT1: FeatureType = FeatureType(76);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two knights on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_KNIGHT2: FeatureType = FeatureType(77);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least three knights on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_KNIGHT3: FeatureType = FeatureType(78);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least four knights on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_KNIGHT4: FeatureType = FeatureType(79);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one silver on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_SILVER1: FeatureType = FeatureType(80);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two silvers on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_SILVER2: FeatureType = FeatureType(81);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least three silvers on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_SILVER3: FeatureType = FeatureType(82);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least four silvers on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_SILVER4: FeatureType = FeatureType(83);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one gold on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_GOLD1: FeatureType = FeatureType(84);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two golds on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_GOLD2: FeatureType = FeatureType(85);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least three golds on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_GOLD3: FeatureType = FeatureType(86);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least four golds on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_GOLD4: FeatureType = FeatureType(87);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one bishop on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_BISHOP1: FeatureType = FeatureType(88);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two bishops on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_BISHOP2: FeatureType = FeatureType(89);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least one rook on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_ROOK1: FeatureType = FeatureType(90);
    /// Stand plane: **1.0** on every square when the side not to move
    /// holds at least two rooks on the stand.
    /// Otherwise the plane is filled with **0.0**.
    pub const OP_STAND_ROOK2: FeatureType = FeatureType(91);

    /// Check plane: **1.0** on every square when the king of
    /// the side to move is in check.
    /// Otherwise the plane is filled with **0.0**.
    pub const CHECK: FeatureType = FeatureType(92);
    /// Check plane: **1.0** on every file when no pawn of
    /// the side to move exists on the file.
    /// Otherwise the file is filled with **0.0**.
    pub const NO_MY_PAWN_FILE: FeatureType = FeatureType(93);
    /// Check plane: **1.0** on every file when no pawn of
    /// the side not to move exists on the file.
    /// Otherwise the file is filled with **0.0**.
    pub const NO_OP_PAWN_FILE: FeatureType = FeatureType(94);
    /// Progress plane: `State::get_ply() / StateConfig::get_max_ply()` on every square.
    pub const PROGRESS: FeatureType = FeatureType(95);
    /// Progress-unit plane: `1.0 / StateConfig::get_max_ply()` on every square.
    pub const PROGRESS_UNIT: FeatureType = FeatureType(96);

    /// Work in progress.
    pub const RULE_DECLARE27: FeatureType = FeatureType(97);
    /// Work in progress.
    pub const RULE_DRAW24: FeatureType = FeatureType(98);
    /// Work in progress.
    pub const RULE_TRYING: FeatureType = FeatureType(99);
    /// Draw-value plane: `StateConfig::get_black_draw_value()` on every square
    /// if the side to move is the black player.
    /// Otherwise the plane is filled with `StateConfig::get_white_draw_value()`.
    pub const MY_DRAW_VALUE: FeatureType = FeatureType(100);
    /// Draw-value plane: `StateConfig::get_white_draw_value()` on every square
    /// if the side to move is the black player.
    /// Otherwise the plane is filled with `StateConfig::get_black_draw_value()`.
    pub const OP_DRAW_VALUE: FeatureType = FeatureType(101);

    /// Declaration-score plane: the sum of my piece points under the
    /// declaration-win rule, divided by `28.0` if Black is to move or
    /// by `27.0` if White is to move.
    /// The resulting scalar is written to every square.
    pub const MY_DECLARATION_SCORE: FeatureType = FeatureType(102);
    /// Declaration-score plane: the sum of opponent piece points under the
    /// declaration-win rule, divided by `28.0` if Black is to move or
    /// by `27.0` if White is to move.
    /// The resulting scalar is written to every square.
    pub const OP_DECLARATION_SCORE: FeatureType = FeatureType(103);
    /// Declaration-score plane: the sum of my piece points under the
    /// declaration-win rule, divided by `54.0`.
    /// The resulting scalar is written to every square.
    pub const MY_PIECE_SCORE: FeatureType = FeatureType(104);
    /// Declaration-score plane: the sum of opponent piece points under the
    /// declaration-win rule, divided by `54.0`.
    /// The resulting scalar is written to every square.
    pub const OP_PIECE_SCORE: FeatureType = FeatureType(105);

    /// Attack plane: `1.0` on every square that is controlled by at least
    /// one piece of the side to move; `0.0` elsewhere.
    pub const MY_ATTACK: FeatureType = FeatureType(106);
    /// Attack plane: `1.0` on every square that is controlled by at least
    /// one piece of the side not to move; `0.0` elsewhere.
    pub const OP_ATTACK: FeatureType = FeatureType(107);

    pub fn value(&self) -> i32 {
        self.0
    }
}

/// Converts `State` and `StateConfig` into a contiguous feature vector of `f32`.
///
/// Each element of `feature_types` selects a 9x9 plane (see [`FeatureType`]);
/// all selected planes are concatenated in the order
/// given and returned as a single `Vec<f32>` of length `feature_types.len() x 81`.
///
/// # Parameters
///
/// * `state` - current game position to encode.
/// * `state_config` - configs about `state`.
/// * `feature_types` - ordered list of planes to extract.
///
/// # Return value
///
/// A vector of 32-bit floats.
///
/// ```
/// use nshogi::ml::{make_feature_vector, FeatureType};
///
/// let state = nshogi::state::State::from_sfen("startpos").unwrap();
/// let config = nshogi::state_config::StateConfig::new();
/// let v = make_feature_vector(&state, &config,
///     &vec![FeatureType::BLACK, FeatureType::MY_PAWN]);
///
/// assert_eq!(v.len(), 2 * 81); // 2 planes.
/// ```
///
/// ```
/// use nshogi::types::Piece;
/// use nshogi::types::Square;
/// use nshogi::ml::{make_feature_vector, FeatureType};
///
/// let state = nshogi::state::State::from_sfen("startpos").unwrap();
/// let config = nshogi::state_config::StateConfig::new();
/// let v = make_feature_vector(&state, &config,
///     &vec![FeatureType::MY_PAWN]);
///
/// for sq in Square::all() {
///     if state.get_position().piece_on(sq) == Piece::BLACK_PAWN {
///        // If pawn exists on `sq`, then v[`sq`] must be 1.0.
///         assert_eq!(1.0f32, v[sq.value() as usize]);
///     } else {
///        // If pawn does not exist on `sq`, then v[`sq`] must be 0.0.
///         assert_eq!(0.0f32, v[sq.value() as usize]);
///     }
/// }
/// ```
#[inline]
pub fn make_feature_vector(state: &State, state_config: &StateConfig, feature_types: &Vec<FeatureType>) -> Vec<f32> {
    NSHOGI_ML_API.make_feature_vector(
        state.handle.as_ptr(),
        state_config.handle.as_ptr(),
        feature_types.as_ptr() as *const i32,
        feature_types.len())
}

/// Maps a concrete `Move` in the given `state` to a 0-based integer
/// index, suitable for policy-head training or move tables.
///
/// The mapping is **state-dependent**:
/// * Within one state every legal move has a unique index.
/// * Across different positions the *same* numerical index may refer
///   to completely different moves.
#[inline]
pub fn move_to_index(state: &State, m: &Move) -> usize {
    NSHOGI_ML_API.move_to_index(state.handle.as_ptr(), m.value())
}

#[test]
pub fn make_feature_vector_color() {
    let state = State::from_sfen("startpos").unwrap();
    let state_config = StateConfig::new();
    let feature_types = vec![FeatureType::BLACK, FeatureType::WHITE];

    let fv = make_feature_vector(&state, &state_config, &feature_types);
    assert_eq!(feature_types.len() * 81, fv.len());
    assert!(fv[0..81].iter().all(|v| { *v == 1.0f32 }));
    assert!(fv[81..81*2].iter().all(|v| { *v == 0.0f32 }));
}

#[test]
pub fn make_feature_vector_piece() {
    use crate::types::Piece;
    use crate::types::Square;

    let piece_and_feature_type = vec![
        (Piece::BLACK_PAWN, FeatureType::MY_PAWN),
        (Piece::BLACK_LANCE, FeatureType::MY_LANCE),
        (Piece::BLACK_KNIGHT, FeatureType::MY_KNIGHT),
        (Piece::BLACK_SILVER, FeatureType::MY_SILVER),
        (Piece::BLACK_BISHOP, FeatureType::MY_BISHOP),
        (Piece::BLACK_ROOK, FeatureType::MY_ROOK),
        (Piece::BLACK_GOLD, FeatureType::MY_GOLD),
        (Piece::BLACK_KING, FeatureType::MY_KING),
        (Piece::BLACK_PRO_PAWN, FeatureType::MY_PRO_PAWN),
        (Piece::BLACK_PRO_LANCE, FeatureType::MY_PRO_LANCE),
        (Piece::BLACK_PRO_KNIGHT, FeatureType::MY_PRO_KNIGHT),
        (Piece::BLACK_PRO_SILVER, FeatureType::MY_PRO_SILVER),
        (Piece::BLACK_PRO_BISHOP, FeatureType::MY_PRO_BISHOP),
        (Piece::BLACK_PRO_ROOK, FeatureType::MY_PRO_ROOK),
        (Piece::WHITE_PAWN, FeatureType::OP_PAWN),
        (Piece::WHITE_LANCE, FeatureType::OP_LANCE),
        (Piece::WHITE_KNIGHT, FeatureType::OP_KNIGHT),
        (Piece::WHITE_SILVER, FeatureType::OP_SILVER),
        (Piece::WHITE_BISHOP, FeatureType::OP_BISHOP),
        (Piece::WHITE_ROOK, FeatureType::OP_ROOK),
        (Piece::WHITE_GOLD, FeatureType::OP_GOLD),
        (Piece::WHITE_KING, FeatureType::OP_KING),
        (Piece::WHITE_PRO_PAWN, FeatureType::OP_PRO_PAWN),
        (Piece::WHITE_PRO_LANCE, FeatureType::OP_PRO_LANCE),
        (Piece::WHITE_PRO_KNIGHT, FeatureType::OP_PRO_KNIGHT),
        (Piece::WHITE_PRO_SILVER, FeatureType::OP_PRO_SILVER),
        (Piece::WHITE_PRO_BISHOP, FeatureType::OP_PRO_BISHOP),
        (Piece::WHITE_PRO_ROOK, FeatureType::OP_PRO_ROOK),
    ];

    let state = State::from_sfen("kgsnlbrp1/+s+n+l+p5/5+P+L+N+S/1PRBLNSGK/9/9/9/9/9 b G7Pg7p 1").unwrap();
    let state_config = StateConfig::new();
    let feature_types = piece_and_feature_type.iter().map(|v| { v.1 }).collect();

    let fv = make_feature_vector(&state, &state_config, &feature_types);
    assert_eq!(feature_types.len() * 81, fv.len());

    for (i, v) in piece_and_feature_type.iter().enumerate() {
        for sq in Square::all() {
            if state.get_position().piece_on(sq) == v.0 {
                assert_eq!(1.0f32, fv[i * 81 + sq.value() as usize]);
            } else {
                assert_eq!(0.0f32, fv[i * 81 + sq.value() as usize]);
            }
        }
    }
}
