module patch (t_0, t_1, g1, g2, g3);
input g1, g2, g3;
output t_0, t_1;
wire w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11;

nor ( w1 , g2 , g3 );
not ( w2 , g1 );
and ( w3 , w2 , w1 );
nor ( w4 , g2 , g1 );
and ( w5 , w4 , w1 );
nor ( w6 , w3 , w5 );
not ( w7 , w6 );
and ( w8 , w7 , w1 );
nor ( w9 , g2 , w6 );
nor ( w10 , w9 , w5 );
not ( w11 , w10 );
nand ( t_0 , w8 , w11 );
nand ( t_1 , g1 , g2 );

endmodule
