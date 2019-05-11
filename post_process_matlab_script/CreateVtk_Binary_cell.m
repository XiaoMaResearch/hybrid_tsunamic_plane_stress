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




%% Cell data output 
NElem = size(Element,1);
fprintf(FileID, 'CELLS %i %i\n', NElem, (2^dims+1)*NElem);
if dims == 2
  Element = [4*ones(NElem,1) Element]';
end
fwrite(FileID, Element, 'integer*4', 'ieee-be');

fprintf(FileID, 'CELL_TYPES %i\n', NElem);
type = 9*ones(NElem,1);
fwrite(FileID, type, 'integer*4', 'ieee-be');


fprintf(FileID, 'CELL_DATA %i\n', NElem);
% E
fprintf(FileID, ['SCALARS U double 1\n']);
fprintf(FileID, 'LOOKUP_TABLE default\n');
fwrite(FileID, U_temp, 'double', 'ieee-be');


fclose(FileID);
fprintf('Done\n');