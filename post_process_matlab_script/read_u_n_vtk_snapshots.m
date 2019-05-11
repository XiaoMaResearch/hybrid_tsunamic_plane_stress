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
%Element = Element+1;
%number of elements:
[rows,~] =size(Nodes);

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
u_n_store = reshape(u_n,2*rows,[]);
[~,numt] = size(u_n_store);
%%
l_x = max(Nodes(:,1))-min(Nodes(:,1));
l_y = max(Nodes(:,2))-min(Nodes(:,2));



%%
dx = 50.0;
nx = l_x/dx;
ny = l_y/dx;
dim = 2;

for i =1:numt
    Node = Nodes;
Element = Elements;
   U = u_n(1+(i-1)*(nx+1)*(ny+2)*dim:i*(nx+1)*(ny+2)*dim);
   folder2 = 'disp_vtk_output';
   name = 'U';
str = sprintf([folder2 '/data_%d.vtk'],i);
filename = str;
    CreateVtk_Binary
end





%%
%save('results/data_main.mat','slip_store_couple','slip_rate_store_couple','shear_store_couple','x','numt','time')
%% 
