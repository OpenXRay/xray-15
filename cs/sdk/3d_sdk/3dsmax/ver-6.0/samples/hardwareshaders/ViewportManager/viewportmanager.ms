--------------------------------------------
-- tests for the Shader Manager Interface
-- Neil Hazzard
-------------------------------------------

newmat = StandardMaterial()

manager = DXShaderManager.getViewportManager newmat
if(manager==null) then
	manager = DXShaderManager.addViewportManager newmat
	
effect = manager.getactiveviewporteffect()

-- Get effect directly
if(effect == null)then
	effect = manager.setviewporteffect 1		-- 1 in our case is the lightmap, 0 sets it "none"
	
-- Search for the effect based on name	
num = manager.getNumViewportEffects()

for i = 1 to num do(
	if(manager.getViewportEffectName(i) == "LightMap") then
		effect = manager.setviewporteffect i
) 

	
-- the following are hardcoded 
	
effect.lightmap_filename = GetDir(#image) + "\\testlightmap.bmp"
effect.diffuse_filename = GetDir(#image) + "\\testdiffuse.bmp"

-- set the mapping channel to use
effect.lightmap_mapping = 1
effect.diffuse_mapping = 1


$.material = newmat
redrawviews()
-- make sure the manager is active and UI is in Sync
manager.enabled = true

-- activate the hardware material
manager.activateEffect newmat true

------------------------------------------------------------------------
-- the following commands can be used to turn of the manager
-- It will not have an effect if the user is runnig in DX mode
-- dxShaderManager.SetVisible false
-- This could be place in the Medit menu bar, for example
------------------------------------------------------------------------