module top ( y1 , y2 , a , b , c ); 
input a , b , c ; 
output y1 , y2 ; 
wire g1 , g2 , g3 ,g4, g5; 
wire t_0 ,t_1; 

and ( g1 , a , b ); 
xor ( g2 , a , c ); 
nor ( g3 , b , c ); 
and ( y1 , g1 , g4 ); 
or ( y2 , t_0 , g5 ); 
xor ( g4 , a , t_1 ); 
nor ( g5 , b , t_1 ); 
patch p0 (.t_0(t_0), .t_1(t_1), .g1(g2), .g2(g1), .g3(g3));
endmodule 

