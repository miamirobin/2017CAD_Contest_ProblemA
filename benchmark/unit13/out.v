module top ( g0 , g1 , g2 , g3 , g4 , g5 , g6 , g7 , g8 , g9 , g10 , g11 , g12 , g13 , g14 , g15 , g16 , g17 , g18 , g19 , g20 , g21 , g22 , g23 , g24 , g25 , g26 , g27 , g28 , g29 , g30 , g31 , g32 , g33 , g34 , g35 , g36 , g37 , g38 , g39 , g40 , g41 , g42 , g43 , g44 , g45 , g46 , g47 , g48 , g49 , g50 , g51 , g52 , g53 , g54 , g55 , g56 , g57 , g58 , g59 , g60 , g61 , g62 , g63 ); 
input g0 , g1 , g2 , g3 , g4 , g5 , g6 , g7 , g8 , g9 , g10 , g11 , g12 , g13 , g14 , g15 , g16 , g17 , g18 , g19 , g20 , g21 , g22 , g23 , g24 ; 
output g25 , g26 , g27 , g28 , g29 , g30 , g31 , g32 , g33 , g34 , g35 , g36 , g37 , g38 , g39 , g40 , g41 , g42 , g43 , g44 , g45 , g46 , g47 , g48 , g49 , g50 , g51 , g52 , g53 , g54 , g55 , g56 , g57 , g58 , g59 , g60 , g61 , g62 , g63 ; 

wire n1 , n2 , n3 , n4 , n5 , n6 , n7 , n8 , n9 , 
n10 , n11 , n12 , n13 , n14 , n15 , n16 , n17 , n18 , n19 , 
n20 , n21 , n22 , n23 , n24 , n25 , n26 , n27 , n28 , n29 , 
n30 , n31 , n32 , n33 , n34 , n35 , n36 , n37 , n38 , n39 , 
n40 , n41 , n42 , n43 , n44 , n45 , n46 , n47 , n48 , n49 , 
n50 , n51 , n52 , n53 , n54 , n55 , n56 , n57 , n58 , n59 , 
n60 , n61 , n62 , n63 , n64 , n65 , n66 , n67 , n68 , n69 , 
n70 , n71 , n72 , n73 , n74 , n75 , n76 , n77 , n78 , n79 , 
n80 , n81 , n82 , n83 , n84 , n85 , n86 , n87 , n88 , n89 , 
n90 , n91 , n92 , n93 , n94 , n95 , n96 , n97 , n98 , n99 , 
n100 , n101 , n102 , n103 , n104 , n105 , n106 , n107 , n108 , n109 , 
n110 , n111 , n112 , n113 , n114 , n115 , n116 , n117 , n118 , n119 , 
n120 , n121 , n122 , n123 , n124 , n125 , n126 , n127 , n128 , n129 , 
n130 , n131 , n132 , n133 , n134 , n135 , n136 , n137 , n138 , n139 , 
n140 , n141 , n142 , n143 , n144 , n145 , n146 , n147 , n148 , n149 , 
n150 , n151 , n152 , n153 , n154 , n155 , n156 , n157 , n158 , n159 , 
n160 , n161 , n162 , n163 , n164 , n165 , n166 , n167 , n168 , n169 , 
n170 , n171 , n172 , n173 , n174 , n175 , n176 , n177 , n178 , n179 , 
n180 , n181 , n182 , n183 , n184 , n185 , n186 , n187 , n188 , n189 , 
n190 , n191 , n192 , n193 , n194 , n195 , n196 , n197 , n198 , n199 , 
n200 , n201 , n202 , n203 , n204 , n205 , n206 , n207 , n208 , n209 , 
n210 , n211 , n212 , n213 , n214 , n215 , n216 , n217 , n218 , n219 , 
n220 , n221 , n222 , n223 , n224 , n225 , n226 , n227 , n228 , n229 , 
n230 , n231 , n232 , n233 , n234 , n235 , n236 , n237 , n238 , n239 , 
n240 , n241 , n242 , n243 , n244 , n245 , n246 , n247 , n248 , n249 , 
n250 , n251 , n252 , n253 , n254 , n255 , n256 , n257 , n258 , n259 , 
n260 , n261 , n262 , n263 , n264 , n265 , n266 , n267 , n268 , n269 , 
n270 , n271 , n272 , n273 , n274 , n275 , n276 , n277 , n278 , n279 , 
n280 , n281 , n282 , n283 , n284 , n285 , n286 , n287 , n288 , n289 , 
n290 , n291 , n292 , n293 , n294 , n295 , n296 , n297 , n298 , n299 , 
n300 , n301 , n302 , n303 , n304 , n305 , n306 , n307 , n308 , n309 , 
n310 , n311 , n312 , n313 , n314 , n315 , n316 , n317 , n318 , n319 , 
n320 , n321 , n322 , n323 , n324 , n325 , n326 , n327 , n328 , n329 , 
n330 , n331 , n332 , n333 , n334 , n335 , n336 , n337 , n338 , n339 , 
n340 ; 
wire t_0 ; 
buf ( n1 , g0 ); 
buf ( n2 , g1 ); 
buf ( n3 , g2 ); 
buf ( n4 , g3 ); 
buf ( n5 , g4 ); 
buf ( n6 , g5 ); 
buf ( n7 , g6 ); 
buf ( n8 , g7 ); 
buf ( n9 , g8 ); 
buf ( n10 , g9 ); 
buf ( n11 , g10 ); 
buf ( n12 , g11 ); 
buf ( n13 , g12 ); 
buf ( n14 , g13 ); 
buf ( n15 , g14 ); 
buf ( n16 , g15 ); 
buf ( n17 , g16 ); 
buf ( n18 , g17 ); 
buf ( n19 , g18 ); 
buf ( n20 , g19 ); 
buf ( n21 , g20 ); 
buf ( n22 , g21 ); 
buf ( n23 , g22 ); 
buf ( n24 , g23 ); 
buf ( n25 , g24 ); 
buf ( g25 , n26 ); 
buf ( g26 , n27 ); 
buf ( g27 , n28 ); 
buf ( g28 , n29 ); 
buf ( g29 , n30 ); 
buf ( g30 , n31 ); 
buf ( g31 , n32 ); 
buf ( g32 , n33 ); 
buf ( g33 , n34 ); 
buf ( g34 , n35 ); 
buf ( g35 , n36 ); 
buf ( g36 , n37 ); 
buf ( g37 , n38 ); 
buf ( g38 , n39 ); 
buf ( g39 , n40 ); 
buf ( g40 , n41 ); 
buf ( g41 , n42 ); 
buf ( g42 , n43 ); 
buf ( g43 , n44 ); 
buf ( g44 , n45 ); 
buf ( g45 , n46 ); 
buf ( g46 , n47 ); 
buf ( g47 , n48 ); 
buf ( g48 , n49 ); 
buf ( g49 , n50 ); 
buf ( g50 , n51 ); 
buf ( g51 , n52 ); 
buf ( g52 , n53 ); 
buf ( g53 , n54 ); 
buf ( g54 , n55 ); 
buf ( g55 , n56 ); 
buf ( g56 , n57 ); 
buf ( g57 , n58 ); 
buf ( g58 , n59 ); 
buf ( g59 , n60 ); 
buf ( g60 , n61 ); 
buf ( g61 , n62 ); 
buf ( g62 , n63 ); 
buf ( g63 , n64 ); 
buf ( n26 , n321 ); 
buf ( n27 , n329 ); 
buf ( n28 , n340 ); 
buf ( n29 , n131 ); 
buf ( n30 , n339 ); 
buf ( n31 , n331 ); 
buf ( n32 , n196 ); 
buf ( n33 , n178 ); 
buf ( n34 , n334 ); 
buf ( n35 , n180 ); 
buf ( n36 , n336 ); 
buf ( n37 , n335 ); 
buf ( n38 , n264 ); 
buf ( n39 , n180 ); 
buf ( n40 , n159 ); 
buf ( n41 , n337 ); 
buf ( n42 , n308 ); 
buf ( n43 , n211 ); 
buf ( n44 , n330 ); 
buf ( n45 , 1'b0 ); 
buf ( n46 , 1'b0 ); 
buf ( n47 , n338 ); 
buf ( n48 , n275 ); 
buf ( n49 , 1'b0 ); 
buf ( n50 , 1'b0 ); 
buf ( n51 , n338 ); 
buf ( n52 , n169 ); 
buf ( n53 , 1'b0 ); 
buf ( n54 , 1'b0 ); 
buf ( n55 , n338 ); 
buf ( n56 , n314 ); 
buf ( n57 , 1'b0 ); 
buf ( n58 , 1'b0 ); 
buf ( n59 , n338 ); 
buf ( n60 , n207 ); 
buf ( n61 , 1'b0 ); 
buf ( n62 , 1'b0 ); 
buf ( n63 , n338 ); 
buf ( n64 , n245 ); 
not ( n77 , n25 ); 
nor ( n78 , n77 , n24 ); 
not ( n79 , n78 ); 
nand ( n80 , n8 , n24 ); 
nand ( n81 , n79 , n12 , n80 ); 
not ( n82 , n21 ); 
nand ( n83 , n81 , n82 ); 
not ( n84 , n25 ); 
not ( n85 , n22 ); 
nor ( n86 , n85 , n12 ); 
nand ( n87 , n84 , n86 ); 
not ( n88 , n3 ); 
nor ( n89 , n88 , n2 ); 
not ( n90 , n9 ); 
nor ( n91 , n90 , n7 ); 
nand ( n92 , n89 , n91 ); 
not ( n93 , n92 ); 
nor ( n94 , n22 , n24 ); 
and ( n95 , n12 , n94 ); 
nand ( n96 , n93 , n25 , n95 ); 
and ( n97 , n83 , n87 , n96 ); 
not ( n98 , n23 ); 
nor ( n99 , n97 , n98 ); 
not ( n100 , n22 ); 
nand ( n101 , n21 , n98 ); 
nor ( n102 , n100 , n101 ); 
not ( n103 , n22 ); 
not ( n104 , n25 ); 
nand ( n105 , n104 , n24 ); 
or ( n106 , n103 , n12 , n105 ); 
nor ( n107 , n4 , n5 ); 
not ( n108 , n107 ); 
nand ( n109 , n98 , n22 ); 
nor ( n110 , n109 , n105 ); 
nand ( n111 , n108 , n110 ); 
nand ( n112 , n106 , n111 ); 
or ( n113 , n99 , n102 , n112 ); 
not ( n114 , n14 ); 
nor ( n115 , n114 , n13 ); 
not ( n116 , n115 ); 
not ( n117 , n116 ); 
nand ( n118 , n113 , n117 ); 
nor ( n119 , n22 , n25 ); 
not ( n120 , n24 ); 
and ( n121 , n119 , n82 , n120 ); 
not ( n122 , n121 ); 
not ( n123 , n122 ); 
and ( n124 , n14 , n123 ); 
not ( n125 , n22 ); 
nand ( n126 , n125 , n24 ); 
not ( n127 , n126 ); 
nand ( n128 , n25 , n23 , n127 ); 
and ( n129 , n128 , n122 ); 
not ( n130 , n18 ); 
nor ( n131 , n129 , n130 ); 
nor ( n132 , n124 , n131 ); 
not ( n133 , n109 ); 
not ( n134 , n133 ); 
nand ( n135 , n21 , n25 ); 
or ( n136 , n13 , n135 ); 
not ( n137 , n19 ); 
nand ( n138 , n137 , n120 , n84 , n82 ); 
nand ( n139 , n136 , n138 ); 
not ( n140 , n139 ); 
or ( n141 , n134 , n140 ); 
nand ( n142 , n12 , n7 , n9 ); 
not ( n143 , n142 ); 
not ( n144 , n2 ); 
not ( n145 , n3 ); 
nor ( n146 , n144 , n145 ); 
nand ( n147 , n143 , n146 ); 
or ( n148 , n84 , n147 ); 
not ( n149 , n14 ); 
nand ( n150 , n21 , n149 ); 
nand ( n151 , n148 , n150 ); 
not ( n152 , n13 ); 
nor ( n153 , n22 , n23 ); 
and ( n154 , n152 , n153 ); 
nand ( n155 , n151 , n24 , n154 ); 
nand ( n156 , n141 , n155 ); 
nand ( n157 , n10 , n12 ); 
nand ( n158 , n21 , n78 , n153 ); 
nor ( n159 , n157 , n158 ); 
nand ( n160 , n19 , n24 ); 
not ( n161 , n21 ); 
nand ( n162 , n161 , n14 ); 
nand ( n163 , n120 , n162 ); 
nand ( n164 , n22 , n25 ); 
not ( n165 , n164 ); 
and ( n166 , n160 , n163 , n98 , n165 ); 
nor ( n167 , n156 , n159 , n166 ); 
and ( n168 , n118 , n132 , n167 ); 
nor ( n169 , n168 , n16 ); 
not ( n170 , n19 ); 
not ( n171 , n165 ); 
not ( n172 , n171 ); 
not ( n173 , n24 ); 
nor ( n174 , n173 , n23 ); 
nand ( n175 , n170 , n172 , n174 ); 
not ( n176 , n164 ); 
nand ( n177 , n176 , n23 , n21 ); 
nor ( n178 , n11 , n177 ); 
not ( n179 , n178 ); 
and ( n180 , n25 , n23 , n127 ); 
not ( n181 , n15 ); 
and ( n182 , n181 , n130 ); 
not ( n183 , n17 ); 
nand ( n184 , n180 , n182 , n183 ); 
nand ( n185 , n179 , n184 ); 
not ( n186 , n185 ); 
not ( n187 , n12 ); 
not ( n188 , n21 ); 
nand ( n189 , n188 , n23 ); 
nor ( n190 , n84 , n187 , n189 ); 
and ( n191 , n21 , n23 ); 
not ( n192 , n12 ); 
nor ( n193 , n192 , n25 ); 
not ( n194 , n193 ); 
not ( n195 , n194 ); 
and ( n196 , n191 , n22 , n195 ); 
nand ( n197 , n23 , n22 , n187 ); 
and ( n198 , n153 , n147 ); 
and ( n199 , n23 , n8 , n22 ); 
nor ( n200 , n199 , n25 ); 
nor ( n201 , n198 , n200 ); 
and ( n202 , n197 , n201 ); 
nor ( n203 , n202 , n120 ); 
or ( n204 , n190 , n196 , n203 ); 
nand ( n205 , n204 , n117 ); 
and ( n206 , n175 , n186 , n205 ); 
nor ( n207 , n206 , n16 ); 
nand ( n208 , n130 , n121 ); 
not ( n209 , n208 ); 
nand ( n210 , n14 , n209 ); 
not ( n211 , n158 ); 
nand ( n212 , n21 , n22 ); 
not ( n213 , n212 ); 
not ( n214 , n213 ); 
nand ( n215 , n22 , n23 ); 
not ( n216 , n215 ); 
not ( n217 , n24 ); 
nor ( n218 , n217 , n25 ); 
nor ( n219 , n216 , n218 ); 
and ( n220 , n214 , n219 ); 
nor ( n221 , n220 , n14 ); 
not ( n222 , n127 ); 
nand ( n223 , n82 , n80 ); 
and ( n224 , n222 , n223 ); 
nand ( n225 , n23 , n193 ); 
nor ( n226 , n224 , n225 ); 
nor ( n227 , n221 , n226 ); 
not ( n228 , n4 ); 
nand ( n229 , n5 , n6 ); 
and ( n230 , n228 , n229 ); 
nor ( n231 , n230 , n21 ); 
nand ( n232 , n231 , n12 , n110 ); 
nand ( n233 , n103 , n174 ); 
or ( n234 , n86 , n94 ); 
nand ( n235 , n234 , n23 ); 
nand ( n236 , n233 , n214 , n235 ); 
nand ( n237 , n25 , n236 ); 
nand ( n238 , n227 , n232 , n237 ); 
and ( n239 , n152 , n238 ); 
not ( n240 , n119 ); 
or ( n241 , n24 , n150 , n240 ); 
nand ( n242 , n241 , n177 ); 
nor ( n243 , n239 , n242 ); 
and ( n244 , t_0 , n243 ); 
nor ( n245 , n244 , n16 ); 
not ( n246 , n12 ); 
not ( n247 , n25 ); 
and ( n248 , n246 , n247 ); 
nor ( n249 , n10 , n23 ); 
nor ( n250 , n248 , n249 ); 
not ( n251 , n174 ); 
not ( n252 , n22 ); 
nand ( n253 , n252 , n12 ); 
nand ( n254 , n120 , n253 ); 
nand ( n255 , n250 , n240 , n251 , n254 ); 
and ( n256 , n21 , n255 ); 
not ( n257 , n219 ); 
and ( n258 , n116 , n257 ); 
nor ( n259 , n256 , n258 ); 
not ( n260 , n195 ); 
nand ( n261 , n103 , n92 ); 
nand ( n262 , n260 , n115 , n261 ); 
and ( n263 , n262 , n23 , n120 ); 
and ( n264 , n25 , n82 , n103 ); 
nor ( n265 , n263 , n264 ); 
not ( n266 , n12 ); 
nand ( n267 , n84 , n98 , n107 ); 
nand ( n268 , n240 , n189 , n267 ); 
not ( n269 , n268 ); 
or ( n270 , n266 , n269 ); 
nand ( n271 , n25 , n103 , n183 , n182 ); 
nand ( n272 , n270 , n271 ); 
nand ( n273 , n24 , n272 ); 
and ( n274 , n259 , n265 , n273 ); 
nor ( n275 , n274 , n16 ); 
not ( n276 , n194 ); 
nand ( n277 , n276 , n102 ); 
and ( n278 , n5 , n4 ); 
not ( n279 , n5 ); 
and ( n280 , n279 , n228 ); 
nor ( n281 , n278 , n280 ); 
nor ( n282 , n187 , n6 ); 
nand ( n283 , n110 , n281 , n282 ); 
not ( n284 , n24 ); 
nand ( n285 , n284 , n164 ); 
nand ( n286 , n135 , n187 ); 
nand ( n287 , n285 , n286 ); 
and ( n288 , n23 , n287 ); 
and ( n289 , n120 , n21 , n119 ); 
nor ( n290 , n288 , n289 ); 
and ( n291 , n277 , n283 , n290 ); 
nor ( n292 , n291 , n149 ); 
nand ( n293 , n78 , n103 , n23 ); 
nand ( n294 , n189 , n293 ); 
not ( n295 , n84 ); 
not ( n296 , n24 ); 
nand ( n297 , n296 , n215 ); 
not ( n298 , n297 ); 
or ( n299 , n295 , n298 ); 
not ( n300 , n212 ); 
not ( n301 , n126 ); 
or ( n302 , n300 , n301 ); 
nand ( n303 , n302 , n98 ); 
nand ( n304 , n299 , n303 ); 
nor ( n305 , n294 , n304 ); 
nor ( n306 , n152 , n305 ); 
nor ( n307 , n185 , n292 , n306 ); 
nor ( n308 , n187 , n158 ); 
not ( n309 , n308 ); 
not ( n310 , n150 ); 
nand ( n311 , n171 , n222 ); 
nand ( n312 , n310 , n311 , n98 ); 
and ( n313 , n307 , n309 , n312 ); 
nor ( n314 , n313 , n16 ); 
and ( n315 , n24 , n101 ); 
nor ( n316 , n315 , n213 ); 
or ( n317 , n25 , n316 ); 
nand ( n318 , n317 , n189 , n293 ); 
nand ( n319 , n149 , n318 ); 
not ( n320 , n306 ); 
nand ( n321 , n319 , n210 , n320 ); 
not ( n322 , n229 ); 
or ( n323 , n5 , n6 ); 
nand ( n324 , n323 , n4 ); 
not ( n325 , n324 ); 
or ( n326 , n322 , n325 ); 
nand ( n327 , n326 , n115 ); 
nor ( n328 , n21 , n327 ); 
and ( n329 , n328 , n12 , n110 ); 
nor ( n330 , n181 , n14 , n208 ); 
not ( n331 , n277 ); 
not ( n332 , n25 ); 
nand ( n333 , n332 , n120 , n153 ); 
nor ( n334 , n82 , n149 , n333 ); 
not ( n335 , n177 ); 
and ( n336 , n11 , n335 ); 
not ( n337 , n333 ); 
and ( n338 , n1 , n20 ); 
not ( n339 , n305 ); 
not ( n340 , n155 ); 
patch p0 (.t_0(t_0), .g1(n307), .g2(n30), .g3(g28), .g4(n202), .g5(n52), .g6(n123), .g7(n224), .g8(n56), .g9(g34), .g10(n181), .g11(n17));
endmodule 

