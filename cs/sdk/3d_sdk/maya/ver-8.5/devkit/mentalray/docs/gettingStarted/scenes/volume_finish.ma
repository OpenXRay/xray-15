//Maya ASCII 6.0ff11 scene
//Name: volume_finish.ma
//Last modified: Thu, Feb 05, 2004 02:05:31 PM
requires maya "6.0ff11";
requires "Mayatomr" "6.0.1.9m - 3.3.0.655 ";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
fileInfo "product" "Maya Unlimited 6.0 Beta";
fileInfo "version" "6.0Beta3";
fileInfo "cutIdentifier" "200402032210-612277";
fileInfo "osv" "Microsoft Windows XP Service Pack 1 (Build 2600)\n";
createNode transform -s -n "persp";
	setAttr ".v" no;
	setAttr ".t" -type "double3" -23.619826522627271 10.361731316891644 -0.43214923584797449 ;
	setAttr ".r" -type "double3" -11.738352729566859 -90.999999999994841 5.0888874903416268e-014 ;
createNode camera -s -n "perspShape" -p "persp";
	setAttr -k off ".v" no;
	setAttr ".fl" 34.999999999999993;
	setAttr ".ncp" 0.001;
	setAttr ".coi" 24.128229804582691;
	setAttr ".imn" -type "string" "persp";
	setAttr ".den" -type "string" "persp_depth";
	setAttr ".man" -type "string" "persp_mask";
	setAttr ".tp" -type "double3" 0.00020797999999988548 5.4530185341497415 
		-0.019859999999999989 ;
	setAttr ".hc" -type "string" "viewSet -p %camera";
createNode transform -s -n "top";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 100 0 ;
	setAttr ".r" -type "double3" -89.999999999999986 0 0 ;
createNode camera -s -n "topShape" -p "top";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".ncp" 0.001;
	setAttr ".coi" 100;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "top";
	setAttr ".den" -type "string" "top_depth";
	setAttr ".man" -type "string" "top_mask";
	setAttr ".hc" -type "string" "viewSet -t %camera";
	setAttr ".o" yes;
createNode transform -s -n "front";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 0 100 ;
createNode camera -s -n "frontShape" -p "front";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".ncp" 0.001;
	setAttr ".coi" 100;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "front";
	setAttr ".den" -type "string" "front_depth";
	setAttr ".man" -type "string" "front_mask";
	setAttr ".hc" -type "string" "viewSet -f %camera";
	setAttr ".o" yes;
createNode transform -s -n "side";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 100 4.7470398277717969 -0.67814854682454273 ;
	setAttr ".r" -type "double3" 0 89.999999999999986 0 ;
createNode camera -s -n "sideShape" -p "side";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".ncp" 0.001;
	setAttr ".coi" 100;
	setAttr ".ow" 19.537136706135623;
	setAttr ".imn" -type "string" "side";
	setAttr ".den" -type "string" "side_depth";
	setAttr ".man" -type "string" "side_mask";
	setAttr ".hc" -type "string" "viewSet -s %camera";
	setAttr ".o" yes;
createNode transform -n "vase";
	setAttr ".s" -type "double3" 1.1423030861549477 1.1423030861549477 1.1423030861549477 ;
createNode nurbsSurface -n "vaseShape" -p "vase";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".dvu" 0;
	setAttr ".dvv" 0;
	setAttr ".cpr" 4;
	setAttr ".cps" 4;
	setAttr ".cc" -type "nurbsSurface" 
		3 3 0 2 no 
		28 5 5 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
		 27 28 28 28
		13 -2 -1 0 1 2 3 4 5 6 7 8 9 10
		
		286
		-2.5666328965245668 9.552460426745526 -2.5666328965245628
		4.1418536309045842e-016 9.552460426745526 -3.6297670518979799
		2.5666328965245642 9.552460426745526 -2.5666328965245642
		3.6297670518979799 9.552460426745526 -1.13128985698164e-015
		2.5666328965245651 9.552460426745526 2.5666328965245637
		1.2222001640447046e-015 9.552460426745526 3.6297670518979808
		-2.5666328965245633 9.552460426745526 2.5666328965245646
		-3.6297670518979799 9.552460426745526 1.9494826205492217e-015
		-2.5666328965245668 9.552460426745526 -2.5666328965245628
		4.1418536309045842e-016 9.552460426745526 -3.6297670518979799
		2.5666328965245642 9.552460426745526 -2.5666328965245642
		-2.5891807858136913 9.502931602004935 -2.5891755370300347
		-3.7114505127746032e-006 9.502931602004935 -3.6616508712830331
		2.5891755370300364 9.502931602004935 -2.5891807858136886
		3.6616508712830331 9.502931602004935 -3.7114505143336537e-006
		2.589180785813689 9.502931602004935 2.589175537030036
		3.7114505144253637e-006 9.502931602004935 3.661650871283034
		-2.5891755370300351 9.502931602004935 2.589180785813689
		-3.6616508712830331 9.502931602004935 3.7114505151590335e-006
		-2.5891807858136913 9.502931602004935 -2.5891755370300347
		-3.7114505127746032e-006 9.502931602004935 -3.6616508712830331
		2.5891755370300364 9.502931602004935 -2.5891807858136886
		-2.6478579088211633 9.3783051721860069 -2.6478603447901299
		1.7224901781036273e-006 9.3783051721860069 -3.744638288381926
		2.6478603447901312 9.3783051721860069 -2.6478579088211607
		3.744638288381926 9.3783051721860069 1.7224901765092423e-006
		2.6478579088211616 9.3783051721860069 2.6478603447901308
		-1.7224901764154554e-006 9.3783051721860069 3.7446382883819269
		-2.6478603447901303 9.3783051721860069 2.6478579088211611
		-3.744638288381926 9.3783051721860069 -1.7224901756651562e-006
		-2.6478579088211633 9.3783051721860069 -2.6478603447901299
		1.7224901781036273e-006 9.3783051721860069 -3.744638288381926
		2.6478603447901312 9.3783051721860069 -2.6478579088211607
		-1.8530136637292396 8.9744798322148061 -1.8530130110133185
		-4.6153985177994707e-007 8.9744798322148061 -2.6205565929686943
		1.8530130110133196 8.9744798322148061 -1.8530136637292378
		2.6205565929686943 8.9744798322148061 -4.6153985289572261e-007
		1.8530136637292383 8.9744798322148061 1.8530130110133194
		4.615398529613566e-007 8.9744798322148061 2.6205565929686947
		-1.8530130110133187 8.9744798322148061 1.853013663729238
		-2.6205565929686943 8.9744798322148061 4.6153985348642734e-007
		-1.8530136637292396 8.9744798322148061 -1.8530130110133185
		-4.6153985177994707e-007 8.9744798322148061 -2.6205565929686943
		1.8530130110133196 8.9744798322148061 -1.8530136637292378
		-1.879173558545828 9.0101767234390717 -1.8791737334405283
		1.2366923094252464e-007 9.0101767234390717 -2.6575528562176496
		1.8791737334405294 9.0101767234390717 -1.8791735585458262
		2.6575528562176496 9.0101767234390717 1.2366922981099691e-007
		1.8791735585458267 9.0101767234390717 1.8791737334405292
		-1.2366922974443648e-007 9.0101767234390717 2.6575528562176505
		-1.8791737334405285 9.0101767234390717 1.8791735585458265
		-2.6575528562176496 9.0101767234390717 -1.2366922921195282e-007
		-1.879173558545828 9.0101767234390717 -1.8791737334405283
		1.2366923094252464e-007 9.0101767234390717 -2.6575528562176496
		1.8791737334405294 9.0101767234390717 -1.8791735585458262
		-1.0862162566898617 8.0093972739661474 -1.0862162098269654
		-3.31370703031256e-008 8.0093972739661474 -1.5361417287438657
		1.0862162098269661 8.0093972739661474 -1.0862162566898605
		1.5361417287438657 8.0093972739661474 -3.3137070957181062e-008
		1.0862162566898608 8.0093972739661474 1.0862162098269659
		3.313707099565492e-008 8.0093972739661474 1.5361417287438659
		-1.0862162098269657 8.0093972739661474 1.0862162566898608
		-1.5361417287438657 8.0093972739661474 3.3137071303445722e-008
		-1.0862162566898617 8.0093972739661474 -1.0862162098269654
		-3.31370703031256e-008 8.0093972739661474 -1.5361417287438657
		1.0862162098269661 8.0093972739661474 -1.0862162566898605
		-1.2655616319209289 7.8128213370224753 -1.2655616444778015
		8.8790514784114805e-009 7.8128213370224753 -1.7897744327606542
		1.2655616444778022 7.8128213370224753 -1.2655616319209275
		1.7897744327606542 7.8128213370224753 8.8790507163647717e-009
		1.265561631920928 7.8128213370224753 1.2655616444778022
		-8.8790506715384961e-009 7.8128213370224753 1.7897744327606546
		-1.2655616444778017 7.8128213370224753 1.2655616319209277
		-1.7897744327606542 7.8128213370224753 -8.8790503129282795e-009
		-1.2655616319209289 7.8128213370224753 -1.2655616444778015
		8.8790514784114805e-009 7.8128213370224753 -1.7897744327606542
		1.2655616444778022 7.8128213370224753 -1.2655616319209275
		-1.3411374342870457 7.5809969736795173 -1.3411374309224391
		-2.3791344020911618e-009 7.5809969736795173 -1.8966547461958594
		1.3411374309224398 7.5809969736795173 -1.3411374342870443
		1.8966547461958594 7.5809969736795173 -2.3791352096451644e-009
		1.3411374342870448 7.5809969736795173 1.3411374309224395
		2.3791352571483421e-009 7.5809969736795173 1.8966547461958596
		-1.3411374309224393 7.5809969736795173 1.3411374342870446
		-1.8966547461958594 7.5809969736795173 2.3791356371737544e-009
		-1.3411374342870457 7.5809969736795173 -1.3411374309224391
		-2.3791344020911618e-009 7.5809969736795173 -1.8966547461958594
		1.3411374309224398 7.5809969736795173 -1.3411374342870443
		-1.45275363810487 7.4430531386411829 -1.4527536390064104
		6.3748743410527873e-010 7.4430531386411829 -2.0545038984322477
		1.4527536390064113 7.4430531386411829 -1.4527536381048685
		2.0545038984322477 7.4430531386411829 6.3748655934256834e-010
		1.4527536381048689 7.4430531386411829 1.4527536390064111
		-6.3748650788593832e-010 7.4430531386411829 2.0545038984322481
		-1.4527536390064106 7.4430531386411829 1.4527536381048687
		-2.0545038984322477 7.4430531386411829 -6.3748609623289804e-010
		-1.45275363810487 7.4430531386411829 -1.4527536390064104
		6.3748743410527873e-010 7.4430531386411829 -2.0545038984322477
		1.4527536390064113 7.4430531386411829 -1.4527536381048685
		-1.7217327376151201 7.2171990618461246 -1.7217327373735498
		-1.7081390254896301e-010 7.2171990618461246 -2.4348977881462441
		1.7217327373735507 7.2171990618461246 -1.7217327376151184
		2.4348977881462441 7.2171990618461246 -1.708149392750507e-010
		1.7217327376151188 7.2171990618461246 1.7217327373735505
		1.7081500025893826e-010 7.2171990618461246 2.4348977881462446
		-1.72173273737355 7.2171990618461246 1.7217327376151186
		-2.4348977881462441 7.2171990618461246 1.7081548813003832e-010
		-1.7217327376151201 7.2171990618461246 -1.7217327373735498
		-1.7081390254896301e-010 7.2171990618461246 -2.4348977881462441
		1.7217327373735507 7.2171990618461246 -1.7217327376151184
		-2.0173621051874617 6.9967419786954332 -2.0173621052521864
		4.5769847176428264e-011 6.9967419786954332 -2.852980849419414
		2.0173621052521873 6.9967419786954332 -2.0173621051874595
		2.852980849419414 6.9967419786954332 4.5768632439740457e-011
		2.0173621051874604 6.9967419786954332 2.0173621052521873
		-4.5768560984641185e-011 6.9967419786954332 2.8529808494194144
		-2.0173621052521868 6.9967419786954332 2.01736210518746
		-2.852980849419414 6.9967419786954332 -4.5767989343846918e-011
		-2.0173621051874617 6.9967419786954332 -2.0173621052521864
		4.5769847176428264e-011 6.9967419786954332 -2.852980849419414
		2.0173621052521873 6.9967419786954332 -2.0173621051874595
		-2.5434148279626418 6.3378802725759416 -2.5434148279452944
		-1.2263495997845033e-011 6.3378802725759416 -3.5969317444333333
		2.5434148279452957 6.3378802725759416 -2.5434148279626392
		3.5969317444333333 6.3378802725759416 -1.2265027492511172e-011
		2.5434148279626401 6.3378802725759416 2.5434148279452953
		1.2265117580432712e-011 6.3378802725759416 3.5969317444333342
		-2.5434148279452948 6.3378802725759416 2.5434148279626396
		-3.5969317444333333 6.3378802725759416 1.2265838283805012e-011
		-2.5434148279626418 6.3378802725759416 -2.5434148279452944
		-1.2263495997845033e-011 6.3378802725759416 -3.5969317444333333
		2.5434148279452957 6.3378802725759416 -2.5434148279626392
		-2.7143886496368528 6.0392411167810733 -2.7143886496414957
		3.2865417688474432e-012 6.0392411167810733 -3.8387252418713116
		2.7143886496414971 6.0392411167810733 -2.7143886496368501
		3.8387252418713116 6.0392411167810733 3.2849073238119486e-012
		2.714388649636851 6.0392411167810733 2.7143886496414966
		-3.2848111799863315e-012 6.0392411167810733 3.838725241871312
		-2.7143886496414962 6.0392411167810733 2.7143886496368506
		-3.8387252418713116 6.0392411167810733 -3.2840420293813925e-012
		-2.7143886496368528 6.0392411167810733 -2.7143886496414957
		3.2865417688474432e-012 6.0392411167810733 -3.8387252418713116
		2.7143886496414971 6.0392411167810733 -2.7143886496368501
		-2.8887251449672053 5.4952067178782142 -2.8887251449659557
		-8.800427725061135e-013 5.4952067178782142 -4.0852742779799227
		2.8887251449659574 5.4952067178782142 -2.8887251449672027
		4.0852742779799227 5.4952067178782142 -8.8178219271505353e-013
		2.8887251449672031 5.4952067178782142 2.888725144965957
		8.8188451155087361e-013 5.4952067178782142 4.0852742779799236
		-2.8887251449659561 5.4952067178782142 2.8887251449672031
		-4.0852742779799227 5.4952067178782142 8.8270306223743348e-013
		-2.8887251449672053 5.4952067178782142 -2.8887251449659557
		-8.800427725061135e-013 5.4952067178782142 -4.0852742779799227
		2.8887251449659574 5.4952067178782142 -2.8887251449672027
		-3.108057452892969 4.4368952342613968 -3.108057452893298
		2.3643311640187554e-013 4.4368952342613968 -4.3954570025162463
		3.1080574528932998 4.4368952342613968 -3.1080574528929659
		4.3954570025162463 4.4368952342613968 2.3456162719026646e-013
		3.1080574528929668 4.4368952342613968 3.1080574528932994
		-2.3445153958958359e-013 4.4368952342613968 4.3954570025162472
		-3.1080574528932985 4.4368952342613968 3.1080574528929663
		-4.3954570025162463 4.4368952342613968 -2.3357083878412047e-013
		-3.108057452892969 4.4368952342613968 -3.108057452893298
		2.3643311640187554e-013 4.4368952342613968 -4.3954570025162463
		3.1080574528932998 4.4368952342613968 -3.1080574528929659
		-2.9462889074293623 3.2836345817671337 -2.9462889074327552
		2.4025421578286574e-012 3.2836345817671337 -4.1666817315584117
		2.9462889074327565 3.2836345817671337 -2.9462889074293592
		4.1666817315584117 3.2836345817671337 2.4007680761110213e-012
		2.9462889074293601 3.2836345817671337 2.9462889074327565
		-2.4006637183629258e-012 3.2836345817671337 4.1666817315584126
		-2.9462889074327556 3.2836345817671337 2.9462889074293597
		-4.1666817315584117 3.2836345817671337 -2.3998288563781547e-012
		-2.9462889074293623 3.2836345817671337 -2.9462889074327552
		2.4025421578286574e-012 3.2836345817671337 -4.1666817315584117
		2.9462889074327565 3.2836345817671337 -2.9462889074293592
		-2.6818885273137751 2.5197207908397901 -2.6818885273128603
		-6.4319913792034157e-013 2.5197207908397901 -3.7927631280993004
		2.6818885273128621 2.5197207908397901 -2.6818885273137725
		3.7927631280993004 2.5197207908397901 -6.448140132956127e-013
		2.6818885273137729 2.5197207908397901 2.6818885273128616
		6.4490900596474644e-013 2.5197207908397901 3.7927631280993013
		-2.6818885273128608 2.5197207908397901 2.6818885273137729
		-3.7927631280993004 2.5197207908397901 6.4566894731781508e-013
		-2.6818885273137751 2.5197207908397901 -2.6818885273128603
		-6.4319913792034157e-013 2.5197207908397901 -3.7927631280993004
		2.6818885273128621 2.5197207908397901 -2.6818885273137725
		-2.5169740826829954 2.1860945963367451 -2.5169740826832356
		1.7286674523737425e-013 2.1860945963367451 -3.5595388838720416
		2.516974082683237 2.1860945963367451 -2.5169740826829927
		3.5595388838720416 2.1860945963367451 1.7135117163134055e-013
		2.5169740826829936 2.1860945963367451 2.5169740826832365
		-1.7126202024275036e-013 2.1860945963367451 3.5595388838720421
		-2.5169740826832361 2.1860945963367451 2.5169740826829932
		-3.5595388838720416 2.1860945963367451 -1.705488091340286e-013
		-2.5169740826829954 2.1860945963367451 -2.5169740826832356
		1.7286674523737425e-013 2.1860945963367451 -3.5595388838720416
		2.516974082683237 2.1860945963367451 -2.5169740826829927
		-2.1556251962559081 1.3823335525726055 -2.1556251962558388
		-4.5862889137362176e-014 1.3823335525726055 -3.0485143879382206
		2.1556251962558401 1.3823335525726055 -2.1556251962559059
		3.0485143879382206 1.3823335525726055 -4.7160879718363294e-014
		2.1556251962559063 1.3823335525726055 2.1556251962558397
		4.7237232105481017e-014 1.3823335525726055 3.048514387938221
		-2.1556251962558393 1.3823335525726055 2.1556251962559059
		-3.0485143879382206 1.3823335525726055 4.7848051202422709e-014
		-2.1556251962559081 1.3823335525726055 -2.1556251962558388
		-4.5862889137362176e-014 1.3823335525726055 -3.0485143879382206
		2.1556251962558401 1.3823335525726055 -2.1556251962559059
		-1.7883858923136231 0.89318880530139055 -1.7883858923136378
		1.2670692138839909e-014 0.89318880530139055 -2.5291595836666456
		1.7883858923136386 0.89318880530139055 -1.7883858923136213
		2.5291595836666456 0.89318880530139055 1.1593831445501253e-014
		1.788385892313622 0.89318880530139055 1.7883858923136384
		-1.1530486698834277e-014 0.89318880530139055 2.5291595836666461
		-1.788385892313638 0.89318880530139055 1.7883858923136216
		-2.5291595836666456 0.89318880530139055 -1.1023728725498437e-014
		-1.7883858923136231 0.89318880530139055 -1.7883858923136378
		1.2670692138839909e-014 0.89318880530139055 -2.5291595836666456
		1.7883858923136386 0.89318880530139055 -1.7883858923136213
		-1.4433877732104061 0.49898577940644978 -1.443387773210399
		-3.0849789623709069e-015 0.49898577940644978 -2.0412585646376522
		1.4433877732103999 0.49898577940644978 -1.4433877732104048
		2.0412585646376522 0.49898577940644978 -3.9541021001099677e-015
		1.4433877732104052 0.49898577940644978 1.4433877732103997
		4.0052269905652074e-015 0.49898577940644978 2.0412585646376527
		-1.4433877732103992 0.49898577940644978 1.443387773210405
		-2.0412585646376522 0.49898577940644978 4.4142261142071173e-015
		-1.4433877732104061 0.49898577940644978 -1.443387773210399
		-3.0849789623709069e-015 0.49898577940644978 -2.0412585646376522
		1.4433877732103999 0.49898577940644978 -1.4433877732104048
		-1.0153153335425309 0.041308184576415649 -1.0153153335425307
		1.053143795336538e-015 0.041308184576415649 -1.4358727147812096
		1.0153153335425313 0.041308184576415649 -1.01531533354253
		1.4358727147812096 0.041308184576415649 4.4178067805008398e-016
		1.0153153335425302 0.041308184576415649 1.0153153335425311
		-4.0581814173911624e-016 0.041308184576415649 1.43587271478121
		-1.0153153335425309 0.041308184576415649 1.01531533354253
		-1.4358727147812096 0.041308184576415649 -1.1811785125137294e-016
		-1.0153153335425309 0.041308184576415649 -1.0153153335425307
		1.053143795336538e-015 0.041308184576415649 -1.4358727147812096
		1.0153153335425313 0.041308184576415649 -1.01531533354253
		-0.79842152790021914 -0.0050494108042369989 -0.79842152790021748
		-1.106101582541708e-016 -0.0050494108042369989 -1.1291385532471372
		0.79842152790021792 -0.0050494108042369989 -0.79842152790021836
		1.1291385532471372 -0.0050494108042369989 -5.9137259538055134e-016
		0.79842152790021859 -0.0050494108042369989 0.79842152790021781
		6.1965273874092674e-016 -0.0050494108042369989 1.1291385532471374
		-0.79842152790021759 -0.0050494108042369989 0.79842152790021848
		-1.1291385532471372 -0.0050494108042369989 8.4589388562392924e-016
		-0.79842152790021914 -0.0050494108042369989 -0.79842152790021748
		-1.106101582541708e-016 -0.0050494108042369989 -1.1291385532471372
		0.79842152790021792 -0.0050494108042369989 -0.79842152790021836
		-0.51202975802062667 0.0071499574465284802 -0.51202975802062578
		1.5102444691438395e-016 0.0071499574465284802 -0.72411942813138364
		0.51202975802062611 0.0071499574465284802 -0.51202975802062611
		0.72411942813138364 0.0071499574465284802 -1.5728972767284143e-016
		0.51202975802062622 0.0071499574465284802 0.512029758020626
		1.7542585558973706e-016 0.0071499574465284802 0.72411942813138375
		-0.51202975802062589 0.0071499574465284802 0.51202975802062622
		-0.72411942813138364 0.0071499574465284802 3.2051487892490192e-016
		-0.51202975802062667 0.0071499574465284802 -0.51202975802062578
		1.5102444691438395e-016 0.0071499574465284802 -0.72411942813138364
		0.51202975802062611 0.0071499574465284802 -0.51202975802062611
		-0.16788913153322388 0.0055233801204496707 -0.16788913153322363
		2.7088528178654538e-017 0.0055233801204496707 -0.23743108678932548
		0.16788913153322371 0.0055233801204496707 -0.16788913153322371
		0.23743108678932548 0.0055233801204496707 -7.4004422377447376e-017
		0.16788913153322377 0.0055233801204496707 0.16788913153322368
		7.9951066527806319e-017 0.0055233801204496707 0.23743108678932554
		-0.16788913153322366 0.0055233801204496707 0.16788913153322374
		-0.23743108678932548 0.0055233801204496707 1.2752421973067781e-016
		-0.16788913153322388 0.0055233801204496707 -0.16788913153322363
		2.7088528178654538e-017 0.0055233801204496707 -0.23743108678932548
		0.16788913153322371 0.0055233801204496707 -0.16788913153322371
		0.0041811820442009019 0.0047100890919436356 0.0041811820442008958
		-6.7462417960037443e-019 0.0047100890919436356 0.0059130843536597723
		-0.0041811820442008976 0.0047100890919436356 0.0041811820442008976
		-0.0059130843536597723 0.0047100890919436356 1.8430374808080387e-018
		-0.0041811820442008993 0.0047100890919436356 -0.0041811820442008976
		-1.9911352255379455e-018 0.0047100890919436356 -0.0059130843536597741
		0.0041811820442008958 0.0047100890919436356 -0.0041811820442008985
		0.0059130843536597723 0.0047100890919436356 -3.1759171833771985e-018
		0.0041811820442009019 0.0047100890919436356 0.0041811820442008958
		-6.7462417960037443e-019 0.0047100890919436356 0.0059130843536597723
		-0.0041811820442008976 0.0047100890919436356 0.0041811820442008976
		
		;
createNode transform -n "pSphere1";
	setAttr ".t" -type "double3" 1.7223394553968658 4.6023658290285372 -0.87544538535469707 ;
	setAttr ".s" -type "double3" 1.2526009466337538 1.2526009466337538 1.2526009466337538 ;
createNode mesh -n "pSphereShape1" -p "pSphere1";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
createNode transform -n "pointLight1";
	setAttr ".t" -type "double3" -7.5564120516489606 15.284857674716804 8.9850484168889686 ;
createNode pointLight -n "pointLightShape1" -p "pointLight1";
	setAttr -k off ".v";
	setAttr ".in" 7;
	setAttr ".us" no;
createNode transform -n "pSphere2";
	setAttr ".t" -type "double3" 0 6.12295258733901 2.5204388261140163 ;
createNode mesh -n "pSphereShape2" -p "pSphere2";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
createNode transform -n "pSphere3";
	setAttr ".t" -type "double3" -1.4727667185617688 3.8887495636733274 0.070383107318235716 ;
	setAttr ".s" -type "double3" 1.8868324980399982 1.8868324980399982 1.8868324980399982 ;
createNode mesh -n "pSphereShape3" -p "pSphere3";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
createNode lightLinker -n "lightLinker1";
	setAttr -s 6 ".lnk";
createNode displayLayerManager -n "layerManager";
createNode displayLayer -n "defaultLayer";
createNode renderLayerManager -n "renderLayerManager";
createNode renderLayer -n "defaultRenderLayer";
createNode renderLayer -s -n "globalRender";
createNode mentalrayItemsList -s -n "mentalrayItemsList";
	setAttr -s 11 ".opt";
createNode mentalrayOptions -s -n "Draft";
	setAttr ".maxr" 2;
createNode mentalrayOptions -s -n "DraftMotionBlur";
	setAttr ".maxr" 2;
	setAttr ".mb" 1;
	setAttr ".tconr" 1;
	setAttr ".tcong" 1;
	setAttr ".tconb" 1;
	setAttr ".tcona" 1;
createNode mentalrayOptions -s -n "PreviewMotionblur";
	setAttr ".minsp" -1;
	setAttr ".maxsp" 1;
	setAttr ".fil" 1;
	setAttr ".rflr" 2;
	setAttr ".rfrr" 2;
	setAttr ".maxr" 4;
	setAttr ".mb" 1;
	setAttr ".tconr" 0.5;
	setAttr ".tcong" 0.5;
	setAttr ".tconb" 0.5;
	setAttr ".tcona" 0.5;
createNode mentalrayOptions -s -n "PreviewCaustics";
	setAttr ".minsp" -1;
	setAttr ".maxsp" 1;
	setAttr ".fil" 1;
	setAttr ".rflr" 2;
	setAttr ".rfrr" 2;
	setAttr ".maxr" 4;
	setAttr ".ca" yes;
	setAttr ".cc" 1;
	setAttr ".cr" 1;
createNode mentalrayOptions -s -n "PreviewGlobalIllum";
	setAttr ".minsp" -1;
	setAttr ".maxsp" 1;
	setAttr ".fil" 1;
	setAttr ".rflr" 2;
	setAttr ".rfrr" 2;
	setAttr ".maxr" 4;
	setAttr ".gi" yes;
	setAttr ".gc" 1;
	setAttr ".gr" 1;
createNode mentalrayOptions -s -n "PreviewFinalgather";
	setAttr ".minsp" -1;
	setAttr ".maxsp" 1;
	setAttr ".fil" 1;
	setAttr ".rflr" 2;
	setAttr ".rfrr" 2;
	setAttr ".maxr" 4;
	setAttr ".gi" yes;
	setAttr ".fg" yes;
createNode mentalrayOptions -s -n "ProductionMotionblur";
	setAttr ".minsp" 0;
	setAttr ".maxsp" 2;
	setAttr ".fil" 2;
	setAttr ".rflr" 10;
	setAttr ".rfrr" 10;
	setAttr ".maxr" 20;
	setAttr ".mb" 2;
createNode script -n "uiConfigurationScriptNode";
	setAttr ".b" -type "string" (
		"// Maya Mel UI Configuration File.\n"
		+ "//\n"
		+ "//  This script is machine generated.  Edit at your own risk.\n"
		+ "//\n"
		+ "//\n"
		+ "global string $gMainPane;\n"
		+ "if (`paneLayout -exists $gMainPane`) {\n"
		+ "\tglobal int $gUseScenePanelConfig;\n"
		+ "\tint    $useSceneConfig = $gUseScenePanelConfig;\n"
		+ "\tint    $menusOkayInPanels = `optionVar -q allowMenusInPanels`;\tint    $nVisPanes = `paneLayout -q -nvp $gMainPane`;\n"
		+ "\tint    $nPanes = 0;\n"
		+ "\tstring $editorName;\n"
		+ "\tstring $panelName;\n"
		+ "\tstring $itemFilterName;\n"
		+ "\tstring $panelConfig;\n"
		+ "\t//\n"
		+ "\t//  get current state of the UI\n"
		+ "\t//\n"
		+ "\tsceneUIReplacement -update $gMainPane;\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" \"Top View\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `modelPanel -unParent -l \"Top View\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = $panelName;\n"
		+ "            modelEditor -e \n"
		+ "                -camera \"side\" \n"
		+ "                -useInteractiveMode 0\n"
		+ "                -displayLights \"default\" \n"
		+ "                -displayAppearance \"wireframe\" \n"
		+ "                -activeOnly 0\n"
		+ "                -wireframeOnShaded 0\n"
		+ "                -bufferMode \"double\" \n"
		+ "                -twoSidedLighting 1\n"
		+ "                -backfaceCulling 0\n"
		+ "                -xray 0\n"
		+ "                -displayTextures 0\n"
		+ "                -smoothWireframe 0\n"
		+ "                -textureAnisotropic 0\n"
		+ "                -textureHilight 1\n"
		+ "                -textureSampling 2\n"
		+ "                -textureDisplay \"modulate\" \n"
		+ "                -textureMaxSize 1024\n"
		+ "                -fogging 0\n"
		+ "                -fogSource \"fragment\" \n"
		+ "                -fogMode \"linear\" \n"
		+ "                -fogStart 0\n"
		+ "                -fogEnd 100\n"
		+ "                -fogDensity 0.1\n"
		+ "                -fogColor 0.5 0.5 0.5 1 \n"
		+ "                -maxConstantTransparency 1\n"
		+ "                -rendererName \"base_OpenGL_Renderer\" \n"
		+ "                -colorResolution 256 256 \n"
		+ "                -bumpResolution 512 512 \n"
		+ "                -textureCompression 0\n"
		+ "                -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "                -transpInShadows 0\n"
		+ "                -cullingOverride \"none\" \n"
		+ "                -lowQualityLighting 0\n"
		+ "                -maximumNumHardwareLights 1\n"
		+ "                -occlusionCulling 0\n"
		+ "                -useBaseRenderer 0\n"
		+ "                -useReducedRenderer 0\n"
		+ "                -smallObjectCulling 0\n"
		+ "                -smallObjectThreshold -1 \n"
		+ "                -interactiveDisableShadows 0\n"
		+ "                -interactiveBackFaceCull 0\n"
		+ "                -sortTransparent 1\n"
		+ "                -nurbsCurves 1\n"
		+ "                -nurbsSurfaces 1\n"
		+ "                -polymeshes 1\n"
		+ "                -subdivSurfaces 1\n"
		+ "                -planes 1\n"
		+ "                -lights 1\n"
		+ "                -cameras 1\n"
		+ "                -controlVertices 1\n"
		+ "                -hulls 1\n"
		+ "                -grid 1\n"
		+ "                -joints 1\n"
		+ "                -ikHandles 1\n"
		+ "                -deformers 1\n"
		+ "                -dynamics 1\n"
		+ "                -fluids 1\n"
		+ "                -locators 1\n"
		+ "                -dimensions 1\n"
		+ "                -handles 1\n"
		+ "                -pivots 1\n"
		+ "                -textures 1\n"
		+ "                -strokes 1\n"
		+ "                -shadows 0\n"
		+ "                $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tmodelPanel -edit -l \"Top View\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n"
		+ "        modelEditor -e \n"
		+ "            -camera \"side\" \n"
		+ "            -useInteractiveMode 0\n"
		+ "            -displayLights \"default\" \n"
		+ "            -displayAppearance \"wireframe\" \n"
		+ "            -activeOnly 0\n"
		+ "            -wireframeOnShaded 0\n"
		+ "            -bufferMode \"double\" \n"
		+ "            -twoSidedLighting 1\n"
		+ "            -backfaceCulling 0\n"
		+ "            -xray 0\n"
		+ "            -displayTextures 0\n"
		+ "            -smoothWireframe 0\n"
		+ "            -textureAnisotropic 0\n"
		+ "            -textureHilight 1\n"
		+ "            -textureSampling 2\n"
		+ "            -textureDisplay \"modulate\" \n"
		+ "            -textureMaxSize 1024\n"
		+ "            -fogging 0\n"
		+ "            -fogSource \"fragment\" \n"
		+ "            -fogMode \"linear\" \n"
		+ "            -fogStart 0\n"
		+ "            -fogEnd 100\n"
		+ "            -fogDensity 0.1\n"
		+ "            -fogColor 0.5 0.5 0.5 1 \n"
		+ "            -maxConstantTransparency 1\n"
		+ "            -rendererName \"base_OpenGL_Renderer\" \n"
		+ "            -colorResolution 256 256 \n"
		+ "            -bumpResolution 512 512 \n"
		+ "            -textureCompression 0\n"
		+ "            -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "            -transpInShadows 0\n"
		+ "            -cullingOverride \"none\" \n"
		+ "            -lowQualityLighting 0\n"
		+ "            -maximumNumHardwareLights 1\n"
		+ "            -occlusionCulling 0\n"
		+ "            -useBaseRenderer 0\n"
		+ "            -useReducedRenderer 0\n"
		+ "            -smallObjectCulling 0\n"
		+ "            -smallObjectThreshold -1 \n"
		+ "            -interactiveDisableShadows 0\n"
		+ "            -interactiveBackFaceCull 0\n"
		+ "            -sortTransparent 1\n"
		+ "            -nurbsCurves 1\n"
		+ "            -nurbsSurfaces 1\n"
		+ "            -polymeshes 1\n"
		+ "            -subdivSurfaces 1\n"
		+ "            -planes 1\n"
		+ "            -lights 1\n"
		+ "            -cameras 1\n"
		+ "            -controlVertices 1\n"
		+ "            -hulls 1\n"
		+ "            -grid 1\n"
		+ "            -joints 1\n"
		+ "            -ikHandles 1\n"
		+ "            -deformers 1\n"
		+ "            -dynamics 1\n"
		+ "            -fluids 1\n"
		+ "            -locators 1\n"
		+ "            -dimensions 1\n"
		+ "            -handles 1\n"
		+ "            -pivots 1\n"
		+ "            -textures 1\n"
		+ "            -strokes 1\n"
		+ "            -shadows 0\n"
		+ "            $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" \"Side View\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `modelPanel -unParent -l \"Side View\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = $panelName;\n"
		+ "            modelEditor -e \n"
		+ "                -camera \"side\" \n"
		+ "                -useInteractiveMode 0\n"
		+ "                -displayLights \"default\" \n"
		+ "                -displayAppearance \"wireframe\" \n"
		+ "                -activeOnly 0\n"
		+ "                -wireframeOnShaded 0\n"
		+ "                -bufferMode \"double\" \n"
		+ "                -twoSidedLighting 1\n"
		+ "                -backfaceCulling 0\n"
		+ "                -xray 0\n"
		+ "                -displayTextures 0\n"
		+ "                -smoothWireframe 0\n"
		+ "                -textureAnisotropic 0\n"
		+ "                -textureHilight 1\n"
		+ "                -textureSampling 2\n"
		+ "                -textureDisplay \"modulate\" \n"
		+ "                -textureMaxSize 1024\n"
		+ "                -fogging 0\n"
		+ "                -fogSource \"fragment\" \n"
		+ "                -fogMode \"linear\" \n"
		+ "                -fogStart 0\n"
		+ "                -fogEnd 100\n"
		+ "                -fogDensity 0.1\n"
		+ "                -fogColor 0.5 0.5 0.5 1 \n"
		+ "                -maxConstantTransparency 1\n"
		+ "                -rendererName \"base_OpenGL_Renderer\" \n"
		+ "                -colorResolution 256 256 \n"
		+ "                -bumpResolution 512 512 \n"
		+ "                -textureCompression 0\n"
		+ "                -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "                -transpInShadows 0\n"
		+ "                -cullingOverride \"none\" \n"
		+ "                -lowQualityLighting 0\n"
		+ "                -maximumNumHardwareLights 1\n"
		+ "                -occlusionCulling 0\n"
		+ "                -useBaseRenderer 0\n"
		+ "                -useReducedRenderer 0\n"
		+ "                -smallObjectCulling 0\n"
		+ "                -smallObjectThreshold -1 \n"
		+ "                -interactiveDisableShadows 0\n"
		+ "                -interactiveBackFaceCull 0\n"
		+ "                -sortTransparent 1\n"
		+ "                -nurbsCurves 1\n"
		+ "                -nurbsSurfaces 1\n"
		+ "                -polymeshes 1\n"
		+ "                -subdivSurfaces 1\n"
		+ "                -planes 1\n"
		+ "                -lights 1\n"
		+ "                -cameras 1\n"
		+ "                -controlVertices 1\n"
		+ "                -hulls 1\n"
		+ "                -grid 1\n"
		+ "                -joints 1\n"
		+ "                -ikHandles 1\n"
		+ "                -deformers 1\n"
		+ "                -dynamics 1\n"
		+ "                -fluids 1\n"
		+ "                -locators 1\n"
		+ "                -dimensions 1\n"
		+ "                -handles 1\n"
		+ "                -pivots 1\n"
		+ "                -textures 1\n"
		+ "                -strokes 1\n"
		+ "                -shadows 0\n"
		+ "                $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tmodelPanel -edit -l \"Side View\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n"
		+ "        modelEditor -e \n"
		+ "            -camera \"side\" \n"
		+ "            -useInteractiveMode 0\n"
		+ "            -displayLights \"default\" \n"
		+ "            -displayAppearance \"wireframe\" \n"
		+ "            -activeOnly 0\n"
		+ "            -wireframeOnShaded 0\n"
		+ "            -bufferMode \"double\" \n"
		+ "            -twoSidedLighting 1\n"
		+ "            -backfaceCulling 0\n"
		+ "            -xray 0\n"
		+ "            -displayTextures 0\n"
		+ "            -smoothWireframe 0\n"
		+ "            -textureAnisotropic 0\n"
		+ "            -textureHilight 1\n"
		+ "            -textureSampling 2\n"
		+ "            -textureDisplay \"modulate\" \n"
		+ "            -textureMaxSize 1024\n"
		+ "            -fogging 0\n"
		+ "            -fogSource \"fragment\" \n"
		+ "            -fogMode \"linear\" \n"
		+ "            -fogStart 0\n"
		+ "            -fogEnd 100\n"
		+ "            -fogDensity 0.1\n"
		+ "            -fogColor 0.5 0.5 0.5 1 \n"
		+ "            -maxConstantTransparency 1\n"
		+ "            -rendererName \"base_OpenGL_Renderer\" \n"
		+ "            -colorResolution 256 256 \n"
		+ "            -bumpResolution 512 512 \n"
		+ "            -textureCompression 0\n"
		+ "            -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "            -transpInShadows 0\n"
		+ "            -cullingOverride \"none\" \n"
		+ "            -lowQualityLighting 0\n"
		+ "            -maximumNumHardwareLights 1\n"
		+ "            -occlusionCulling 0\n"
		+ "            -useBaseRenderer 0\n"
		+ "            -useReducedRenderer 0\n"
		+ "            -smallObjectCulling 0\n"
		+ "            -smallObjectThreshold -1 \n"
		+ "            -interactiveDisableShadows 0\n"
		+ "            -interactiveBackFaceCull 0\n"
		+ "            -sortTransparent 1\n"
		+ "            -nurbsCurves 1\n"
		+ "            -nurbsSurfaces 1\n"
		+ "            -polymeshes 1\n"
		+ "            -subdivSurfaces 1\n"
		+ "            -planes 1\n"
		+ "            -lights 1\n"
		+ "            -cameras 1\n"
		+ "            -controlVertices 1\n"
		+ "            -hulls 1\n"
		+ "            -grid 1\n"
		+ "            -joints 1\n"
		+ "            -ikHandles 1\n"
		+ "            -deformers 1\n"
		+ "            -dynamics 1\n"
		+ "            -fluids 1\n"
		+ "            -locators 1\n"
		+ "            -dimensions 1\n"
		+ "            -handles 1\n"
		+ "            -pivots 1\n"
		+ "            -textures 1\n"
		+ "            -strokes 1\n"
		+ "            -shadows 0\n"
		+ "            $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" \"Front View\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `modelPanel -unParent -l \"Front View\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = $panelName;\n"
		+ "            modelEditor -e \n"
		+ "                -camera \"front\" \n"
		+ "                -useInteractiveMode 0\n"
		+ "                -displayLights \"default\" \n"
		+ "                -displayAppearance \"wireframe\" \n"
		+ "                -activeOnly 0\n"
		+ "                -wireframeOnShaded 0\n"
		+ "                -bufferMode \"double\" \n"
		+ "                -twoSidedLighting 1\n"
		+ "                -backfaceCulling 0\n"
		+ "                -xray 0\n"
		+ "                -displayTextures 0\n"
		+ "                -smoothWireframe 0\n"
		+ "                -textureAnisotropic 0\n"
		+ "                -textureHilight 1\n"
		+ "                -textureSampling 2\n"
		+ "                -textureDisplay \"modulate\" \n"
		+ "                -textureMaxSize 1024\n"
		+ "                -fogging 0\n"
		+ "                -fogSource \"fragment\" \n"
		+ "                -fogMode \"linear\" \n"
		+ "                -fogStart 0\n"
		+ "                -fogEnd 100\n"
		+ "                -fogDensity 0.1\n"
		+ "                -fogColor 0.5 0.5 0.5 1 \n"
		+ "                -maxConstantTransparency 1\n"
		+ "                -rendererName \"base_OpenGL_Renderer\" \n"
		+ "                -colorResolution 256 256 \n"
		+ "                -bumpResolution 512 512 \n"
		+ "                -textureCompression 0\n"
		+ "                -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "                -transpInShadows 0\n"
		+ "                -cullingOverride \"none\" \n"
		+ "                -lowQualityLighting 0\n"
		+ "                -maximumNumHardwareLights 1\n"
		+ "                -occlusionCulling 0\n"
		+ "                -useBaseRenderer 0\n"
		+ "                -useReducedRenderer 0\n"
		+ "                -smallObjectCulling 0\n"
		+ "                -smallObjectThreshold -1 \n"
		+ "                -interactiveDisableShadows 0\n"
		+ "                -interactiveBackFaceCull 0\n"
		+ "                -sortTransparent 1\n"
		+ "                -nurbsCurves 1\n"
		+ "                -nurbsSurfaces 1\n"
		+ "                -polymeshes 1\n"
		+ "                -subdivSurfaces 1\n"
		+ "                -planes 1\n"
		+ "                -lights 1\n"
		+ "                -cameras 1\n"
		+ "                -controlVertices 1\n"
		+ "                -hulls 1\n"
		+ "                -grid 1\n"
		+ "                -joints 1\n"
		+ "                -ikHandles 1\n"
		+ "                -deformers 1\n"
		+ "                -dynamics 1\n"
		+ "                -fluids 1\n"
		+ "                -locators 1\n"
		+ "                -dimensions 1\n"
		+ "                -handles 1\n"
		+ "                -pivots 1\n"
		+ "                -textures 1\n"
		+ "                -strokes 1\n"
		+ "                -shadows 0\n"
		+ "                $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tmodelPanel -edit -l \"Front View\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n"
		+ "        modelEditor -e \n"
		+ "            -camera \"front\" \n"
		+ "            -useInteractiveMode 0\n"
		+ "            -displayLights \"default\" \n"
		+ "            -displayAppearance \"wireframe\" \n"
		+ "            -activeOnly 0\n"
		+ "            -wireframeOnShaded 0\n"
		+ "            -bufferMode \"double\" \n"
		+ "            -twoSidedLighting 1\n"
		+ "            -backfaceCulling 0\n"
		+ "            -xray 0\n"
		+ "            -displayTextures 0\n"
		+ "            -smoothWireframe 0\n"
		+ "            -textureAnisotropic 0\n"
		+ "            -textureHilight 1\n"
		+ "            -textureSampling 2\n"
		+ "            -textureDisplay \"modulate\" \n"
		+ "            -textureMaxSize 1024\n"
		+ "            -fogging 0\n"
		+ "            -fogSource \"fragment\" \n"
		+ "            -fogMode \"linear\" \n"
		+ "            -fogStart 0\n"
		+ "            -fogEnd 100\n"
		+ "            -fogDensity 0.1\n"
		+ "            -fogColor 0.5 0.5 0.5 1 \n"
		+ "            -maxConstantTransparency 1\n"
		+ "            -rendererName \"base_OpenGL_Renderer\" \n"
		+ "            -colorResolution 256 256 \n"
		+ "            -bumpResolution 512 512 \n"
		+ "            -textureCompression 0\n"
		+ "            -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "            -transpInShadows 0\n"
		+ "            -cullingOverride \"none\" \n"
		+ "            -lowQualityLighting 0\n"
		+ "            -maximumNumHardwareLights 1\n"
		+ "            -occlusionCulling 0\n"
		+ "            -useBaseRenderer 0\n"
		+ "            -useReducedRenderer 0\n"
		+ "            -smallObjectCulling 0\n"
		+ "            -smallObjectThreshold -1 \n"
		+ "            -interactiveDisableShadows 0\n"
		+ "            -interactiveBackFaceCull 0\n"
		+ "            -sortTransparent 1\n"
		+ "            -nurbsCurves 1\n"
		+ "            -nurbsSurfaces 1\n"
		+ "            -polymeshes 1\n"
		+ "            -subdivSurfaces 1\n"
		+ "            -planes 1\n"
		+ "            -lights 1\n"
		+ "            -cameras 1\n"
		+ "            -controlVertices 1\n"
		+ "            -hulls 1\n"
		+ "            -grid 1\n"
		+ "            -joints 1\n"
		+ "            -ikHandles 1\n"
		+ "            -deformers 1\n"
		+ "            -dynamics 1\n"
		+ "            -fluids 1\n"
		+ "            -locators 1\n"
		+ "            -dimensions 1\n"
		+ "            -handles 1\n"
		+ "            -pivots 1\n"
		+ "            -textures 1\n"
		+ "            -strokes 1\n"
		+ "            -shadows 0\n"
		+ "            $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" \"Persp View\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `modelPanel -unParent -l \"Persp View\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = $panelName;\n"
		+ "            modelEditor -e \n"
		+ "                -camera \"persp\" \n"
		+ "                -useInteractiveMode 0\n"
		+ "                -displayLights \"default\" \n"
		+ "                -displayAppearance \"wireframe\" \n"
		+ "                -activeOnly 0\n"
		+ "                -wireframeOnShaded 0\n"
		+ "                -bufferMode \"double\" \n"
		+ "                -twoSidedLighting 1\n"
		+ "                -backfaceCulling 0\n"
		+ "                -xray 0\n"
		+ "                -displayTextures 0\n"
		+ "                -smoothWireframe 0\n"
		+ "                -textureAnisotropic 0\n"
		+ "                -textureHilight 1\n"
		+ "                -textureSampling 2\n"
		+ "                -textureDisplay \"modulate\" \n"
		+ "                -textureMaxSize 1024\n"
		+ "                -fogging 0\n"
		+ "                -fogSource \"fragment\" \n"
		+ "                -fogMode \"linear\" \n"
		+ "                -fogStart 0\n"
		+ "                -fogEnd 100\n"
		+ "                -fogDensity 0.1\n"
		+ "                -fogColor 0.5 0.5 0.5 1 \n"
		+ "                -maxConstantTransparency 1\n"
		+ "                -rendererName \"base_OpenGL_Renderer\" \n"
		+ "                -colorResolution 256 256 \n"
		+ "                -bumpResolution 512 512 \n"
		+ "                -textureCompression 0\n"
		+ "                -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "                -transpInShadows 0\n"
		+ "                -cullingOverride \"none\" \n"
		+ "                -lowQualityLighting 0\n"
		+ "                -maximumNumHardwareLights 1\n"
		+ "                -occlusionCulling 0\n"
		+ "                -useBaseRenderer 0\n"
		+ "                -useReducedRenderer 0\n"
		+ "                -smallObjectCulling 0\n"
		+ "                -smallObjectThreshold -1 \n"
		+ "                -interactiveDisableShadows 0\n"
		+ "                -interactiveBackFaceCull 0\n"
		+ "                -sortTransparent 1\n"
		+ "                -nurbsCurves 1\n"
		+ "                -nurbsSurfaces 1\n"
		+ "                -polymeshes 1\n"
		+ "                -subdivSurfaces 1\n"
		+ "                -planes 1\n"
		+ "                -lights 1\n"
		+ "                -cameras 1\n"
		+ "                -controlVertices 1\n"
		+ "                -hulls 1\n"
		+ "                -grid 1\n"
		+ "                -joints 1\n"
		+ "                -ikHandles 1\n"
		+ "                -deformers 1\n"
		+ "                -dynamics 1\n"
		+ "                -fluids 1\n"
		+ "                -locators 1\n"
		+ "                -dimensions 1\n"
		+ "                -handles 1\n"
		+ "                -pivots 1\n"
		+ "                -textures 1\n"
		+ "                -strokes 1\n"
		+ "                -shadows 0\n"
		+ "                $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tmodelPanel -edit -l \"Persp View\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n"
		+ "        modelEditor -e \n"
		+ "            -camera \"persp\" \n"
		+ "            -useInteractiveMode 0\n"
		+ "            -displayLights \"default\" \n"
		+ "            -displayAppearance \"wireframe\" \n"
		+ "            -activeOnly 0\n"
		+ "            -wireframeOnShaded 0\n"
		+ "            -bufferMode \"double\" \n"
		+ "            -twoSidedLighting 1\n"
		+ "            -backfaceCulling 0\n"
		+ "            -xray 0\n"
		+ "            -displayTextures 0\n"
		+ "            -smoothWireframe 0\n"
		+ "            -textureAnisotropic 0\n"
		+ "            -textureHilight 1\n"
		+ "            -textureSampling 2\n"
		+ "            -textureDisplay \"modulate\" \n"
		+ "            -textureMaxSize 1024\n"
		+ "            -fogging 0\n"
		+ "            -fogSource \"fragment\" \n"
		+ "            -fogMode \"linear\" \n"
		+ "            -fogStart 0\n"
		+ "            -fogEnd 100\n"
		+ "            -fogDensity 0.1\n"
		+ "            -fogColor 0.5 0.5 0.5 1 \n"
		+ "            -maxConstantTransparency 1\n"
		+ "            -rendererName \"base_OpenGL_Renderer\" \n"
		+ "            -colorResolution 256 256 \n"
		+ "            -bumpResolution 512 512 \n"
		+ "            -textureCompression 0\n"
		+ "            -transparencyAlgorithm \"frontAndBackCull\" \n"
		+ "            -transpInShadows 0\n"
		+ "            -cullingOverride \"none\" \n"
		+ "            -lowQualityLighting 0\n"
		+ "            -maximumNumHardwareLights 1\n"
		+ "            -occlusionCulling 0\n"
		+ "            -useBaseRenderer 0\n"
		+ "            -useReducedRenderer 0\n"
		+ "            -smallObjectCulling 0\n"
		+ "            -smallObjectThreshold -1 \n"
		+ "            -interactiveDisableShadows 0\n"
		+ "            -interactiveBackFaceCull 0\n"
		+ "            -sortTransparent 1\n"
		+ "            -nurbsCurves 1\n"
		+ "            -nurbsSurfaces 1\n"
		+ "            -polymeshes 1\n"
		+ "            -subdivSurfaces 1\n"
		+ "            -planes 1\n"
		+ "            -lights 1\n"
		+ "            -cameras 1\n"
		+ "            -controlVertices 1\n"
		+ "            -hulls 1\n"
		+ "            -grid 1\n"
		+ "            -joints 1\n"
		+ "            -ikHandles 1\n"
		+ "            -deformers 1\n"
		+ "            -dynamics 1\n"
		+ "            -fluids 1\n"
		+ "            -locators 1\n"
		+ "            -dimensions 1\n"
		+ "            -handles 1\n"
		+ "            -pivots 1\n"
		+ "            -textures 1\n"
		+ "            -strokes 1\n"
		+ "            -shadows 0\n"
		+ "            $editorName;\n"
		+ "modelEditor -e -viewSelected 0 $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"outlinerPanel\" \"Outliner\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `outlinerPanel -unParent -l \"Outliner\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = $panelName;\n"
		+ "            outlinerEditor -e \n"
		+ "                -mainListConnection \"worldList\" \n"
		+ "                -selectionConnection \"modelList\" \n"
		+ "                -showShapes 0\n"
		+ "                -showAttributes 0\n"
		+ "                -showConnected 0\n"
		+ "                -showAnimCurvesOnly 0\n"
		+ "                -autoExpand 0\n"
		+ "                -showDagOnly 1\n"
		+ "                -ignoreDagHierarchy 0\n"
		+ "                -expandConnections 0\n"
		+ "                -showUnitlessCurves 1\n"
		+ "                -showCompounds 1\n"
		+ "                -showLeafs 1\n"
		+ "                -showNumericAttrsOnly 0\n"
		+ "                -highlightActive 1\n"
		+ "                -autoSelectNewObjects 0\n"
		+ "                -doNotSelectNewObjects 0\n"
		+ "                -dropIsParent 1\n"
		+ "                -transmitFilters 0\n"
		+ "                -setFilter \"defaultSetFilter\" \n"
		+ "                -showSetMembers 0\n"
		+ "                -allowMultiSelection 1\n"
		+ "                -alwaysToggleSelect 0\n"
		+ "                -directSelect 0\n"
		+ "                -displayMode \"DAG\" \n"
		+ "                -expandObjects 0\n"
		+ "                -setsIgnoreFilters 1\n"
		+ "                -editAttrName 0\n"
		+ "                -showAttrValues 0\n"
		+ "                -highlightSecondary 0\n"
		+ "                -showUVAttrsOnly 0\n"
		+ "                -showTextureNodesOnly 0\n"
		+ "                -sortOrder \"none\" \n"
		+ "                -longNames 0\n"
		+ "                -niceNames 1\n"
		+ "                $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\toutlinerPanel -edit -l \"Outliner\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n"
		+ "        outlinerEditor -e \n"
		+ "            -mainListConnection \"worldList\" \n"
		+ "            -selectionConnection \"modelList\" \n"
		+ "            -showShapes 0\n"
		+ "            -showAttributes 0\n"
		+ "            -showConnected 0\n"
		+ "            -showAnimCurvesOnly 0\n"
		+ "            -autoExpand 0\n"
		+ "            -showDagOnly 1\n"
		+ "            -ignoreDagHierarchy 0\n"
		+ "            -expandConnections 0\n"
		+ "            -showUnitlessCurves 1\n"
		+ "            -showCompounds 1\n"
		+ "            -showLeafs 1\n"
		+ "            -showNumericAttrsOnly 0\n"
		+ "            -highlightActive 1\n"
		+ "            -autoSelectNewObjects 0\n"
		+ "            -doNotSelectNewObjects 0\n"
		+ "            -dropIsParent 1\n"
		+ "            -transmitFilters 0\n"
		+ "            -setFilter \"defaultSetFilter\" \n"
		+ "            -showSetMembers 0\n"
		+ "            -allowMultiSelection 1\n"
		+ "            -alwaysToggleSelect 0\n"
		+ "            -directSelect 0\n"
		+ "            -displayMode \"DAG\" \n"
		+ "            -expandObjects 0\n"
		+ "            -setsIgnoreFilters 1\n"
		+ "            -editAttrName 0\n"
		+ "            -showAttrValues 0\n"
		+ "            -highlightSecondary 0\n"
		+ "            -showUVAttrsOnly 0\n"
		+ "            -showTextureNodesOnly 0\n"
		+ "            -sortOrder \"none\" \n"
		+ "            -longNames 0\n"
		+ "            -niceNames 1\n"
		+ "            $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"graphEditor\" \"Graph Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"graphEditor\" -l \"Graph Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = ($panelName+\"OutlineEd\");\n"
		+ "            outlinerEditor -e \n"
		+ "                -mainListConnection \"graphEditorList\" \n"
		+ "                -selectionConnection \"graphEditor1FromOutliner\" \n"
		+ "                -highlightConnection \"keyframeList\" \n"
		+ "                -showShapes 1\n"
		+ "                -showAttributes 1\n"
		+ "                -showConnected 1\n"
		+ "                -showAnimCurvesOnly 1\n"
		+ "                -autoExpand 1\n"
		+ "                -showDagOnly 0\n"
		+ "                -ignoreDagHierarchy 0\n"
		+ "                -expandConnections 1\n"
		+ "                -showUnitlessCurves 1\n"
		+ "                -showCompounds 0\n"
		+ "                -showLeafs 1\n"
		+ "                -showNumericAttrsOnly 1\n"
		+ "                -highlightActive 0\n"
		+ "                -autoSelectNewObjects 1\n"
		+ "                -doNotSelectNewObjects 0\n"
		+ "                -dropIsParent 1\n"
		+ "                -transmitFilters 1\n"
		+ "                -setFilter \"0\" \n"
		+ "                -showSetMembers 0\n"
		+ "                -allowMultiSelection 1\n"
		+ "                -alwaysToggleSelect 0\n"
		+ "                -directSelect 0\n"
		+ "                -displayMode \"DAG\" \n"
		+ "                -expandObjects 0\n"
		+ "                -setsIgnoreFilters 1\n"
		+ "                -editAttrName 0\n"
		+ "                -showAttrValues 0\n"
		+ "                -highlightSecondary 0\n"
		+ "                -showUVAttrsOnly 0\n"
		+ "                -showTextureNodesOnly 0\n"
		+ "                -sortOrder \"none\" \n"
		+ "                -longNames 0\n"
		+ "                -niceNames 1\n"
		+ "                $editorName;\n"
		+ "\t\t\t$editorName = ($panelName+\"GraphEd\");\n"
		+ "            animCurveEditor -e \n"
		+ "                -mainListConnection \"graphEditor1FromOutliner\" \n"
		+ "                -displayKeys 1\n"
		+ "                -displayTangents 0\n"
		+ "                -displayActiveKeys 0\n"
		+ "                -displayActiveKeyTangents 1\n"
		+ "                -displayInfinities 0\n"
		+ "                -autoFit 0\n"
		+ "                -snapTime \"integer\" \n"
		+ "                -snapValue \"none\" \n"
		+ "                -showResults \"off\" \n"
		+ "                -showBufferCurves \"off\" \n"
		+ "                -smoothness \"fine\" \n"
		+ "                -resultSamples 1\n"
		+ "                -resultScreenSamples 0\n"
		+ "                -resultUpdate \"delayed\" \n"
		+ "                $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Graph Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t\t$editorName = ($panelName+\"OutlineEd\");\n"
		+ "            outlinerEditor -e \n"
		+ "                -mainListConnection \"graphEditorList\" \n"
		+ "                -selectionConnection \"graphEditor1FromOutliner\" \n"
		+ "                -highlightConnection \"keyframeList\" \n"
		+ "                -showShapes 1\n"
		+ "                -showAttributes 1\n"
		+ "                -showConnected 1\n"
		+ "                -showAnimCurvesOnly 1\n"
		+ "                -autoExpand 1\n"
		+ "                -showDagOnly 0\n"
		+ "                -ignoreDagHierarchy 0\n"
		+ "                -expandConnections 1\n"
		+ "                -showUnitlessCurves 1\n"
		+ "                -showCompounds 0\n"
		+ "                -showLeafs 1\n"
		+ "                -showNumericAttrsOnly 1\n"
		+ "                -highlightActive 0\n"
		+ "                -autoSelectNewObjects 1\n"
		+ "                -doNotSelectNewObjects 0\n"
		+ "                -dropIsParent 1\n"
		+ "                -transmitFilters 1\n"
		+ "                -setFilter \"0\" \n"
		+ "                -showSetMembers 0\n"
		+ "                -allowMultiSelection 1\n"
		+ "                -alwaysToggleSelect 0\n"
		+ "                -directSelect 0\n"
		+ "                -displayMode \"DAG\" \n"
		+ "                -expandObjects 0\n"
		+ "                -setsIgnoreFilters 1\n"
		+ "                -editAttrName 0\n"
		+ "                -showAttrValues 0\n"
		+ "                -highlightSecondary 0\n"
		+ "                -showUVAttrsOnly 0\n"
		+ "                -showTextureNodesOnly 0\n"
		+ "                -sortOrder \"none\" \n"
		+ "                -longNames 0\n"
		+ "                -niceNames 1\n"
		+ "                $editorName;\n"
		+ "\t\t\t$editorName = ($panelName+\"GraphEd\");\n"
		+ "            animCurveEditor -e \n"
		+ "                -mainListConnection \"graphEditor1FromOutliner\" \n"
		+ "                -displayKeys 1\n"
		+ "                -displayTangents 0\n"
		+ "                -displayActiveKeys 0\n"
		+ "                -displayActiveKeyTangents 1\n"
		+ "                -displayInfinities 0\n"
		+ "                -autoFit 0\n"
		+ "                -snapTime \"integer\" \n"
		+ "                -snapValue \"none\" \n"
		+ "                -showResults \"off\" \n"
		+ "                -showBufferCurves \"off\" \n"
		+ "                -smoothness \"fine\" \n"
		+ "                -resultSamples 1\n"
		+ "                -resultScreenSamples 0\n"
		+ "                -resultUpdate \"delayed\" \n"
		+ "                $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dopeSheetPanel\" \"Dope Sheet\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"dopeSheetPanel\" -l \"Dope Sheet\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = ($panelName+\"OutlineEd\");\n"
		+ "            outlinerEditor -e \n"
		+ "                -mainListConnection \"animationList\" \n"
		+ "                -selectionConnection \"dopeSheetPanel1OutlinerSelection\" \n"
		+ "                -highlightConnection \"keyframeList\" \n"
		+ "                -showShapes 1\n"
		+ "                -showAttributes 1\n"
		+ "                -showConnected 1\n"
		+ "                -showAnimCurvesOnly 1\n"
		+ "                -autoExpand 0\n"
		+ "                -showDagOnly 0\n"
		+ "                -ignoreDagHierarchy 0\n"
		+ "                -expandConnections 1\n"
		+ "                -showUnitlessCurves 0\n"
		+ "                -showCompounds 1\n"
		+ "                -showLeafs 1\n"
		+ "                -showNumericAttrsOnly 1\n"
		+ "                -highlightActive 0\n"
		+ "                -autoSelectNewObjects 0\n"
		+ "                -doNotSelectNewObjects 1\n"
		+ "                -dropIsParent 1\n"
		+ "                -transmitFilters 0\n"
		+ "                -setFilter \"0\" \n"
		+ "                -showSetMembers 0\n"
		+ "                -allowMultiSelection 1\n"
		+ "                -alwaysToggleSelect 0\n"
		+ "                -directSelect 0\n"
		+ "                -displayMode \"DAG\" \n"
		+ "                -expandObjects 0\n"
		+ "                -setsIgnoreFilters 1\n"
		+ "                -editAttrName 0\n"
		+ "                -showAttrValues 0\n"
		+ "                -highlightSecondary 0\n"
		+ "                -showUVAttrsOnly 0\n"
		+ "                -showTextureNodesOnly 0\n"
		+ "                -sortOrder \"none\" \n"
		+ "                -longNames 0\n"
		+ "                -niceNames 1\n"
		+ "                $editorName;\n"
		+ "\t\t\t$editorName = ($panelName+\"DopeSheetEd\");\n"
		+ "            dopeSheetEditor -e \n"
		+ "                -mainListConnection \"dopeSheetPanel1FromOutliner\" \n"
		+ "                -highlightConnection \"dopeSheetPanel1OutlinerSelection\" \n"
		+ "                -displayKeys 1\n"
		+ "                -displayTangents 0\n"
		+ "                -displayActiveKeys 0\n"
		+ "                -displayActiveKeyTangents 0\n"
		+ "                -displayInfinities 0\n"
		+ "                -autoFit 0\n"
		+ "                -snapTime \"integer\" \n"
		+ "                -snapValue \"none\" \n"
		+ "                -outliner \"dopeSheetPanel1OutlineEd\" \n"
		+ "                -showSummary 1\n"
		+ "                -showScene 0\n"
		+ "                -hierarchyBelow 0\n"
		+ "                -showTicks 0\n"
		+ "                $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Dope Sheet\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t\t$editorName = ($panelName+\"OutlineEd\");\n"
		+ "            outlinerEditor -e \n"
		+ "                -mainListConnection \"animationList\" \n"
		+ "                -selectionConnection \"dopeSheetPanel1OutlinerSelection\" \n"
		+ "                -highlightConnection \"keyframeList\" \n"
		+ "                -showShapes 1\n"
		+ "                -showAttributes 1\n"
		+ "                -showConnected 1\n"
		+ "                -showAnimCurvesOnly 1\n"
		+ "                -autoExpand 0\n"
		+ "                -showDagOnly 0\n"
		+ "                -ignoreDagHierarchy 0\n"
		+ "                -expandConnections 1\n"
		+ "                -showUnitlessCurves 0\n"
		+ "                -showCompounds 1\n"
		+ "                -showLeafs 1\n"
		+ "                -showNumericAttrsOnly 1\n"
		+ "                -highlightActive 0\n"
		+ "                -autoSelectNewObjects 0\n"
		+ "                -doNotSelectNewObjects 1\n"
		+ "                -dropIsParent 1\n"
		+ "                -transmitFilters 0\n"
		+ "                -setFilter \"0\" \n"
		+ "                -showSetMembers 0\n"
		+ "                -allowMultiSelection 1\n"
		+ "                -alwaysToggleSelect 0\n"
		+ "                -directSelect 0\n"
		+ "                -displayMode \"DAG\" \n"
		+ "                -expandObjects 0\n"
		+ "                -setsIgnoreFilters 1\n"
		+ "                -editAttrName 0\n"
		+ "                -showAttrValues 0\n"
		+ "                -highlightSecondary 0\n"
		+ "                -showUVAttrsOnly 0\n"
		+ "                -showTextureNodesOnly 0\n"
		+ "                -sortOrder \"none\" \n"
		+ "                -longNames 0\n"
		+ "                -niceNames 1\n"
		+ "                $editorName;\n"
		+ "\t\t\t$editorName = ($panelName+\"DopeSheetEd\");\n"
		+ "            dopeSheetEditor -e \n"
		+ "                -mainListConnection \"dopeSheetPanel1FromOutliner\" \n"
		+ "                -highlightConnection \"dopeSheetPanel1OutlinerSelection\" \n"
		+ "                -displayKeys 1\n"
		+ "                -displayTangents 0\n"
		+ "                -displayActiveKeys 0\n"
		+ "                -displayActiveKeyTangents 0\n"
		+ "                -displayInfinities 0\n"
		+ "                -autoFit 0\n"
		+ "                -snapTime \"integer\" \n"
		+ "                -snapValue \"none\" \n"
		+ "                -outliner \"dopeSheetPanel1OutlineEd\" \n"
		+ "                -showSummary 1\n"
		+ "                -showScene 0\n"
		+ "                -hierarchyBelow 0\n"
		+ "                -showTicks 0\n"
		+ "                $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"clipEditorPanel\" \"Trax Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"clipEditorPanel\" -l \"Trax Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = ($panelName+\"ClipEditor\");\n"
		+ "            clipEditor -e \n"
		+ "                -mainListConnection \"clipEditorList\" \n"
		+ "                -highlightConnection \"clipEditorPanel1HighlightConnection\" \n"
		+ "                -displayKeys 0\n"
		+ "                -displayTangents 0\n"
		+ "                -displayActiveKeys 0\n"
		+ "                -displayActiveKeyTangents 0\n"
		+ "                -displayInfinities 0\n"
		+ "                -autoFit 0\n"
		+ "                -snapTime \"none\" \n"
		+ "                -snapValue \"integer\" \n"
		+ "                $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Trax Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t\t$editorName = ($panelName+\"ClipEditor\");\n"
		+ "            clipEditor -e \n"
		+ "                -mainListConnection \"clipEditorList\" \n"
		+ "                -highlightConnection \"clipEditorPanel1HighlightConnection\" \n"
		+ "                -displayKeys 0\n"
		+ "                -displayTangents 0\n"
		+ "                -displayActiveKeys 0\n"
		+ "                -displayActiveKeyTangents 0\n"
		+ "                -displayInfinities 0\n"
		+ "                -autoFit 0\n"
		+ "                -snapTime \"none\" \n"
		+ "                -snapValue \"integer\" \n"
		+ "                $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"hyperGraphPanel\" \"Hypergraph\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"hyperGraphPanel\" -l \"Hypergraph\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t\t$editorName = ($panelName+\"HyperGraphEd\");\n"
		+ "            hyperGraph -e \n"
		+ "                -orientation \"horiz\" \n"
		+ "                -zoom 1\n"
		+ "                -animateTransition 0\n"
		+ "                -showShapes 0\n"
		+ "                -showDeformers 0\n"
		+ "                -showExpressions 0\n"
		+ "                -showConstraints 0\n"
		+ "                -showUnderworld 0\n"
		+ "                -showInvisible 0\n"
		+ "                -transitionFrames 1\n"
		+ "                -freeform 0\n"
		+ "                -imageEnabled 0\n"
		+ "                -graphType \"DAG\" \n"
		+ "                -updateSelection 1\n"
		+ "                -updateNodeAdded 1\n"
		+ "                -useDrawOverrideColor 0\n"
		+ "                $editorName;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Hypergraph\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t\t$editorName = ($panelName+\"HyperGraphEd\");\n"
		+ "            hyperGraph -e \n"
		+ "                -orientation \"horiz\" \n"
		+ "                -zoom 1\n"
		+ "                -animateTransition 0\n"
		+ "                -showShapes 0\n"
		+ "                -showDeformers 0\n"
		+ "                -showExpressions 0\n"
		+ "                -showConstraints 0\n"
		+ "                -showUnderworld 0\n"
		+ "                -showInvisible 0\n"
		+ "                -transitionFrames 1\n"
		+ "                -freeform 0\n"
		+ "                -imageEnabled 0\n"
		+ "                -graphType \"DAG\" \n"
		+ "                -updateSelection 1\n"
		+ "                -updateNodeAdded 1\n"
		+ "                -useDrawOverrideColor 0\n"
		+ "                $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"hyperShadePanel\" \"Hypershade\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"hyperShadePanel\" -l \"Hypershade\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Hypershade\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"visorPanel\" \"Visor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"visorPanel\" -l \"Visor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Visor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"polyTexturePlacementPanel\" \"UV Texture Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"polyTexturePlacementPanel\" -l \"UV Texture Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"UV Texture Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"multiListerPanel\" \"Multilister\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"multiListerPanel\" -l \"Multilister\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Multilister\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"renderWindowPanel\" \"Render View\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"renderWindowPanel\" -l \"Render View\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Render View\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"blendShapePanel\" \"Blend Shape\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\tblendShapePanel -unParent -l \"Blend Shape\" -mbv $menusOkayInPanels ;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tblendShapePanel -edit -l \"Blend Shape\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dynRelEdPanel\" \"Dynamic Relationships\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"dynRelEdPanel\" -l \"Dynamic Relationships\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Dynamic Relationships\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextPanel \"devicePanel\" \"Devices\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\tdevicePanel -unParent -l \"Devices\" -mbv $menusOkayInPanels ;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tdevicePanel -edit -l \"Devices\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"relationshipPanel\" \"Relationship Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"relationshipPanel\" -l \"Relationship Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Relationship Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"referenceEditorPanel\" \"Reference Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"referenceEditorPanel\" -l \"Reference Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Reference Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"preloadReferenceEditorPanel\" \"Preload Reference Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"preloadReferenceEditorPanel\" -l \"Preload Reference Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Preload Reference Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"componentEditorPanel\" \"Component Editor\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"componentEditorPanel\" -l \"Component Editor\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Component Editor\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dynPaintScriptedPanelType\" \"Paint Effects\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"dynPaintScriptedPanelType\" -l \"Paint Effects\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Paint Effects\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"webBrowserPanel\" \"Web Browser\"`;\n"
		+ "\tif (\"\" == $panelName) {\n"
		+ "\t\tif ($useSceneConfig) {\n"
		+ "\t\t\t$panelName = `scriptedPanel -unParent  -type \"webBrowserPanel\" -l \"Web Browser\" -mbv $menusOkayInPanels `;\n"
		+ "\t\t}\n"
		+ "\t} else {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\tscriptedPanel -edit -l \"Web Browser\" -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n"
		+ "\t}\n"
		+ "\tif ($useSceneConfig) {\n"
		+ "        string $configName = `getPanel -cwl \"Current Layout\"`;\n"
		+ "        if (\"\" != $configName) {\n"
		+ "\t\t\tpanelConfiguration -edit -label \"Current Layout\"\n"
		+ "\t\t\t\t-defaultImage \"\"\n"
		+ "\t\t\t\t-image \"\"\n"
		+ "\t\t\t\t-sc false\n"
		+ "\t\t\t\t-configString \"global string $gMainPane; paneLayout -e -cn \\\"quad\\\" -ps 1 47 50 -ps 2 53 50 -ps 3 53 50 -ps 4 47 50 $gMainPane;\"\n"
		+ "\t\t\t\t-removeAllPanels\n"
		+ "\t\t\t\t-ap true\n"
		+ "\t\t\t\t\t\"Web Browser\"\n"
		+ "\t\t\t\t\t\"scriptedPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `scriptedPanel -unParent  -type \\\"webBrowserPanel\\\" -l \\\"Web Browser\\\" -mbv $menusOkayInPanels `\"\n"
		+ "\t\t\t\t\t\"scriptedPanel -edit -l \\\"Web Browser\\\" -mbv $menusOkayInPanels  $panelName\"\n"
		+ "\t\t\t\t-ap false\n"
		+ "\t\t\t\t\t\"Hypershade\"\n"
		+ "\t\t\t\t\t\"scriptedPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `scriptedPanel -unParent  -type \\\"hyperShadePanel\\\" -l \\\"Hypershade\\\" -mbv $menusOkayInPanels `\"\n"
		+ "\t\t\t\t\t\"scriptedPanel -edit -l \\\"Hypershade\\\" -mbv $menusOkayInPanels  $panelName\"\n"
		+ "\t\t\t\t-ap false\n"
		+ "\t\t\t\t\t\"Persp View\"\n"
		+ "\t\t\t\t\t\"modelPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `modelPanel -unParent -l \\\"Persp View\\\" -mbv $menusOkayInPanels `;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -cam `findStartUpCamera persp` \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"wireframe\\\" \\n    -activeOnly 0\\n    -wireframeOnShaded 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 1\\n    -backfaceCulling 0\\n    -xray 0\\n    -displayTextures 0\\n    -smoothWireframe 0\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 1024\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -maxConstantTransparency 1\\n    -rendererName \\\"base_OpenGL_Renderer\\\" \\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -fluids 1\\n    -locators 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -shadows 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName\"\n"
		+ "\t\t\t\t\t\"modelPanel -edit -l \\\"Persp View\\\" -mbv $menusOkayInPanels  $panelName;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -cam `findStartUpCamera persp` \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"wireframe\\\" \\n    -activeOnly 0\\n    -wireframeOnShaded 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 1\\n    -backfaceCulling 0\\n    -xray 0\\n    -displayTextures 0\\n    -smoothWireframe 0\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 1024\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -maxConstantTransparency 1\\n    -rendererName \\\"base_OpenGL_Renderer\\\" \\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -fluids 1\\n    -locators 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -shadows 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName\"\n"
		+ "\t\t\t\t-ap false\n"
		+ "\t\t\t\t\t\"Render View\"\n"
		+ "\t\t\t\t\t\"scriptedPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `scriptedPanel -unParent  -type \\\"renderWindowPanel\\\" -l \\\"Render View\\\" -mbv $menusOkayInPanels `\"\n"
		+ "\t\t\t\t\t\"scriptedPanel -edit -l \\\"Render View\\\" -mbv $menusOkayInPanels  $panelName\"\n"
		+ "\t\t\t\t$configName;\n"
		+ "            setNamedPanelLayout \"Current Layout\";\n"
		+ "        }\n"
		+ "        panelHistory -e -clear mainPanelHistory;\n"
		+ "        setFocus `paneLayout -q -p1 $gMainPane`;\n"
		+ "        sceneUIReplacement -deleteRemaining;\n"
		+ "        sceneUIReplacement -clear;\n"
		+ "\t}\n"
		+ "grid -spacing 5 -size 12 -divisions 5 -displayAxes yes -displayGridLines yes -displayDivisionLines yes -displayPerspectiveLabels no -displayOrthographicLabels no -displayAxesBold yes -perspectiveLabelPosition axis -orthographicLabelPosition edge;\n"
		+ "}\n");
	setAttr ".st" 3;
createNode script -n "sceneConfigurationScriptNode";
	setAttr ".b" -type "string" "playbackOptions -min 1 -max 24 -ast 1 -aet 48 ";
	setAttr ".st" 6;
createNode mentalrayGlobals -s -n "mentalrayGlobals";
	addAttr -ci true -sn "optimizePhotonShadows" -ln "optimizePhotonShadows" 
		-min 0 -max 1 -at "bool";
createNode mentalrayOptions -s -n "miDefaultOptions";
	setAttr ".rflr" 6;
	setAttr ".rfrr" 6;
	setAttr ".maxr" 12;
	setAttr ".shmth" 0;
createNode mentalrayFramebuffer -s -n "miDefaultFramebuffer";
	setAttr ".w" 360;
	setAttr ".h" 270;
	setAttr ".dar" 1.3333333730697632;
createNode cameraView -n "cameraView1";
	setAttr ".e" -type "double3" 0.21721577004215042 14.764947594399846 18.103059162857665 ;
	setAttr ".coi" -type "double3" 0.27287233854904508 4.7481246847026064 2.1586915344333839 ;
	setAttr ".u" -type "double3" 0.0018569061536942806 0.84676602329328876 -0.5319622671727815 ;
	setAttr ".tp" -type "double3" 0 7.3229649943348267 -3.2705768924876271 ;
	setAttr ".fl" 34.999999999999993;
createNode mentalrayOptions -s -n "Preview";
	setAttr ".minsp" -1;
	setAttr ".maxsp" 1;
	setAttr ".fil" 1;
	setAttr ".rflr" 2;
	setAttr ".rfrr" 2;
	setAttr ".maxr" 4;
createNode mentalrayOptions -s -n "PreviewFinalGather";
	setAttr ".minsp" -1;
	setAttr ".maxsp" 1;
	setAttr ".fil" 1;
	setAttr ".rflr" 2;
	setAttr ".rfrr" 2;
	setAttr ".maxr" 4;
	setAttr ".gi" yes;
	setAttr ".fg" yes;
createNode mentalrayOptions -s -n "Production";
	setAttr ".minsp" 0;
	setAttr ".maxsp" 2;
	setAttr ".fil" 2;
	setAttr ".rflr" 10;
	setAttr ".rfrr" 10;
	setAttr ".maxr" 20;
createNode polySphere -n "polySphere1";
createNode phong -n "phong1";
createNode shadingEngine -n "phong1SG";
	setAttr ".ihi" 0;
	setAttr ".ro" yes;
createNode materialInfo -n "materialInfo3";
createNode polySphere -n "polySphere2";
createNode polySphere -n "polySphere3";
createNode dielectric_material -n "dielectric_material1";
	setAttr ".S00" -type "float3" 0.98000002 0.815 0.79699999 ;
	setAttr ".S01" 2;
	setAttr ".S05" 40;
createNode shadingEngine -n "dielectric_material1SG";
	setAttr ".ihi" 0;
	setAttr ".ro" yes;
createNode materialInfo -n "materialInfo5";
createNode checker -n "checker1";
	setAttr ".c2" -type "float3" 0.65100002 0.63700002 1 ;
createNode place2dTexture -n "place2dTexture1";
	setAttr ".re" -type "float2" 3 3 ;
createNode phong -n "phong3";
	setAttr ".c" -type "float3" 0.63200003 0.78299999 0.80900002 ;
createNode shadingEngine -n "phong3SG";
	setAttr ".ihi" 0;
	setAttr ".ro" yes;
createNode materialInfo -n "materialInfo7";
createNode transmat -n "transmat1";
createNode shadingEngine -n "transmat1SG";
	setAttr ".ihi" 0;
	setAttr ".ro" yes;
createNode materialInfo -n "materialInfo8";
createNode parti_volume -n "parti_volume1";
	setAttr ".S01" -type "float3" 0.80000001 0.80000001 0.80000001 ;
	setAttr ".S02" 0.10000000149011612;
	setAttr ".S06" 0.5;
	setAttr ".S08" 0.05000000074505806;
	setAttr ".S09" 0.20000000298023224;
select -ne :time1;
	setAttr ".o" 1;
select -ne :renderPartition;
	setAttr -s 6 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 5 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :defaultRenderUtilityList1;
select -ne :lightList1;
select -ne :defaultTextureList1;
select -ne :initialShadingGroup;
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :defaultRenderGlobals;
	setAttr ".ren" -type "string" "mentalRay";
	setAttr ".top" 269;
	setAttr ".rght" 359;
	setAttr ".hbl" -type "string" "";
select -ne :defaultResolution;
	setAttr ".w" 320;
	setAttr ".h" 240;
select -ne :defaultLightSet;
select -ne :hardwareRenderGlobals;
	addAttr -ci true -sn "ani" -ln "animation" -bt "ANIM" -min 0 -max 1 -at "bool";
	setAttr ".fn" -type "string" "%s.%e";
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
	setAttr -k on ".ani";
select -ne :defaultHardwareRenderGlobals;
	setAttr ".fn" -type "string" "im";
	setAttr ".res" -type "string" "ntsc_4d 646 485 1.333";
connectAttr "cameraView1.msg" ":perspShape.b" -na;
connectAttr "polySphere1.out" "pSphereShape1.i";
connectAttr "polySphere2.out" "pSphereShape2.i";
connectAttr "polySphere3.out" "pSphereShape3.i";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[0].llnk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.lnk[0].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[1].llnk";
connectAttr ":initialParticleSE.msg" "lightLinker1.lnk[1].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[2].llnk";
connectAttr "transmat1SG.msg" "lightLinker1.lnk[2].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[7].llnk";
connectAttr "phong1SG.msg" "lightLinker1.lnk[7].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[9].llnk";
connectAttr "dielectric_material1SG.msg" "lightLinker1.lnk[9].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[11].llnk";
connectAttr "phong3SG.msg" "lightLinker1.lnk[11].olnk";
connectAttr "layerManager.dli[0]" "defaultLayer.id";
connectAttr "renderLayerManager.rlmi[0]" "defaultRenderLayer.rlid";
connectAttr ":mentalrayGlobals.msg" ":mentalrayItemsList.glb";
connectAttr ":Draft.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":DraftMotionBlur.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":PreviewMotionblur.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":PreviewCaustics.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":PreviewGlobalIllum.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":PreviewFinalgather.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":ProductionMotionblur.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":miDefaultOptions.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":Preview.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":PreviewFinalGather.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":Production.msg" ":mentalrayItemsList.opt" -na;
connectAttr ":miDefaultFramebuffer.msg" ":mentalrayItemsList.fb" -na;
connectAttr ":miDefaultOptions.msg" ":mentalrayGlobals.opt";
connectAttr ":miDefaultFramebuffer.msg" ":mentalrayGlobals.fb";
connectAttr "checker1.oc" "phong1.c";
connectAttr "phong1.oc" "phong1SG.ss";
connectAttr "pSphereShape1.iog" "phong1SG.dsm" -na;
connectAttr "phong1SG.msg" "materialInfo3.sg";
connectAttr "phong1.msg" "materialInfo3.m";
connectAttr "checker1.msg" "materialInfo3.t" -na;
connectAttr "dielectric_material1.msg" "dielectric_material1SG.mims";
connectAttr "pSphereShape3.iog" "dielectric_material1SG.dsm" -na;
connectAttr "dielectric_material1SG.msg" "materialInfo5.sg";
connectAttr "place2dTexture1.o" "checker1.uv";
connectAttr "place2dTexture1.ofs" "checker1.fs";
connectAttr "phong3.oc" "phong3SG.ss";
connectAttr "pSphereShape2.iog" "phong3SG.dsm" -na;
connectAttr "phong3SG.msg" "materialInfo7.sg";
connectAttr "phong3.msg" "materialInfo7.m";
connectAttr "transmat1.msg" "transmat1SG.mims";
connectAttr "vaseShape.iog" "transmat1SG.dsm" -na;
connectAttr "parti_volume1.msg" "transmat1SG.mivs";
connectAttr "transmat1SG.msg" "materialInfo8.sg";
connectAttr "phong1SG.pa" ":renderPartition.st" -na;
connectAttr "dielectric_material1SG.pa" ":renderPartition.st" -na;
connectAttr "phong3SG.pa" ":renderPartition.st" -na;
connectAttr "transmat1SG.pa" ":renderPartition.st" -na;
connectAttr "phong1.msg" ":defaultShaderList1.s" -na;
connectAttr "dielectric_material1.msg" ":defaultShaderList1.s" -na;
connectAttr "phong3.msg" ":defaultShaderList1.s" -na;
connectAttr "place2dTexture1.msg" ":defaultRenderUtilityList1.u" -na;
connectAttr "lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "pointLightShape1.ltd" ":lightList1.l" -na;
connectAttr "checker1.msg" ":defaultTextureList1.tx" -na;
connectAttr "pointLight1.iog" ":defaultLightSet.dsm" -na;
// End of volume_finish.ma
