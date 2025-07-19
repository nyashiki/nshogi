use std::ffi::c_void;
use std::ptr::NonNull;

use std::marker::PhantomData;

use crate::io::ToCSA;
use crate::io::ToSfen;
use crate::nshogi::NSHOGI_IO_API;
use crate::nshogi::NSHOGI_POSITION_API;
use crate::state::State;
use crate::types::{Color, Piece, PieceType, Square};

/// Represents the positions of pieces in shogi.
pub struct Position<'a> {
    pub(crate) handle: NonNull<c_void>,
    pub(crate) _borrows: PhantomData<&'a State>,
}

impl<'a> Position<'a> {
    pub(crate) fn new(handle: NonNull<c_void>) -> Self {
        Position {
            handle: handle,
            _borrows: PhantomData,
        }
    }

    /// Returns the side to move.
    #[inline]
    pub fn get_side_to_move(&self) -> Color {
        NSHOGI_POSITION_API.get_side_to_move(self.handle.as_ptr())
    }

    /// Returns the piece on `square`.
    #[inline]
    pub fn piece_on(&self, square: Square) -> Piece {
        NSHOGI_POSITION_API.piece_on(self.handle.as_ptr(), square.value() as i32)
    }

    /// Returns the count of `piece_type` on stand of `color`.
    #[inline]
    pub fn get_stand_count(&self, color: Color, piece_type: PieceType) -> u8 {
        NSHOGI_POSITION_API.get_stand_count(self.handle.as_ptr(), color, piece_type)
    }
}

impl<'a> ToSfen for Position<'a> {
    /// Returns the sfen notation of the state.
    ///
    /// ```
    /// use nshogi::io::ToSfen;
    /// let state = nshogi::state::State::from_sfen("startpos moves 2g2f").unwrap();
    /// assert_eq!(
    ///     "lnsgkgsnl/1r5b1/ppppppppp/9/9/7P1/PPPPPPP1P/1B5R1/LNSGKGSNL w - 1",
    ///     state.get_position().to_sfen()
    /// );
    /// ```
    fn to_sfen(&self) -> String {
        NSHOGI_IO_API.position_to_sfen(self.handle.as_ptr())
    }
}

impl<'a> ToCSA for Position<'a> {
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
    ///         "P6 *  *  *  *  *  *  * +FU * \n",
    ///         "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n",
    ///         "P8 * +KA *  *  *  *  * +HI * \n",
    ///         "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n",
    ///         "-\n",
    ///     ),
    ///     state.get_position().to_csa()
    /// );
    /// ```
    fn to_csa(&self) -> String {
        NSHOGI_IO_API.position_to_csa(self.handle.as_ptr())
    }
}
