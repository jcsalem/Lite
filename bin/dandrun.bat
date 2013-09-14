:LOOP
Ltool --time 320 --filter random rotwash "hsv(.65,.45,.35)" "hsv(.68,.7,.75)" 
Lstarry --color "range:hsv(.1,1,1);hsv(.134,.5,.2)" --rate 10 --time 100
Ltool --rate .25 --time 10 rotate "hsv(.11,1,.8)"
Ltool --rate .5 --time 10 rotate "hsv(.11,1,.8)"
Ltool --rate 1 --time 10 rotate "hsv(.11,1,.8)"
Ltool --rate 2 --time 5 rotate "hsv(.11,1,.8)"
Ltool --rate 4 --time 2.5 rotate "hsv(.11,1,.8)"
Ltool --rate 6 --time 1 rotate "hsv(.11,1,.8)"
Ltool  --time 100 --filter random rotwash "hsv(0,.8,.5)" "hsv(1,.8,.5)"
Lstarry --time 240 --rate 4.0 --fade 3
Lsparkle --color realstar --time 30 --fade 1
goto LOOP