# State graph generated by ./write_sg 4.2 (compiled 15-Oct-03 at 3:06 PM)
# from </home/vale/Scaricati/iccad95/rcv-setup.g> on 26-Jul-18 at 9:34 PM
.model /home/vale/Scaricati/iccad95/rcv-setup.g
.inputs  reqrcv sending acksend
.outputs  enwoq rejsend
.state graph # 14 states
s1 reqrcv- s2
s13 enwoq- s0
s12 enwoq+ s1
s7 rejsend- s9
s9 acksend- s10
s6 sending- s8
s6 acksend+ s5
s3 reqrcv+ s4
s3 sending- s0
s2 enwoq- s0
s11 reqrcv- s13
s8 acksend+ s7
s10 enwoq+ s11
s4 rejsend+ s6
s5 sending- s7
s0 reqrcv+ s12
s0 sending+ s3
.marking {s0}
.end
