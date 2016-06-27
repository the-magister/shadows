require(R.utils)
s = 3

intToBin(s)
d1 = 755
intToBin(d1)
d2 = 754
intToBin(d2)
d3 = 753
intToBin(d3)
soln = sprintf("%2s%10s%10s%10s",intToBin(s), intToBin(d3), intToBin(d2), intToBin(d1))
print(nchar(soln))

msg = 0
msg = bitwOr(bitwShiftR(msg,0), s)
print(nchar(intToBin(msg)))
msg = bitwOr(bitwShiftR(msg,10),d3)
print(nchar(intToBin(msg)))
msg = bitwOr(bitwShiftR(msg,10),d2)
print(nchar(intToBin(msg)))
msg = bitwOr(bitwShiftR(msg,10),d1)
print(nchar(intToBin(msg)))

msg.b = sprintf("%32s",intToBin(msg))
print(nchar(msg.b))

print(msg.b)
print(soln)