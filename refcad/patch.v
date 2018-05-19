module patch (t_0, a, b, c);
input a, b, c;
output t_0;
wire w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17;

not ( w1 , a );
not ( w2 , a );
and ( w3 , b , w2 );
not ( w4 , c );
and ( w5 , w3 , w4 );
not ( w6 , w5 );
and ( w7 , w1 , w6 );
not ( w8 , w7 );
not ( w9 , c );
not ( w10 , w5 );
and ( w11 , w9 , w10 );
not ( w12 , w11 );
and ( w13 , w8 , w12 );
not ( w14 , w5 );
and ( w15 , b , w14 );
not ( w16 , w15 );
and ( w17 , w13 , w16 );
not ( t_0 , w17 );

endmodule;
