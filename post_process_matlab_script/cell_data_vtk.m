clear all
close all
clc
%%
folder = 'results';
fileID = fopen([folder '/ezz.bin']);
u_n = fread(fileID,'double');
fclose(fileID);
Nodes = load([folder '/Node.txt']);
Elements = load([folder '/Element.txt']);
%Element = Element+1;
%number of elements:
%%
[rows,~] =size(Elements);

density = 2670.0;
v_s =3.464e3;
v_p = 6.0e3;
G= v_s*2*density;
Lambda = v_p^2*density-2.0*G;
E  = G*(3.0*Lambda+2.0*G)/(Lambda+G);
nu = Lambda/(2.0*(Lambda+G));
time_data = load([folder '/time.txt']);
dt = time_data(2);
numt = time_data(3)/20;
time_run = time_data(1);
u_n_store = reshape(u_n,rows,[]);
[~,numt] = size(u_n_store);
%%

Lc = 500.0;


%%
for i =1:1:numt
U_temp = u_n_store(:,i);
Node = Nodes;
Element = Elements;
folder2 = 'vtk_output';
dim = 2;
name = 'out of plane strain';
str = sprintf([folder2 '/data_%d.vtk'],i);
filename = str;
CreateVtk_Binary_element
end