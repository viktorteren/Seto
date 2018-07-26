.model clock.g
.inputs  c
.outputs  a
.state graph 
s7 c+ s2
s4 c- s6
s1 c+ s8
s6 a- s1
s3 c- s7
s9 c- s5
s8 a+ s3
s2 a- s9
s5 c+ s0
s0 a+ s4
.marking {s0}
.end

