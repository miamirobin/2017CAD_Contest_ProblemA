module patch (t_0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11);
input g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11;
output t_0;
wire w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19, w20, w21, w22, w23, w24, w25, w26, w27, w28, w29, w30, w31, w32, w33, w34, w35, w36, w37, w38, w39, w40, w41, w42, w43, w44, w45, w46, w47, w48, w49, w50, w51, w52, w53, w54, w55, w56;

and ( w1 , g1 , g2 );
and ( w2 , w5 , g4 );
and ( w3 , w2 , g5 );
and ( w4 , g5 , g6 );
not ( w5 , g3 );
and ( w6 , w4 , w5 );
nor ( w7 , w3 , w6 );
nor ( w8 , w1 , w7 );
and ( w9 , g2 , g7 );
and ( w10 , w21 , g7 );
nor ( w11 , g7 , w7 );
nor ( w12 , w10 , w11 );
nor ( w13 , w12 , g2 );
not ( w14 , g7 );
and ( w15 , w14 , g2 );
nor ( w16 , w15 , w7 );
and ( w17 , w13 , w16 );
nor ( w18 , w9 , w17 );
not ( w19 , w18 );
and ( w20 , w19 , w16 );
not ( w21 , g8 );
and ( w22 , g4 , w21 );
not ( w23 , w22 );
and ( w24 , w23 , g7 );
and ( w25 , w24 , g1 );
not ( w26 , g2 );
and ( w27 , w25 , w26 );
nor ( w28 , w27 , w6 );
and ( w29 , w28 , w44 );
not ( w30 , g9 );
and ( w31 , w29 , w30 );
and ( w32 , g10 , g9 );
nor ( w33 , w32 , w6 );
and ( w34 , g5 , g8 );
nor ( w35 , w34 , g2 );
and ( w36 , w35 , g7 );
nor ( w37 , w36 , w6 );
not ( w38 , w37 );
and ( w39 , w38 , g1 );
and ( w40 , w39 , g7 );
nor ( w41 , w40 , w6 );
and ( w42 , w33 , w41 );
nor ( w43 , w42 , g3 );
not ( w44 , g6 );
and ( w45 , w33 , w44 );
nor ( w46 , w45 , g3 );
and ( w47 , w46 , g10 );
nor ( w48 , w47 , w6 );
and ( w49 , g11 , g6 );
nor ( w50 , w49 , w6 );
nor ( w51 , w48 , w50 );
nor ( w52 , w43 , w51 );
nor ( w53 , w31 , w52 );
nor ( w54 , w20 , w53 );
not ( w55 , w54 );
and ( w56 , w8 , w55 );
nor ( t_0 , w56 , w53 );

endmodule