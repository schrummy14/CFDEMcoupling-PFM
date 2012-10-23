clc;
close all;
clear;

%==========================================================================
%m-file to calculate the properties of the imported clump
%==========================================================================

fprintf('\n sphere10.multisphere:\n')

rhoC=440
scaling=1e-3
nP=10
rP=0.3530494
VP=nP*(2*rP*scaling)^3*pi/6
dH=2*scaling
VC=dH^3*pi/6
rhoP=rhoC*VC/VP

%   -0.3301476 -3.4959164E-008       0.5563700	0.3530494
%   0.5421265 -3.2849375E-008       0.3530494	0.3530494
%   0.0822932       0.5358442       0.3530495	0.3530494
%   0.0822932      -0.5358442       0.3530494	0.3530494
%   -0.5126716      -0.3946047 -2.9544078E-009	0.3530494
%   -0.5126715       0.3946047 -1.7513663E-008	0.3530494
%   0.0822932       0.5358442      -0.3530494	0.3530494
%   0.0822932      -0.5358442      -0.3530495	0.3530494
%   0.5421264 -3.9639417E-008      -0.3530495	0.3530494
%   -0.3301475 -1.5302462E-008      -0.5563700	0.3530494