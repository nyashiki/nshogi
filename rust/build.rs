use std::collections::HashSet;
use std::env;

fn main() {
    let mut build = cc::Build::new();

    build.cpp(true)
        .files([
            "../src/buildinfo/capability.cc",
            "../src/c_api/api.cc",
            "../src/c_api/io.cc",
            "../src/c_api/ml.cc",
            "../src/c_api/move.cc",
            "../src/c_api/position.cc",
            "../src/c_api/solver.cc",
            "../src/c_api/state.cc",
            "../src/core/internal/bitboard.cc",
            "../src/core/internal/hash.cc",
            "../src/core/internal/huffmanimpl.cc",
            "../src/core/internal/movegenerator.cc",
            "../src/core/internal/statehelper.cc",
            "../src/core/internal/stateimpl.cc",
            "../src/core/huffman.cc",
            "../src/core/initializer.cc",
            "../src/core/movegenerator.cc",
            "../src/core/position.cc",
            "../src/core/positionbuilder.cc",
            "../src/core/state.cc",
            "../src/core/statebuilder.cc",
            "../src/io/csa.cc",
            "../src/io/file.cc",
            "../src/io/huffman.cc",
            "../src/io/sfen.cc",
            "../src/ml/featurebitboard.cc",
            "../src/ml/featurestack.cc",
            "../src/ml/simpleteacher.cc",
            "../src/ml/teacherloader.cc",
            "../src/ml/utils.cc",
            "../src/solver/internal/dfpn.cc",
            "../src/solver/internal/mate1ply.cc",
            "../src/solver/dfpn.cc",
            "../src/solver/dfs.cc",
            "../src/solver/mate1ply.cc",
            "../src/solver/dfpn.cc",
            ]);

    build.flag("-std=c++20");

    let arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap_or_default();
    let features = env::var("CARGO_CFG_TARGET_FEATURE").unwrap_or_default();
    let feats: HashSet<&str> = features.split(',').collect();

    if arch == "x86_64" {
        if feats.contains("sse2") {
            build.define("USE_SSE2", None)
                .flag_if_supported("-msse2");
        }
        if feats.contains("sse4.1") {
            build.define("USE_SSE41", None)
                .flag_if_supported("-msse4.1");
        }
        if feats.contains("sse4.2") {
            build.define("USE_SSE42", None)
                .flag_if_supported("-msse4.2");
        }
        if feats.contains("avx") {
            build.define("USE_AVX", None)
                .flag_if_supported("-mavx")
                .flag_if_supported("-mbmi")
                .flag_if_supported("-mbmi2");
        }
        if feats.contains("avx2") {
            build.define("USE_AVX2", None)
                .flag_if_supported("-mavx2")
                .flag_if_supported("-mlzcnt");
        }
    } else if arch == "aarch64" {
        if feats.contains("neon") {
            build.define("USE_NEON", None);
        }
    }

    build.compile("nshogi");
}
