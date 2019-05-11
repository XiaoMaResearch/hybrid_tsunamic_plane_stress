clear all
close all
clc
%%
folder = 'results';
fileID = fopen([folder '/v_n.bin']);
u_n = fread(fileID,'double');
fclose(fileID);
Nodes = load([folder '/Node.txt']);
Elements = load([folder '/Element.txt']);
[num_nodes,~] =size(Nodes);
dx = abs(Nodes(2,1)-Nodes(1,1));
dim =2; 
density = 2670.0;
v_s =3.464e3;
v_p = 6.0e3;
G= v_s*2*density;
Lambda = v_p^2*density-2.0*G;
E  = G*(3.0*Lambda+2.0*G)/(Lambda+G);
nu = Lambda/(2.0*(Lambda+G));
time_data = load([folder '/time.txt']);
dt = time_data(2);
time_run = time_data(1);
u_n_store = reshape(u_n,dim*num_nodes,[]);
[~,numt] = size(u_n_store);
%%
for i =1:numt
    Node = Nodes;
    Element = Elements;
    U = u_n_store(:,i);
    folder2 = 'point_vtk_output';
    name = 'U';
    str = sprintf([folder2 '/data_%d.vtk'],i);
    filename = str;
    CreateVtk_Binary_point;
end
