module patch (t_0, g1, g2, g3, g4);
input g1, g2, g3, g4;
output t_0;
wire w1, w2, w3, w4, w5, w6, w7, w8;

and ( w1 , g1 , g2 );
nor ( w2 , w1 , g3 );
not ( w3 , g4 );
and ( w4 , w3 , g2 );
not ( w5 , w2 );
and ( w6 , w5 , w4 );
not ( w7 , g3 );
and ( w8 , w7 , w1 );
or ( t_0 , w6 , w8 );

endmodule
