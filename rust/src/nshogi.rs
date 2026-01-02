use std::ffi::{CStr, CString, c_char, c_void};
use std::ptr::NonNull;
use std::sync::LazyLock;

use crate::types::{
    CSAParseError, Color, Move, MoveGetError, Piece, PieceType, Repetition, SfenParseError, Square,
};

// NShogi C APIs.

#[repr(C)]
#[derive(Clone)]
pub struct NShogiMoveCApi {
    to: unsafe extern "C" fn(u32) -> i8,
    from: unsafe extern "C" fn(u32) -> i8,
    drop: unsafe extern "C" fn(u32) -> i32,
    promote: unsafe extern "C" fn(u32) -> i32,
    piece_type: unsafe extern "C" fn(u32) -> u8,
    capture_piece_type: unsafe extern "C" fn(u32) -> u8,
    is_none: unsafe extern "C" fn(u32) -> i32,
    is_win: unsafe extern "C" fn(u32) -> i32,

    none_move: unsafe extern "C" fn() -> u32,
    win_move: unsafe extern "C" fn() -> u32,
}

impl NShogiMoveCApi {
    pub fn to(&self, m: u32) -> Square {
        unsafe { Square::new((self.to)(m)) }
    }

    pub fn from(&self, m: u32) -> Square {
        unsafe { Square::new((self.from)(m)) }
    }

    pub fn is_dropping(&self, m: u32) -> bool {
        unsafe { (self.drop)(m) != 0 }
    }

    pub fn is_promoting(&self, m: u32) -> bool {
        unsafe { (self.promote)(m) != 0 }
    }

    pub fn piece_type(&self, m: u32) -> PieceType {
        unsafe { PieceType::new((self.piece_type)(m)) }
    }

    pub fn capture_piece_type(&self, m: u32) -> PieceType {
        unsafe { PieceType::new((self.capture_piece_type)(m)) }
    }

    pub fn is_none(&self, m: u32) -> bool {
        unsafe { (self.is_none)(m) != 0 }
    }

    pub fn is_win(&self, m: u32) -> bool {
        unsafe { (self.is_win)(m) != 0 }
    }

    pub fn none(&self) -> Move {
        unsafe { Move::new((self.none_move)()) }
    }

    pub fn win(&self) -> Move {
        unsafe { Move::new((self.win_move)()) }
    }
}

#[repr(C)]
#[derive(Clone)]
pub struct NShogiPositionCApi {
    get_side_to_move: unsafe extern "C" fn(*mut c_void) -> u8,
    piece_on: unsafe extern "C" fn(*mut c_void, i32) -> u16,
    get_stand_count: unsafe extern "C" fn(*mut c_void, i32, i32) -> u8,
}

impl NShogiPositionCApi {
    pub fn get_side_to_move(&self, handle: *mut c_void) -> Color {
        unsafe { Color::new((self.get_side_to_move)(handle)) }
    }

    pub fn piece_on(&self, handle: *mut c_void, square: i32) -> Piece {
        unsafe { Piece::new((self.piece_on)(handle, square)) }
    }

    pub fn get_stand_count(&self, handle: *mut c_void, color: Color, piece_type: PieceType) -> u8 {
        unsafe { (self.get_stand_count)(handle, color.value() as i32, piece_type.value() as i32) }
    }
}

#[repr(C)]
#[derive(Clone)]
pub struct NShogiStateCApi {
    destroy_state: unsafe extern "C" fn(*mut c_void) -> (),
    clone: unsafe extern "C" fn(*mut c_void) -> *mut c_void,

    get_side_to_move: unsafe extern "C" fn(*const c_void) -> i32,
    get_ply: unsafe extern "C" fn(*const c_void) -> i32,
    get_position: unsafe extern "C" fn(*const c_void) -> *mut c_void,
    get_initial_position: unsafe extern "C" fn(*const c_void) -> *mut c_void,
    get_repetition: unsafe extern "C" fn(*const c_void) -> i32,
    can_declare: unsafe extern "C" fn(*const c_void) -> i32,
    get_history_move: unsafe extern "C" fn(*const c_void, u16) -> u32,
    get_last_move: unsafe extern "C" fn(*const c_void) -> u32,
    get_hash: unsafe extern "C" fn(*const c_void) -> u64,
    is_in_check: unsafe extern "C" fn(*const c_void) -> i32,

    generate_moves: unsafe extern "C" fn(*const c_void, i32, *mut u32) -> i32,
    do_move: unsafe extern "C" fn(*mut c_void, u32),
    undo_move: unsafe extern "C" fn(*mut c_void),

    create_state_config: unsafe extern "C" fn() -> *mut c_void,
    destroy_state_config: unsafe extern "C" fn(*mut c_void),

    get_max_ply: unsafe extern "C" fn(*const c_void) -> u16,
    get_black_draw_value: unsafe extern "C" fn(*const c_void) -> f32,
    get_white_draw_value: unsafe extern "C" fn(*const c_void) -> f32,

    set_max_ply: unsafe extern "C" fn(*mut c_void, u16),
    set_black_draw_value: unsafe extern "C" fn(*mut c_void, f32),
    set_white_draw_value: unsafe extern "C" fn(*mut c_void, f32),
}

impl NShogiStateCApi {
    pub fn destroy_state(&self, handle: *mut c_void) {
        unsafe {
            (self.destroy_state)(handle);
        }
    }

    pub fn clone_state(&self, handle: *mut c_void) -> NonNull<c_void> {
        unsafe { NonNull::new((self.clone)(handle)).unwrap_unchecked() }
    }

    pub fn get_side_to_move(&self, handle: *const c_void) -> Color {
        unsafe { Color::new((self.get_side_to_move)(handle) as u8) }
    }

    pub fn get_ply(&self, handle: *const c_void) -> i32 {
        unsafe { (self.get_ply)(handle) }
    }

    pub fn get_position(&self, handle: *const c_void) -> NonNull<c_void> {
        unsafe { NonNull::new((self.get_position)(handle)).unwrap_unchecked() }
    }

    pub fn get_initial_position(&self, handle: *const c_void) -> NonNull<c_void> {
        unsafe { NonNull::new((self.get_initial_position)(handle)).unwrap_unchecked() }
    }

    pub fn get_repetition(&self, handle: *const c_void) -> Repetition {
        unsafe { Repetition::new((self.get_repetition)(handle) as u8) }
    }

    pub fn can_declare(&self, handle: *const c_void) -> bool {
        unsafe { (self.can_declare)(handle) != 0 }
    }

    pub fn get_history_move(&self, handle: *const c_void, ply: u16) -> Result<Move, MoveGetError> {
        unsafe {
            if (ply as i32) < self.get_ply(handle) {
                Ok(Move::new((self.get_history_move)(handle, ply)))
            } else {
                Err(MoveGetError::OutOfIndex)
            }
        }
    }

    pub fn get_last_move(&self, handle: *const c_void) -> Result<Move, MoveGetError> {
        unsafe {
            if self.get_ply(handle) > 0 {
                Ok(Move::new((self.get_last_move)(handle)))
            } else {
                Err(MoveGetError::OutOfIndex)
            }
        }
    }

    pub fn get_hash(&self, handle: *const c_void) -> u64 {
        unsafe { (self.get_hash)(handle) }
    }

    pub fn is_in_check(&self, handle: *const c_void) -> bool {
        unsafe { (self.is_in_check)(handle) != 0 }
    }

    pub fn generate_moves(&self, handle: *const c_void, wily_promote: bool) -> Vec<Move> {
        let mut buffer = [0u32; 600];

        let move_count =
            unsafe { (self.generate_moves)(handle, wily_promote as i32, buffer.as_mut_ptr()) }
                as usize;

        buffer[..move_count]
            .iter()
            .copied()
            .map(|value| Move::new(value))
            .collect()
    }

    pub fn do_move(&self, handle: *mut c_void, m: Move) {
        unsafe { (self.do_move)(handle, m.value()) }
    }

    pub fn undo_move(&self, handle: *mut c_void) {
        unsafe { (self.undo_move)(handle) }
    }

    pub fn create_state_config(&self) -> NonNull<c_void> {
        unsafe { NonNull::new((self.create_state_config)()).unwrap_unchecked() }
    }

    pub fn destroy_state_config(&self, state_config: *mut c_void) {
        unsafe { (self.destroy_state_config)(state_config) }
    }

    pub fn get_max_ply(&self, state_config: *const c_void) -> u16 {
        unsafe { (self.get_max_ply)(state_config) }
    }

    pub fn get_black_draw_value(&self, state_config: *const c_void) -> f32 {
        unsafe { (self.get_black_draw_value)(state_config) }
    }

    pub fn get_white_draw_value(&self, state_config: *const c_void) -> f32 {
        unsafe { (self.get_white_draw_value)(state_config) }
    }

    pub fn set_max_ply(&self, state_config: *mut c_void, max_ply: u16) {
        unsafe { (self.set_max_ply)(state_config, max_ply) }
    }

    pub fn set_black_draw_value(&self, state_config: *mut c_void, value: f32) {
        unsafe { (self.set_black_draw_value)(state_config, value) }
    }

    pub fn set_white_draw_value(&self, state_config: *mut c_void, value: f32) {
        unsafe { (self.set_white_draw_value)(state_config, value) }
    }
}

#[repr(C)]
pub struct NShogiSolverCApi {
    dfs: unsafe extern "C" fn(*mut c_void, i32) -> u32,
    create_dfpn_solver: unsafe extern "C" fn(memory_mb: u64) -> *mut c_void,
    destroy_dfpn_solver: unsafe extern "C" fn(solver: *mut c_void),
    solve_by_dfpn: unsafe extern "C" fn(
        state: *mut c_void,
        solver: *mut c_void,
        max_node_count: u64,
        max_depth: i32,
        strict: i32,
    ) -> u32,
    solve_with_pv_by_dfpn: unsafe extern "C" fn(
        state: *mut c_void,
        solver: *mut c_void,
        max_node_count: u64,
        max_depth: i32,
        strict: i32,
        pv_buffer: *mut u32,
        pv_buffer_size: u32,
    ) -> i32,
}

impl NShogiSolverCApi {
    pub fn solve_by_dfs(&self, state: *mut c_void, depth: i32) -> Move {
        unsafe { Move::new((self.dfs)(state, depth)) }
    }

    pub fn create_dfpn_solver(&self, memory_mb: usize) -> NonNull<c_void> {
        unsafe { NonNull::new((self.create_dfpn_solver)(memory_mb as u64)).unwrap() }
    }

    pub fn destroy_dfpn_solver(&self, handle: *mut c_void) {
        unsafe { (self.destroy_dfpn_solver)(handle) }
    }

    pub fn solve_by_dfpn(
        &self,
        handle: *mut c_void,
        state: *mut c_void,
        max_node_count: u64,
        max_depth: i32,
        strict: bool,
    ) -> Move {
        unsafe {
            Move::new((self.solve_by_dfpn)(
                state,
                handle,
                max_node_count,
                max_depth,
                strict as i32,
            ))
        }
    }

    pub fn solve_with_pv_by_dfpn(
        &self,
        handle: *mut c_void,
        state: *mut c_void,
        max_node_count: u64,
        max_depth: i32,
        strict: bool,
    ) -> Vec<Move> {
        let mut pv_buffer = [0u32; 1024];

        let pv_length = unsafe {
            (self.solve_with_pv_by_dfpn)(
                state,
                handle,
                max_node_count,
                max_depth,
                strict as i32,
                pv_buffer.as_mut_ptr(),
                pv_buffer.len() as u32,
            )
        } as usize;

        pv_buffer[..pv_length]
            .iter()
            .copied()
            .map(|value| Move::new(value))
            .collect()
    }
}

#[repr(C)]
pub struct NShogiMLCApi {
    make_feature_vector: unsafe extern "C" fn(
        *mut std::ffi::c_float,
        *const std::ffi::c_void,
        *const std::ffi::c_void,
        *const std::ffi::c_int,
        i32,
    ) -> (),

    make_feature_vector_with_option: unsafe extern "C" fn(
        *mut std::ffi::c_float,
        *const std::ffi::c_void,
        *const std::ffi::c_void,
        *const std::ffi::c_int,
        i32,
        i32,
    ) -> (),

    move_to_index: unsafe extern "C" fn(*const c_void, u32) -> i32,
    move_to_index_with_option: unsafe extern "C" fn(*const c_void, u32, i32) -> i32,
}

impl NShogiMLCApi {
    pub fn make_feature_vector_with_option(
        &self,
        state: *const c_void,
        state_config: *const c_void,
        feature_types: *const i32,
        num_features: usize,
        channels_first: bool,
    ) -> Vec<f32> {
        let mut buffer: Vec<f32> = Vec::with_capacity(num_features * 9 * 9);

        unsafe {
            buffer.set_len(num_features * 9 * 9);

            (self.make_feature_vector_with_option)(
                buffer.as_mut_ptr(),
                state,
                state_config,
                feature_types,
                num_features as i32,
                channels_first as i32,
            );
        }

        buffer
    }

    pub fn move_to_index_with_option(&self, state: *const c_void, m: u32, channels_first: bool) -> usize {
        unsafe { (self.move_to_index_with_option)(state, m, channels_first as i32) as usize }
    }
}

#[repr(C)]
pub struct NShogiIOCApi {
    // Sfen.
    can_create_state_from_sfen: unsafe extern "C" fn(*const c_char) -> i32,
    can_create_move_from_sfen: unsafe extern "C" fn(*const c_void, *const c_char) -> i32,
    create_state_from_sfen: unsafe extern "C" fn(*const c_char) -> *mut c_void,
    create_move_from_sfen: unsafe extern "C" fn(*const c_void, *const c_char) -> u32,
    move_to_sfen: unsafe extern "C" fn(*mut c_char, u32) -> i32,
    state_to_sfen: unsafe extern "C" fn(*mut c_char, i32, *const c_void) -> i32,
    position_to_sfen: unsafe extern "C" fn(*mut c_char, i32, *const c_void) -> i32,

    // CSA.
    can_create_state_from_csa: unsafe extern "C" fn(*const c_char) -> i32,
    can_create_move_from_csa: unsafe extern "C" fn(*const c_void, *const c_char) -> i32,
    create_state_from_csa: unsafe extern "C" fn(*const c_char) -> *mut c_void,
    create_move_from_csa: unsafe extern "C" fn(*const c_void, *const c_char) -> u32,
    move_to_csa: unsafe extern "C" fn(*mut c_char, *const c_void, u32) -> i32,
    state_to_csa: unsafe extern "C" fn(*mut c_char, i32, *const c_void) -> i32,
    position_to_csa: unsafe extern "C" fn(*mut c_char, i32, *const c_void) -> i32,
}

impl NShogiIOCApi {
    pub fn create_state_from_sfen(&self, sfen: &str) -> Result<NonNull<c_void>, SfenParseError> {
        unsafe {
            let c_sfen = CString::new(sfen).expect("sfen string contains null byte");
            if (self.can_create_state_from_sfen)(c_sfen.as_ptr()) == 0 {
                Err(SfenParseError::RuntimeError("Invalid sfen string."))
            } else {
                let c_state = (self.create_state_from_sfen)(c_sfen.as_ptr());
                NonNull::new(c_state).ok_or(SfenParseError::InternalError(
                    "create_state_from_sfen returned null",
                ))
            }
        }
    }

    pub fn create_move_from_sfen(
        &self,
        state: *const c_void,
        sfen: &str,
    ) -> Result<Move, SfenParseError> {
        unsafe {
            let c_sfen = CString::new(sfen).expect("sfen string contains null byte");
            if (self.can_create_move_from_sfen)(state, c_sfen.as_ptr()) == 0 {
                Err(SfenParseError::RuntimeError("Invalid sfen string."))
            } else {
                Ok(Move::new((self.create_move_from_sfen)(
                    state,
                    c_sfen.as_ptr(),
                )))
            }
        }
    }

    pub fn move_to_sfen(&self, m: u32) -> String {
        const BUFFER_SIZE: usize = 16;
        let mut buffer = [0i8; BUFFER_SIZE];

        unsafe {
            let _ = (self.move_to_sfen)(buffer.as_mut_ptr(), m);
            let sfen = CStr::from_ptr(buffer.as_ptr()).to_str();
            sfen.unwrap().to_string()
        }
    }

    pub fn state_to_sfen(&self, c_state: *const c_void) -> String {
        const BUFFER_SIZE: usize = 16384;
        let mut buffer = [0i8; BUFFER_SIZE];

        unsafe {
            let _ = (self.state_to_sfen)(buffer.as_mut_ptr(), BUFFER_SIZE as i32, c_state);
            let sfen = CStr::from_ptr(buffer.as_ptr()).to_str();
            sfen.unwrap().to_string()
        }
    }

    pub fn position_to_sfen(&self, c_position: *const c_void) -> String {
        const BUFFER_SIZE: usize = 16384;
        let mut buffer = [0i8; BUFFER_SIZE];

        unsafe {
            let _ = (self.position_to_sfen)(buffer.as_mut_ptr(), BUFFER_SIZE as i32, c_position);
            let sfen = CStr::from_ptr(buffer.as_ptr()).to_str();
            sfen.unwrap().to_string()
        }
    }

    pub fn create_state_from_csa(&self, csa: &str) -> Result<NonNull<c_void>, CSAParseError> {
        unsafe {
            let c_csa = CString::new(csa).expect("CSA string contains null byte");
            if (self.can_create_state_from_csa)(c_csa.as_ptr()) == 0 {
                Err(CSAParseError::RuntimeError("Invalid CSA string."))
            } else {
                let c_state = (self.create_state_from_csa)(c_csa.as_ptr());
                NonNull::new(c_state).ok_or(CSAParseError::InternalError(
                        "create_state_from_csa returned null",
                ))
            }
        }
    }

    pub fn create_move_from_csa(&self, state: *const c_void, csa: &str) -> Result<Move, CSAParseError> {
        unsafe {
            let c_csa = CString::new(csa).expect("csa string contains null byte");
            if (self.can_create_move_from_sfen)(state, c_csa.as_ptr()) == 0 {
                Err(CSAParseError::RuntimeError("Invalid csa string."))
            } else {
                Ok(Move::new((self.create_move_from_csa)(
                            state,
                            c_csa.as_ptr(),
                )))
            }
        }
    }

    pub fn move_to_csa(&self, state: *const c_void, m: u32) -> String {
        const BUFFER_SIZE: usize = 16;
        let mut buffer = [0i8; BUFFER_SIZE];

        unsafe {
            let _ = (self.move_to_csa)(buffer.as_mut_ptr(), state, m);
            let csa = CStr::from_ptr(buffer.as_ptr()).to_str();
            csa.unwrap().to_string()
        }
    }

    pub fn state_to_csa(&self, state: *const c_void) -> String {
        const BUFFER_SIZE: usize = 16384;
        let mut buffer = [0i8; BUFFER_SIZE];

        unsafe {
            let _ = (self.state_to_csa)(buffer.as_mut_ptr(), BUFFER_SIZE as i32, state);
            let csa = CStr::from_ptr(buffer.as_ptr()).to_str();
            csa.unwrap().to_string()
        }
    }

    pub fn position_to_csa(&self, position: *const c_void) -> String {
        const BUFFER_SIZE: usize = 16384;
        let mut buffer = [0i8; BUFFER_SIZE];

        unsafe {
            let _ = (self.position_to_csa)(buffer.as_mut_ptr(), BUFFER_SIZE as i32, position);
            let csa = CStr::from_ptr(buffer.as_ptr()).to_str();
            csa.unwrap().to_string()
        }
    }
}

#[repr(C)]
pub struct NShogiCApi {
    move_api: unsafe extern "C" fn() -> *const NShogiMoveCApi,
    position_api: unsafe extern "C" fn() -> *const NShogiPositionCApi,
    state_api: unsafe extern "C" fn() -> *const NShogiStateCApi,
    solver_api: unsafe extern "C" fn() -> *const NShogiSolverCApi,
    ml_api: unsafe extern "C" fn() -> *const NShogiMLCApi,
    io_api: unsafe extern "C" fn() -> *const NShogiIOCApi,
}

// Entry point.
unsafe extern "C" {
    fn nshogiApi() -> *const NShogiCApi;
}

pub(crate) static NSHOGI_API: LazyLock<&'static NShogiCApi> =
    LazyLock::new(|| unsafe { &*nshogiApi() });
pub(crate) static NSHOGI_MOVE_API: LazyLock<&'static NShogiMoveCApi> =
    LazyLock::new(|| unsafe { &*(NSHOGI_API.move_api)() });
pub(crate) static NSHOGI_POSITION_API: LazyLock<&'static NShogiPositionCApi> =
    LazyLock::new(|| unsafe { &*(NSHOGI_API.position_api)() });
pub(crate) static NSHOGI_STATE_API: LazyLock<&'static NShogiStateCApi> =
    LazyLock::new(|| unsafe { &*(NSHOGI_API.state_api)() });
pub(crate) static NSHOGI_SOLVER_API: LazyLock<&'static NShogiSolverCApi> =
    LazyLock::new(|| unsafe { &*(NSHOGI_API.solver_api)() });
pub(crate) static NSHOGI_ML_API: LazyLock<&'static NShogiMLCApi> =
    LazyLock::new(|| unsafe { &*(NSHOGI_API.ml_api)() });
pub(crate) static NSHOGI_IO_API: LazyLock<&'static NShogiIOCApi> =
    LazyLock::new(|| unsafe { &*(NSHOGI_API.io_api)() });
