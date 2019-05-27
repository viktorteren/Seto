.model ff
.inputs 00 01
.outputs 10 11
.state graph
s2r0z1 10 s2r2z1
s2r0z1 01 s0r1z1
s2r0z1 11 error
s2r2z1 01 s0r1z1
s2r2z1 00 s2r0z1
s0r1z1 10 s1r0z0
s0r1z1 00 s2r0z1
s0r1z1 11 s1r1z0
s1r1z0 10 s1r0z0
s1r1z0 01 s0r1z0
s1r1z0 00 s2r0z0
s1r0z0 11 s1r1z0
s1r0z0 00 s2r0z0
s1r0z0 01 s0r1z0
s2r0z0 01 s0r1z0
s2r0z0 10 s2r2z1
s2r0z0 11 error
s0r1z0 00 s2r0z0
s0r1z0 10 s1r0z0
s0r1z0 11 s1r1z0
.marking {s2r0z1}
.end

