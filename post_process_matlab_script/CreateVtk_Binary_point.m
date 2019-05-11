%Writes the result to a .vtk file for importing into Abaqus or Paraview
dims = 2;
% folder = 'results';
% name = 'data';
% filename = [name '_result.vtk'];


FileID = fopen(filename,'w');


%Node = load([folder '/Node.txt']);
Node = [Node, zeros(size(Node,1),1)];

fprintf(FileID,'# vtk DataFile Version 2.0\n');
fprintf(FileID,'Result of analysis: %s\n', name);
fprintf(FileID,'BINARY\n');
fprintf(FileID,'DATASET UNSTRUCTURED_GRID\n');
fprintf(FileID,'POINTS %i double\n', size(Node,1));
fwrite(FileID,Node','double','ieee-be');



%Element = load([folder '/Element.txt']);


NElem = size(Element,1);
fprintf(FileID, 'CELLS %i %i\n', NElem, (2^dims+1)*NElem);
if dims == 2
  Element = [4*ones(NElem,1) Element]';
end
fwrite(FileID, Element, 'integer*4', 'ieee-be');

fprintf(FileID, 'CELL_TYPES %i\n', NElem);
type = 9*ones(NElem,1);
fwrite(FileID, type, 'integer*4', 'ieee-be');



% fileID = fopen([folder '/v_n_final.bin']);
% U = fread(fileID,'double');
% fclose(fileID);
fprintf(FileID, 'POINT_DATA %i\n', size(Node,1));

if dims == 2
    U = reshape([reshape(U,2,[]); zeros(1,size(Node,1))],[],1);
end
fprintf(FileID, 'VECTORS Velocity double\n');
fwrite(FileID, U, 'double', 'ieee-be');

fclose(FileID);
fprintf('Done\n');