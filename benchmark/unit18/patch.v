module patch (t_0, g1, g2, g3, g4, g5, g6);
input g1, g2, g3, g4, g5, g6;
output t_0;
wire w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19, w20, w21, w22, w23, w24, w25, w26, w27, w28, w29, w30, w31, w32, w33, w34, w35, w36, w37, w38, w39, w40, w41, w42, w43, w44, w45, w46, w47, w48, w49, w50, w51, w52, w53, w54;

and ( w1 , w22 , g3 );
and ( w2 , w1 , w7 );
and ( w3 , g1 , w2 );
and ( w4 , w3 , g2 );
and ( w5 , g1 , w4 );
and ( w6 , g2 , w2 );
not ( w7 , g4 );
and ( w8 , w6 , w7 );
nor ( w9 , w5 , w8 );
and ( w10 , w16 , g2 );
and ( w11 , w9 , w10 );
not ( w12 , w11 );
and ( w13 , g1 , w12 );
not ( w14 , w13 );
and ( w15 , w14 , g2 );
not ( w16 , w2 );
and ( w17 , w16 , w15 );
and ( w18 , w17 , g2 );
and ( w19 , w15 , w18 );
and ( w20 , g2 , w36 );
and ( w21 , w9 , w27 );
not ( w22 , g2 );
and ( w23 , w9 , w22 );
nor ( w24 , w23 , w18 );
not ( w25 , w24 );
and ( w26 , w21 , w25 );
not ( w27 , w20 );
and ( w28 , w26 , w27 );
nor ( w29 , g1 , w26 );
not ( w30 , w29 );
and ( w31 , w28 , w30 );
and ( w32 , w31 , w26 );
and ( w33 , w32 , g5 );
and ( w34 , w33 , g6 );
and ( w35 , w52 , g1 );
not ( w36 , w19 );
and ( w37 , w35 , w36 );
nor ( w38 , g6 , w20 );
not ( w39 , g5 );
and ( w40 , w38 , w39 );
not ( w41 , w40 );
and ( w42 , w37 , w41 );
and ( w43 , g3 , w28 );
and ( w44 , w43 , w26 );
not ( w45 , g6 );
and ( w46 , w44 , w45 );
not ( w47 , w46 );
and ( w48 , w47 , g1 );
not ( w49 , w48 );
and ( w50 , w49 , g4 );
nor ( w51 , w50 , w19 );
not ( w52 , w34 );
and ( w53 , w51 , w52 );
and ( w54 , w37 , w53 );
and ( t_0 , w42 , w54 );

endmodule
