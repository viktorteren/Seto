# State graph generated by ./write_sg 4.2 (compiled 15-Oct-03 at 3:06 PM)
# from </home/vale/Scaricati/iccad95/intel_div3.g> on 26-Jul-18 at 9:35 PM
.model /home/vale/Scaricati/iccad95/intel_div3.g
.inputs  clk
.outputs  q1
.state graph 
s0 q1+ s1
s5 clk+ s6
s3 clk- s4
s6 clk- s7
s7 clk+ s0
s4 q1- s5
s2 clk+ s3
s1 clk- s2
.marking {s0}
.end