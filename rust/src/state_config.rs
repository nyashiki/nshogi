use std::ffi::c_void;
use std::ptr::NonNull;

use crate::nshogi::NSHOGI_STATE_API;

/// Configuration for `State`: maximum ply history and draw scores.
pub struct StateConfig {
    pub(crate) handle: NonNull<c_void>
}

impl StateConfig {
    pub fn new() -> Self {
        Self {
            handle: NSHOGI_STATE_API.create_state_config()
        }
    }

    /// Returns the max ply.
    #[inline]
    pub fn get_max_ply(&self) -> u16 {
        NSHOGI_STATE_API.get_max_ply(self.handle.as_ptr())
    }

    /// Returns the draw value for the black player.
    #[inline]
    pub fn get_black_draw_value(&self) -> f32 {
        NSHOGI_STATE_API.get_black_draw_value(self.handle.as_ptr())
    }

    /// Returns the draw value for the white player.
    #[inline]
    pub fn get_white_draw_value(&self) -> f32 {
        NSHOGI_STATE_API.get_white_draw_value(self.handle.as_ptr())
    }

    /// Sets the max ply.
    pub fn set_max_ply(&mut self, value: u16) -> &mut Self {
        NSHOGI_STATE_API.set_max_ply(self.handle.as_ptr(), value);
        self
    }

    /// Sets the draw value for the black player.
    ///
    /// `value` must lie in the closed interval `[0.0, 1.0]`.
    pub fn set_black_draw_value(&mut self, value: f32) -> &mut Self {
        NSHOGI_STATE_API.set_black_draw_value(self.handle.as_ptr(), value);
        self
    }

    /// Sets the draw value for the white player.
    ///
    /// `value` must lie in the closed interval `[0.0, 1.0]`.
    pub fn set_white_draw_value(&mut self, value: f32) -> &mut Self {
        NSHOGI_STATE_API.set_white_draw_value(self.handle.as_ptr(), value);
        self
    }
}

impl Drop for StateConfig {
    fn drop(&mut self) {
        NSHOGI_STATE_API.destroy_state_config(self.handle.as_ptr())
    }
}

impl Clone for StateConfig {
    fn clone(&self) -> Self {
        let mut state_config = Self::new();
        state_config
            .set_max_ply(self.get_max_ply())
            .set_black_draw_value(self.get_black_draw_value())
            .set_white_draw_value(self.get_white_draw_value());
        state_config
    }
}

#[test]
fn test_state_config() {
    let mut state_config = StateConfig::new();

    for max_ply in 1..1024 {
        let mut black_draw_value = 0.0f32;
        while black_draw_value <= 1.0f32 {
            let mut white_draw_value = 0.0f32;
            while white_draw_value <= 1.0f32 {
                state_config
                    .set_max_ply(max_ply)
                    .set_black_draw_value(black_draw_value)
                    .set_white_draw_value(white_draw_value);

                assert_eq!(max_ply, state_config.get_max_ply());
                assert_eq!(black_draw_value, state_config.get_black_draw_value());
                assert_eq!(white_draw_value, state_config.get_white_draw_value());

                white_draw_value += 0.098f32;
            }
            black_draw_value += 0.12f32;
        }
    }
}
