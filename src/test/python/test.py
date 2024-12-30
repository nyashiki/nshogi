import unittest
import numpy as np
import random

import nshogi

class TestState(unittest.TestCase):
    def setUp(self):
        self.sfens = []
        self.legal_moves = []
        with open("res/test/legal-move-examples.txt", "r") as f:
            while True:
                line = f.readline()

                if not line:
                    break

                legal_moves = set(f.readline().rstrip().split(" ")[1:])

                sfen = line.rstrip()
                state = nshogi.io.sfen.make_state_from_sfen(sfen)

                self.sfens.append(sfen)
                self.legal_moves.append(legal_moves)

    def test_get_sfen(self):
        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)
            self.assertEqual(sfen, state.to_sfen())

    def test_legal_moves(self):
        for (sfen, legal_moves) in zip(self.sfens, self.legal_moves):
            state = nshogi.io.sfen.make_state_from_sfen(sfen)
            moves_generated = [nshogi.io.sfen.stringify(m) for m in state.legal_moves]
            self.assertEqual(legal_moves, set(moves_generated))

    def test_clone(self):
        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)
            state_cloned = state.clone()
            self.assertEqual(state.to_sfen(), state_cloned.to_sfen())

    def test_do_move_undo_move(self):
        for i in range(10):
            state = nshogi.io.sfen.make_state_from_sfen("startpos")

            for ply in range(256):
                legal_moves = state.legal_moves

                if len(legal_moves) == 0:
                    break

                sfen = state.to_sfen()

                for m in legal_moves:
                    state.do_move(m)
                    state.undo_move()

                    self.assertEqual(sfen, state.to_sfen())

                state.do_move(random.choice(legal_moves))

class TestML(unittest.TestCase):
    def setUp(self):
        self.sfens = []
        self.legal_moves = []
        with open("res/test/legal-move-examples.txt", "r") as f:
            while True:
                line = f.readline()

                if not line:
                    break

                legal_moves = set(f.readline().rstrip().split(" ")[1:])

                sfen = line.rstrip()
                state = nshogi.io.sfen.make_state_from_sfen(sfen)

                self.sfens.append(sfen)
                self.legal_moves.append(legal_moves)

    def test_feature_black(self):
        feature_types = [
            nshogi.ml.FeatureType.BLACK
        ]

        state_config = nshogi.StateConfig()

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)

            x = nshogi.ml.FeatureStack(feature_types, state, state_config)
            x = x.to_numpy()

            if state.side_to_move == nshogi.Color.BLACK:
                self.assertTrue(all(x == 1))
            else:
                self.assertTrue(all(x == 0))

    def test_feature_white(self):
        feature_types = [
            nshogi.ml.FeatureType.WHITE
        ]

        state_config = nshogi.StateConfig()

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)

            x = nshogi.ml.FeatureStack(feature_types, state, state_config)
            x = x.to_numpy()

            if state.side_to_move == nshogi.Color.WHITE:
                self.assertTrue(all(x == 1))
            else:
                self.assertTrue(all(x == 0))


    def test_feature_pieces_on_board(self):
        features = [
            (nshogi.ml.FeatureType.MY_PAWN, nshogi.Piece.BLACK_PAWN, nshogi.Piece.WHITE_PAWN),
            (nshogi.ml.FeatureType.MY_LANCE, nshogi.Piece.BLACK_LANCE, nshogi.Piece.WHITE_LANCE),
            (nshogi.ml.FeatureType.MY_KNIGHT, nshogi.Piece.BLACK_KNIGHT, nshogi.Piece.WHITE_KNIGHT),
            (nshogi.ml.FeatureType.MY_SILVER, nshogi.Piece.BLACK_SILVER, nshogi.Piece.WHITE_SILVER),
            (nshogi.ml.FeatureType.MY_GOLD, nshogi.Piece.BLACK_GOLD, nshogi.Piece.WHITE_GOLD),
            (nshogi.ml.FeatureType.MY_KING, nshogi.Piece.BLACK_KING, nshogi.Piece.WHITE_KING),
            (nshogi.ml.FeatureType.MY_BISHOP, nshogi.Piece.BLACK_BISHOP, nshogi.Piece.WHITE_BISHOP),
            (nshogi.ml.FeatureType.MY_ROOK, nshogi.Piece.BLACK_ROOK, nshogi.Piece.WHITE_ROOK),
            (nshogi.ml.FeatureType.MY_PROPAWN, nshogi.Piece.BLACK_PRO_PAWN, nshogi.Piece.WHITE_PRO_PAWN),
            (nshogi.ml.FeatureType.MY_PROLANCE, nshogi.Piece.BLACK_PRO_LANCE, nshogi.Piece.WHITE_PRO_LANCE),
            (nshogi.ml.FeatureType.MY_PROKNIGHT, nshogi.Piece.BLACK_PRO_KNIGHT, nshogi.Piece.WHITE_PRO_KNIGHT),
            (nshogi.ml.FeatureType.MY_PROSILVER, nshogi.Piece.BLACK_PRO_SILVER, nshogi.Piece.WHITE_PRO_SILVER),
            (nshogi.ml.FeatureType.MY_PROBISHOP, nshogi.Piece.BLACK_PRO_BISHOP, nshogi.Piece.WHITE_PRO_BISHOP),
            (nshogi.ml.FeatureType.MY_PROROOK, nshogi.Piece.BLACK_PRO_ROOK, nshogi.Piece.WHITE_PRO_ROOK),
        ]

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)
            state_config = nshogi.StateConfig()

            for (feature_type, black_piece, white_piece) in features:
                feature_types = [
                    feature_type
                ]

                x = nshogi.ml.FeatureStack(feature_types, state, state_config)
                x = x.to_numpy()

                for i in range(81):
                    if state.side_to_move == nshogi.Color.BLACK:
                        if state.get_piece_on(i) == black_piece:
                            self.assertEqual(1.0, x[i])
                        else:
                            self.assertEqual(0.0, x[i])
                    else:
                        if state.get_piece_on(i) == white_piece:
                            self.assertEqual(1.0, x[80 - i])
                        else:
                            self.assertEqual(0.0, x[80 - i])

    def test_feature_pieces_my_stand(self):
        features = [
            (nshogi.ml.FeatureType.MY_STAND_PAWN_1, nshogi.PieceType.PAWN, 1),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_2, nshogi.PieceType.PAWN, 2),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_3, nshogi.PieceType.PAWN, 3),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_4, nshogi.PieceType.PAWN, 4),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_5, nshogi.PieceType.PAWN, 5),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_6, nshogi.PieceType.PAWN, 6),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_7, nshogi.PieceType.PAWN, 7),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_8, nshogi.PieceType.PAWN, 8),
            (nshogi.ml.FeatureType.MY_STAND_PAWN_9, nshogi.PieceType.PAWN, 9),
            (nshogi.ml.FeatureType.MY_STAND_LANCE_1, nshogi.PieceType.LANCE, 1),
            (nshogi.ml.FeatureType.MY_STAND_LANCE_2, nshogi.PieceType.LANCE, 2),
            (nshogi.ml.FeatureType.MY_STAND_LANCE_3, nshogi.PieceType.LANCE, 3),
            (nshogi.ml.FeatureType.MY_STAND_LANCE_4, nshogi.PieceType.LANCE, 4),
            (nshogi.ml.FeatureType.MY_STAND_KNIGHT_1, nshogi.PieceType.KNIGHT, 1),
            (nshogi.ml.FeatureType.MY_STAND_KNIGHT_2, nshogi.PieceType.KNIGHT, 2),
            (nshogi.ml.FeatureType.MY_STAND_KNIGHT_3, nshogi.PieceType.KNIGHT, 3),
            (nshogi.ml.FeatureType.MY_STAND_KNIGHT_4, nshogi.PieceType.KNIGHT, 4),
            (nshogi.ml.FeatureType.MY_STAND_SILVER_1, nshogi.PieceType.SILVER, 1),
            (nshogi.ml.FeatureType.MY_STAND_SILVER_2, nshogi.PieceType.SILVER, 2),
            (nshogi.ml.FeatureType.MY_STAND_SILVER_3, nshogi.PieceType.SILVER, 3),
            (nshogi.ml.FeatureType.MY_STAND_SILVER_4, nshogi.PieceType.SILVER, 4),
            (nshogi.ml.FeatureType.MY_STAND_GOLD_1, nshogi.PieceType.GOLD, 1),
            (nshogi.ml.FeatureType.MY_STAND_GOLD_2, nshogi.PieceType.GOLD, 2),
            (nshogi.ml.FeatureType.MY_STAND_GOLD_3, nshogi.PieceType.GOLD, 3),
            (nshogi.ml.FeatureType.MY_STAND_GOLD_4, nshogi.PieceType.GOLD, 4),
            (nshogi.ml.FeatureType.MY_STAND_BISHOP_1, nshogi.PieceType.BISHOP, 1),
            (nshogi.ml.FeatureType.MY_STAND_BISHOP_2, nshogi.PieceType.BISHOP, 2),
            (nshogi.ml.FeatureType.MY_STAND_ROOK_1, nshogi.PieceType.ROOK, 1),
            (nshogi.ml.FeatureType.MY_STAND_ROOK_2, nshogi.PieceType.ROOK, 2),
        ]

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)
            state_config = nshogi.StateConfig()

            for (feature, piece_type, threshold) in features:
                feature_types = [
                    feature
                ]

                x = nshogi.ml.FeatureStack(feature_types, state, state_config)
                x = x.to_numpy()

                count = state.get_stand_count(state.side_to_move, piece_type)

                if count >= threshold:
                    self.assertTrue(all(x == 1))
                else:
                    self.assertTrue(all(x == 0))

    def test_feature_pieces_op_stand(self):
        features = [
            (nshogi.ml.FeatureType.OP_STAND_PAWN_1, nshogi.PieceType.PAWN, 1),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_2, nshogi.PieceType.PAWN, 2),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_3, nshogi.PieceType.PAWN, 3),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_4, nshogi.PieceType.PAWN, 4),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_5, nshogi.PieceType.PAWN, 5),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_6, nshogi.PieceType.PAWN, 6),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_7, nshogi.PieceType.PAWN, 7),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_8, nshogi.PieceType.PAWN, 8),
            (nshogi.ml.FeatureType.OP_STAND_PAWN_9, nshogi.PieceType.PAWN, 9),
            (nshogi.ml.FeatureType.OP_STAND_LANCE_1, nshogi.PieceType.LANCE, 1),
            (nshogi.ml.FeatureType.OP_STAND_LANCE_2, nshogi.PieceType.LANCE, 2),
            (nshogi.ml.FeatureType.OP_STAND_LANCE_3, nshogi.PieceType.LANCE, 3),
            (nshogi.ml.FeatureType.OP_STAND_LANCE_4, nshogi.PieceType.LANCE, 4),
            (nshogi.ml.FeatureType.OP_STAND_KNIGHT_1, nshogi.PieceType.KNIGHT, 1),
            (nshogi.ml.FeatureType.OP_STAND_KNIGHT_2, nshogi.PieceType.KNIGHT, 2),
            (nshogi.ml.FeatureType.OP_STAND_KNIGHT_3, nshogi.PieceType.KNIGHT, 3),
            (nshogi.ml.FeatureType.OP_STAND_KNIGHT_4, nshogi.PieceType.KNIGHT, 4),
            (nshogi.ml.FeatureType.OP_STAND_SILVER_1, nshogi.PieceType.SILVER, 1),
            (nshogi.ml.FeatureType.OP_STAND_SILVER_2, nshogi.PieceType.SILVER, 2),
            (nshogi.ml.FeatureType.OP_STAND_SILVER_3, nshogi.PieceType.SILVER, 3),
            (nshogi.ml.FeatureType.OP_STAND_SILVER_4, nshogi.PieceType.SILVER, 4),
            (nshogi.ml.FeatureType.OP_STAND_GOLD_1, nshogi.PieceType.GOLD, 1),
            (nshogi.ml.FeatureType.OP_STAND_GOLD_2, nshogi.PieceType.GOLD, 2),
            (nshogi.ml.FeatureType.OP_STAND_GOLD_3, nshogi.PieceType.GOLD, 3),
            (nshogi.ml.FeatureType.OP_STAND_GOLD_4, nshogi.PieceType.GOLD, 4),
            (nshogi.ml.FeatureType.OP_STAND_BISHOP_1, nshogi.PieceType.BISHOP, 1),
            (nshogi.ml.FeatureType.OP_STAND_BISHOP_2, nshogi.PieceType.BISHOP, 2),
            (nshogi.ml.FeatureType.OP_STAND_ROOK_1, nshogi.PieceType.ROOK, 1),
            (nshogi.ml.FeatureType.OP_STAND_ROOK_2, nshogi.PieceType.ROOK, 2),
        ]

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)
            state_config = nshogi.StateConfig()

            for (feature, piece_type, threshold) in features:
                feature_types = [
                    feature
                ]

                x = nshogi.ml.FeatureStack(feature_types, state, state_config)
                x = x.to_numpy()

                count = state.get_stand_count(nshogi.Color(1 - int(state.side_to_move)), piece_type)

                if count >= threshold:
                    self.assertTrue(all(x == 1))
                else:
                    self.assertTrue(all(x == 0))

    def test_feature_check(self):
        feature_types = [
            nshogi.ml.FeatureType.CHECK
        ]

        state_config = nshogi.StateConfig()

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)

            x = nshogi.ml.FeatureStack(feature_types, state, state_config)
            x = x.to_numpy()

            if state.is_in_check:
                self.assertTrue(all(x == 1))
            else:
                self.assertTrue(all(x == 0))

    def test_feature_no_my_pawn_file(self):
        feature_types = [
            nshogi.ml.FeatureType.NO_MY_PAWN_FILE
        ]

        state_config = nshogi.StateConfig()

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)

            x = nshogi.ml.FeatureStack(feature_types, state, state_config)
            x = x.to_numpy()

            expected = np.zeros(81)

            for i in range(81):
                if state.side_to_move == nshogi.Color.BLACK:
                    if state.get_piece_on(i) == nshogi.Piece.BLACK_PAWN:
                        y = i // 9
                        expected[y*9:y*9+9] = 1.0
                else:
                    if state.get_piece_on(i) == nshogi.Piece.WHITE_PAWN:
                        y = (80 - i) // 9
                        expected[y*9:y*9+9] = 1.0

            expected = 1.0 - expected
            self.assertTrue(all(expected == x))

    def test_feature_no_op_pawn_file(self):
        feature_types = [
            nshogi.ml.FeatureType.NO_OP_PAWN_FILE
        ]

        state_config = nshogi.StateConfig()

        for sfen in self.sfens:
            state = nshogi.io.sfen.make_state_from_sfen(sfen)

            x = nshogi.ml.FeatureStack(feature_types, state, state_config)
            x = x.to_numpy()

            expected = np.zeros(81)

            for i in range(81):
                if state.side_to_move == nshogi.Color.BLACK:
                    if state.get_piece_on(i) == nshogi.Piece.WHITE_PAWN:
                        y = i // 9
                        expected[y*9:y*9+9] = 1.0
                else:
                    if state.get_piece_on(i) == nshogi.Piece.BLACK_PAWN:
                        y = (80 - i) // 9
                        expected[y*9:y*9+9] = 1.0

            expected = 1.0 - expected
            self.assertTrue(all(expected == x))


if __name__ == "__main__":
    random.seed(42)

    unittest.main()
