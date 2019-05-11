clear all
close all
clc
%%
folder = 'results';
fileID = fopen([folder '/num_nodes_fault.bin']);
num_nodes_fault = fread(fileID,'int');
fclose(fileID);
x = load([folder '/x_fault_0.txt']);
[~,I] = sort(x);
time_data = load([folder '/time.txt']);

fileID = fopen([folder '/slip_0.bin']);
slip = fread(fileID,'double');
fclose(fileID);
fileID = fopen([folder '/slip_rate_0.bin']);
slip_rate = fread(fileID,'double'); 
fclose(fileID);
fileID = fopen([folder '/shear_0.bin']);
shear = fread(fileID,'double');
fclose(fileID);
%number of elements:
nx = length(x)-1; 
dt = time_data(2);
time_run = time_data(1);
numt = time_data(3);
time = dt*(1:1:numt);
slip_store_couple = reshape(slip,2*(nx+1),numt);
slip_rate_store_couple = reshape(slip_rate,2*(nx+1),numt);
shear_store_couple = reshape(shear,2*(nx+1),numt);
%%
vidObj = VideoWriter([folder '.mp4'],'MPEG-4');
 open(vidObj);
 fig=figure;
 set(gca,'FontSize',16)
 set(0,'defaultlinelinewidth',1)
 set(gcf,'color','w');
for k=1:10:numt
  
    subplot(3,1,1)
    slip_temp = slip_store_couple(1:2:end,k);
    plot(x(I)/1e3,slip_temp(I),'-ko')
    xlabel('x (km)')
    ylabel('Slip (m)')
    xlim([-20 20])
    title(sprintf('Time = %3.2fs', time(k)));
    subplot(3,1,2)
    slip_rate_temp = slip_rate_store_couple(1:2:end,k);
    plot(x(I)/1e3,slip_rate_temp(I),'-ko')
        xlim([-20 20])

    xlabel('x (km)')
    ylabel('Slip rate (m/s)')
    subplot(3,1,3)
    shear_temp = shear_store_couple(1:2:end,k);
    plot(x(I)/1e3,shear_temp(I)/1e6,'-ko')
        xlim([-20 20])

    xlabel('x (km)')
    ylabel('Shear (MPa)')
   % xlim([0 23])
    currFrame=getframe(fig);
    writeVideo(vidObj,currFrame);
end
close(vidObj);
%%
%save('results/data_main.mat','slip_store_couple','slip_rate_store_couple','shear_store_couple','x','numt','time')
%% 
