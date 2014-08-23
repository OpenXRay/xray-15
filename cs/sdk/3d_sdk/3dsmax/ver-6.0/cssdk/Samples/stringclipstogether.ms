
-- This function loads each bip file in a source directory into a clip in the mixer.
-- All the clips are appended edge to edge in a single track.
-- A mixdown is performed, copied to the biped, and saved in the destination bip file.

-- It is nice to show the mixer before running this, but it is not necessary

-- bipObj - a biped object
-- sourcePath - a directory from which to load all the source bip files
-- destFile - a bip filename into which to save the mixdown
fn stringbips bipObj sourcePath destFile =
(
	local bipFiles = #() -- a list of the bip files to process
	bipFiles = getFiles (sourcePath + "\\*.bip")

	--- set the mixer up with a single trackgroup and a single track
	local bipCntrl = bipObj.transform.controller
	local mxr = bipCntrl .mixer
	for i = 1 to mxr.numtrackgroups do
	(
		deletetrackgroup mxr i
	)
	bipCntrl .mixerMode = true
	appendtrackgroup mxr 
	local tg = gettrackgroup mxr 1
	local tk = gettrack tg 1

	--- append to the track clips of all bip files in the sourcePath directory
	for i = 1 to bipFiles.count do
	(
		print bipfiles[i]
		appendclip tk bipfiles[i] false 0
	)

	--- set the animation range
	--- this is just so you can see the animation, but it's not necessary
	--- you could use themixer.setanimationRange(), but the mixer must be showing for that call to work
	cp = getclip tk 1
	start = cp.globstart
	cp = getclip tk tk.numclips
	end = cp.globend
	animationRange = interval start end
	
	-- zoom extents - only useful if you have the mixer showing
	themixer.ZoomExtents()

	--- mixdown and save the mixdown to the destination file
	mixdown mxr true true 6 true 180.0 
	copymixdowntobiped mxr 
	bipCntrl .mixermode = false
	biped.SaveBipFile bipCntrl  destFile
)
	
