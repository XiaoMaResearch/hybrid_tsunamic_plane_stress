function handle = PlotMesh_el(Node,Element,Cdata_mat)
% Initial configuration
if iscell(Element)
    MaxNVer = max(cellfun(@numel,Element));      %Max. num. of vertices in mesh
    PadWNaN = @(E) [E NaN(1,MaxNVer-numel(E))];  %Pad cells with NaN
    Element = cellfun(PadWNaN,Element,'UniformOutput',false);
    Element = vertcat(Element);               %Create padded element matrix
end

%NElem = numel(Element);
NElem = size(Element,1);


figure(1000)
handle = patch('Faces',Element,'Vertices',Node,'FaceColor','flat','CData',Cdata_mat,'FaceAlpha',1.0,'EdgeColor','none');
axis equal; 
%axis off; hold on;
% colorbar()
%patch('Faces',ElemMat,'Vertices',Node,'FaceColor','k','FaceAlpha',1.0);
% % Final configuration
% Node = reshape(Curcoord,2,[])';
% plot(Node(:,1),Node(:,2),'ks','LineWidth',1,...
%         'MarkerFaceColor',[1 .5 1],'MarkerSize',8);
% patch('Faces',ElemMat,'Vertices',Node,'FaceColor','r','FaceAlpha',0.1);