% build the demo mex
mex commonsdemo.c mexCommons.c % -DDEBUG

% build a simple demo structure
d.d1 = [1 2 3]';
d.f1 = [4 5 6]';
d.i1 = [7 8 9]';

f = commonsdemo(d);

d
f
