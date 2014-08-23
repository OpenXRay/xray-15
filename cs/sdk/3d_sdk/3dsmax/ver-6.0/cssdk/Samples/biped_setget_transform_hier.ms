-- These are the biped body parts id's.
KEY_LARM = 1
KEY_RARM = 2
KEY_LHAND = 3
KEY_RHAND = 4
KEY_LLEG = 5
KEY_RLEG = 6
KEY_LFOOT = 7
KEY_RFOOT =	8
KEY_SPINE =	9
KEY_TAIL = 10 
KEY_HEAD = 11					
KEY_PELVIS = 12		
KEY_VERTICAL = 13
KEY_HORIZONTAL = 14
KEY_TURN =	15
KEY_FOOTPRINTS = 16
KEY_NECK = 17
KEY_PONY1 = 18
KEY_PONY2 = 19
KEY_PROP1 = 20
KEY_PROP2 = 21
KEY_PROP3 = 22
KEY_LFARM = 23
KEY_RFARM =	24

--very important: this is the order of the biped nodes from the COM out to it's children.
--use this order when setting transforms
nodeOrder = #(KEY_TURN,KEY_HORIZONTAL,KEY_VERTICAL,KEY_PELVIS,KEY_LLEG,KEY_RLEG,
	KEY_LFOOT,KEY_RFOOT,KEY_SPINE,KEY_NECK,	KEY_HEAD,KEY_RARM,KEY_LARM,KEY_LHAND,KEY_RHAND,
	KEY_TAIL,KEY_PONY1,KEY_PONY2,KEY_PROP1,KEY_PROP2,KEY_PROP3,KEY_LFARM,
	KEY_RFARM,KEY_FOOTPRINTS);

-- filter function to select Biped only
-- obj - object to test class of
fn is_biped obj = 
(
	classof obj == Biped_Object
)

-- get COM position	
-- bip - bip object to get root node position of
fn GetBipedCOM_position bip =
(
	biped.GetTransform bip.controller.RootNode #pos
)

-- structure def for storage of node, node number, link number, position, and rotation
struct ObjXformData (node, nodeNum, linkNum, pos, rot, scale)

-- function to collect the biped nodes based on nodeOrder
-- bip_source - source biped
fn GetBipedStructure bip_source =
(
	-- get biped controller
	local bip = bip_source.transform.controller
	
	local bip_node_data = #()
	local nl = biped.maxNumLinks bip.RootNode
	for k=1 to nodeOrder.count do
	(
		local i = nodeOrder[k]
		local anode = biped.getNode bip i
		if anode != undefined do
		(	
			for j=1 to nl do
			(   
				local alink = biped.getNode bip i link:j
				if alink != undefined do
			   		append bip_node_data (ObjXformData alink i j)
	    	)
		)		   
	)
	bip_node_data
)

-- function to fill in the position and rotation of the node into the structure instance.
-- theDataTab - array of ObjXformData instances
fn GetTransform_Source theDataTab = 
(
	for entry in theDataTab do 
	(
		local theNode = entry.node 
		entry.pos = biped.GetTransform theNode #pos
		entry.rot = biped.GetTransform theNode #rotation
		entry.scale = biped.GetTransform theNode #scale
	)
)

-- function to set biped transforms
-- bip_target - target biped object
-- ObjXformDataTab - array of node data
-- com_delta - relative COM Point3 value
-- setKeys - if true, set keys
-- prog_item - progress bar UI item
fn SetTransform_Target bip_target ObjXformDataTab com_delta setKeys prog_item = 
(
	local j = 0
	toolmode.CommandMode = #select
	for entry in ObjXformDataTab do
	(	local targetNode = biped.getNode bip_target entry.nodeNum link:entry.linkNum
		prog_item.value = (j+=1)*100/ObjXformDataTab.count
		if (bip_target.controller.figureMode) do 
		(	biped.SetTransform targetNode #scale entry.scale false
			setKeys = false
		)
		local setPos = false, setRot = false
		case entry.nodeNum of
		(
			-- com
			KEY_VERTICAL:	setPos = true
			KEY_HORIZONTAL:	setPos = true
			KEY_TURN: 		setRot = true
			-- footstep
			KEY_FOOTPRINTS: ()
			-- props
			KEY_PROP1: setPos = setRot = true
			KEY_PROP2: setPos = setRot = true
			KEY_PROP3: setPos = setRot = true
			-- forearms
			KEY_LFARM: ()
			KEY_RFARM: ()
			-- everything else
			default: (setRot = true; if (bip_target.controller.figureMode) do setPos = true)
		)
		if setPos do biped.SetTransform targetNode #pos (entry.pos+com_delta) setKeys
		if setRot do biped.SetTransform targetNode #rotation entry.rot setKeys
	)
)


-- verify bipeds are the same - can not copy bipeds with different nodes and links
-- bip_source - source biped
-- bip_target - target biped
fn MatchBipeds bip_source bip_target = 
(
	sc = bip_source.controller
	tc = bip_target.controller

	-- collect nodes
	sc_node = #()
	append sc_node sc.arms; append sc_node sc.neckLinks; append sc_node sc.spineLinks; append sc_node sc.legLinks; \
	append sc_node sc.tailLinks; append sc_node sc.ponytail1Links; append sc_node sc.ponytail2Links; append sc_node sc.fingers; \
	append sc_node sc.fingerLinks; append sc_node sc.toes; append sc_node sc.toeLinks	
	append sc_node sc.prop1Exists; append sc_node sc.prop2Exists; append sc_node sc.prop3Exists	
	append sc_node sc.forearmTwistLinks	
	
	tc_node = #()
	append tc_node tc.arms; append tc_node tc.neckLinks; append tc_node tc.spineLinks; append tc_node tc.legLinks; \
	append tc_node tc.tailLinks; append tc_node tc.ponytail1Links; append tc_node tc.ponytail2Links; append tc_node tc.fingers; \
    append tc_node tc.fingerLinks; append tc_node tc.toes; append tc_node tc.toeLinks	
	append tc_node tc.prop1Exists; append tc_node tc.prop2Exists; append tc_node tc.prop3Exists	
	append tc_node tc.forearmTwistLinks	
	
	
	msgtext = "Can not copy different bipeds\n\n                    Source     Target\n\n" +\
	"Arms:              "+(sc_node[1]  as string)+"              "+(tc_node[1]  as string)+"\n" +\
	"neckLinks:         "+(sc_node[2]  as string)+"              "+(tc_node[2]  as string)+"\n" +\
	"spineLinks:        "+(sc_node[3]  as string)+"              "+(tc_node[3]  as string)+"\n" +\
	"legLinks:          "+(sc_node[4]  as string)+"              "+(tc_node[4]  as string)+"\n" +\
	"tailLinks:         "+(sc_node[5]  as string)+"              "+(tc_node[5]  as string)+"\n" +\
	"ponytail1Links:    "+(sc_node[6]  as string)+"              "+(tc_node[6]  as string)+"\n" +\
	"ponytail2Links:    "+(sc_node[7]  as string)+"              "+(tc_node[7]  as string)+"\n" +\
	"fingers:           "+(sc_node[8]  as string)+"              "+(tc_node[8]  as string)+"\n" +\
	"fingerLinks:       "+(sc_node[9]  as string)+"              "+(tc_node[9]  as string)+"\n" +\
	"toes:              "+(sc_node[10] as string)+"              "+(tc_node[10] as string)+"\n" +\
	"toeLinks:          "+(sc_node[11] as string)+"              "+(tc_node[11] as string)+"\n" +\
	"prop1:             "+(sc_node[12] as string)+"              "+(tc_node[12] as string)+"\n" +\
	"prop2:             "+(sc_node[13] as string)+"              "+(tc_node[13] as string)+"\n" +\
	"prop3:             "+(sc_node[14] as string)+"              "+(tc_node[14] as string)+"\n" +\
	"forearmTwistLinks: "+(sc_node[15] as string)+"              "+(tc_node[15] as string)+"\n"

	for i=1 to sc_node.count do
	(
		if sc_node[i] != tc_node[i] do
		(
			messagebox msgtext  title: "Error: Bipeds are different" beep:false
			return false
		)	
	)
	return true
)

-- function to copy biped node transforms
-- bip_source - source biped
-- bip_target - target biped
-- start - start frame
-- end - end frame
-- deltaT - frame step rate
-- setKeys - if true, set keys
-- initialOffset - if true, maintain offset of bipeds' COMs at start frame
-- useFixedHier - if true, use fixed (hard coded) hierarchy
-- prog_item - per biped node progress bar UI item
-- prog_time - per frame progress bar UI item
fn Copy_all bip_source bip_target start end deltaT setKeys initialOffset prog_item prog_time =
(
	local com_delta
	if initialOffset then
		com_delta = at time start (GetBipedCOM_position bip_target) - (GetBipedCOM_position bip_source)
	else
		com_delta = [0,0,0]
		
	local ObjXformDataTab = GetBipedStructure bip_source 

	if (bip_target.controller.figureMode) do bip_target.controller.height = bip_source.controller.height 
	for i = start to (end-1) by deltaT do at time i
	(	--format "%\n" i
		prog_time.value = (i-start)*100/(end-start+1)
		GetTransform_Source ObjXformDataTab 
		SetTransform_Target bip_target ObjXformDataTab com_delta setKeys prog_item 
	)
	i = end
	at time i
	(	--format "%\n" i
		prog_time.value = (i-start)*100/(end-start+1)
		GetTransform_Source ObjXformDataTab 
		SetTransform_Target bip_target ObjXformDataTab com_delta setKeys prog_item 
	)
)


rollout _Options "Biped:  Copy Transform" width: 150
(
	
	local bip_source = undefined, bip_target = undefined

	-- UI elements
	group "Biped: Source" 
	(
		pickbutton _source "Pick" filter:is_biped width:100 height:20
	)
	
	group "Biped: Target" 
	(
		pickbutton _target "Pick" filter:is_biped width:100 height:20
	)
	
	group "Animation" 
	(
	    spinner _start "Start Time: " range:[-100000, 100000, 0] type: #integer fieldWidth: 60
		spinner _end "End Time: " range:[-100000, 100000, 0] type: #integer fieldWidth: 60
		spinner _nth "Nth Frame: " range:[1, 100000, 1] type: #integer fieldWidth: 60
		checkbox _setKeys "set keys"
		checkbox _offset "maintain initial offset"
	)

	Button _copy "Copy" width:80 height:20
	Button _help "Help" width:60 height:20
	label prog_item_label "Obj:" align:#left across:2 offset:[-10,0]
	progressbar prog_item align:#right width:110 offset:[8,0]
	label prog_time_label "Time:" align:#left across:2 offset:[-10,0]
	progressbar prog_time align:#right width:110 offset:[8,0]
	
	on _source picked obj do
	(
		bip_source = obj.controller.RootNode
		_source.text = bip_source.name
	)
	
	on _target picked obj do
	(
		bip_target = obj.controller.RootNode
		_target.text = bip_target.name
	)
	
	on _start changed val do
	(
		if _start.value < animationrange.start then
			_start.value = animationrange.start
		if _start.value > animationrange.end then
			_start.value = animationrange.end
		if _start.value > _end.value then
			_end.value = _start.value
	)

	on _end changed val do
	(
		if _end.value > animationrange.end then
			_end.value = animationrange.end
		if _end.value < animationrange.start then
			_end.value = _start.value
		if _start.value > _end.value then
			_start.value = _end.value
	)
	
	on _copy pressed do with undo "Set Poses" on 
	(
		if (bip_source != undefined and (IsDeleted bip_source)) == true then
  			bip_source = undefined
		if (bip_target != undefined and (IsDeleted bip_target)) == true then
  			bip_target = undefined
		if (bip_source != undefined and bip_target != undefined) then
		(
			if ((MatchBipeds bip_source bip_target)!= false) then
				Copy_all bip_source bip_target _start.value _end.value _nth.value _setkeys.state _offset.state prog_item prog_time
			prog_item.value = 0
			prog_time.value = 0
		)
		else
			messageBox ("Bipeds are not selected\n or have been deleted") title:"Selection Error" beep:false
	)
	
	on _help pressed do
	(
		messagebox "\nBiped Copy Transform allows you to copy \nbiped nodes transform in specified time.\n\nCopied bipeds MUST have the same structure. \nError message shows up to describe any\ndifference\n" beep:false
	)


)

CreateDialog _Options
