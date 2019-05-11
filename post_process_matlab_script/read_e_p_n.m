clear all
close all
clc
%%
folder = 'results';
fileID = fopen([folder '/ezz.bin']);
u_n = fread(fileID,'double');
fclose(fileID);
Nodes = load([folder '/Node.txt']);
Element = load([folder '/Element.txt']);
Element = Element+1;
%number of elements:
[rows,~] =size(Element);

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
time = dt*(1:1:numt)*20;
u_n_store = reshape(u_n,rows,[]);
[~,numt] = size(u_n_store);
time = dt*(1:1:numt)*100;
%%
clear vidObj
vidObj = VideoWriter('SED','MPEG-4');
open(vidObj);
%fig=figure;
set(gca,'FontSize',16)
set(0,'defaultlinelinewidth',2)
set(gcf,'color','w');
handle = PlotMesh_el(Nodes,Element,u_n_store(:,1));
for i=1:1:numt
set(handle,'FaceVertexCData',u_n_store(:,i));
currFrame=getframe(figure(1000));
str=sprintf('Time = %f',time(i));
title(str);
colorbar('southoutside');
    writeVideo(vidObj,currFrame);
%      xlim([-1.2e3 1.2e3])
%      ylim([-500 100])
%      caxis([0 1e5])
%set(gcf, 'Position', [50, 50, 700, 700])    
end
close(vidObj);
%%
%save('results/data_main.mat','slip_store_couple','slip_rate_store_couple','shear_store_couple','x','numt','time')
%% 
