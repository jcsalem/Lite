:LOOP
cktool --time 320 --outmap random rotwash "hsv(.65,.45,.35)" "hsv(.68,.7,.75)" 
ckstarry --color "range:hsv(.1,1,1);hsv(.134,.5,.2)" --rate 10 --time 100
cktool --rate .25 --time 10 rotate "hsv(.11,1,.8)"
cktool --rate .5 --time 10 rotate "hsv(.11,1,.8)"
cktool --rate 1 --time 10 rotate "hsv(.11,1,.8)"
cktool --rate 2 --time 5 rotate "hsv(.11,1,.8)"
cktool --rate 4 --time 2.5 rotate "hsv(.11,1,.8)"
cktool --rate 6 --time 1 rotate "hsv(.11,1,.8)"
cktool  --time 100 --outmap random rotwash "hsv(0,.8,.5)" "hsv(1,.8,.5)"
ckstarry --time 240 --rate 4.0 --fade 3
cksparkle --color realstar --time 30 --fade 1
goto LOOP