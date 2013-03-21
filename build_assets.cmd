@set builder=Debug\Bloody3D.exe -a

%builder% -m data\panel_beams.hwmesh ^
	assets\panel_beams.obj panel_beams Pos3f_Norm3f_Tex2f
%builder% -m data\monkey.hwmesh ^
	assets\monkey.obj monkey Pos3f_Norm3f_Tex2f
