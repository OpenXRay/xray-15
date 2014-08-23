
#define ANALYZER_INTERFACE Interface_ID(0x14bf34cc, 0x519c0633)
#define FIXER_INTERFACE Interface_ID(0xfcd6b18, 0x6e714e23)
#define FILTER_INTERFACE Interface_ID(0x36ca302f, 0x23d147a6)
#define WORKBENCH_INTERFACE Interface_ID(0x78aa2c29, 0x19a55d39)

class IWorkBench : public FPStaticInterface
{
public:
	//these function not only affect workbench but all functions
	//outside workbench also!
	DECLARE_DESCRIPTOR(IWorkBench); 
	virtual void Open(); //pops up in viewprt
	virtual void ToggleShowX();
	virtual BOOL GetShowX();
	virtual void ToggleShowY();
	virtual BOOL GetShowY();
	virtual void ToggleShowZ();
	virtual BOOL GetShowZ();

	virtual void ToggleLayerEdit();
	virtual BOOL GetLayerEdit();

	virtual void ToggleDrawDuringMove();
	virtual BOOL GetDrawDuringMove();

	virtual void ToggleLimit180();
	virtual BOOL GetLimit180();

	virtual void ShowQuatCurve();
	virtual void ShowPosCurve();
	virtual void ShowAngSpeedCurve();
	virtual void ShowAngAccelCurve();
	virtual void ShowAngJerkCurve();
	virtual void ShowPosSpeedCurve();
	virtual void ShowPosAccelCurve();
	virtual void ShowPosJerkCurve();

	virtual void PosCurveToWorld();
	virtual void PosCurveToBipRoot();
	virtual void PosCurveToThisNode(INode *node);

	typedef enum {toggleShowX=0,getShowX,toggleShowY,getShowY,toggleShowZ,getShowZ,
		toggleLayerEdit,getLayerEdit,toggleDrawDuringMove,getDrawDuringMove,
		toggleLimit180,getLimit180,
		showQuatCurve,showPosCurve,showAngSpeedCurve,showAngAccelCurve,showAngJerkCurve,
		showPosSpeedCurve,showPosAccelCurve,showPosJerkCurve,posCurveToWorld, posCurveToBipRoot,
		posCurveToThisNode,open} WorkBenchFunctions;


	BEGIN_FUNCTION_MAP
			VFN_0(toggleShowX,ToggleShowX);
			FN_0(getShowX,TYPE_BOOL,GetShowX);
			VFN_0(toggleShowY,ToggleShowY);
			FN_0(getShowY,TYPE_BOOL,GetShowY);
			VFN_0(toggleShowZ,ToggleShowZ);
			FN_0(getShowZ,TYPE_BOOL,GetShowZ);
			VFN_0(toggleLayerEdit,ToggleLayerEdit);
			FN_0(getLayerEdit,TYPE_BOOL,GetLayerEdit);
			VFN_0(toggleDrawDuringMove,ToggleDrawDuringMove);
			FN_0(getDrawDuringMove,TYPE_BOOL,GetDrawDuringMove);
			VFN_0(toggleLimit180,ToggleLimit180);
			FN_0(getLimit180,TYPE_BOOL,GetLimit180);
			VFN_0(showQuatCurve,ShowQuatCurve);
			VFN_0(showPosCurve,ShowPosCurve);
			VFN_0(showAngSpeedCurve,ShowAngSpeedCurve);
			VFN_0(showAngAccelCurve,ShowAngAccelCurve);
			VFN_0(showAngJerkCurve,ShowAngJerkCurve);
			VFN_0(showPosSpeedCurve,ShowPosSpeedCurve);
			VFN_0(showPosAccelCurve,ShowPosAccelCurve);
			VFN_0(showPosJerkCurve,ShowPosJerkCurve);
			VFN_0(posCurveToWorld,PosCurveToWorld);
			VFN_0(posCurveToBipRoot,PosCurveToBipRoot);
			VFN_1(posCurveToThisNode,PosCurveToThisNode,TYPE_INODE);
			VFN_0(open,Open);


	END_FUNCTION_MAP


};


class IAnalyzer : public FPStaticInterface
{
public: 
	DECLARE_DESCRIPTOR(IAnalyzer); 
	//note for the analysis functions if angular type is TRUE then the passed in parent node value isn't used.
	//Otherwise if it's  FALSE(it's a position analysis) then the parent node is used to calculate the cooridinate space
	// of the resulting calculation.	
	virtual void  DoNoiseDetectorAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float deviation,BOOL angular,int noiseType,INode *pNode);
	virtual void  DoSpikeDetectorAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float deviation);
	virtual void  DoKneeWobbleAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float frameThreshold,float fluctuationThreshold);
	virtual void  DoKneeExtensionAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float kneeAngle);
	
	virtual Tab<TimeValue >  GetResults(INode *node);

	virtual void LoadAnalysisFile(char *filename);
	virtual void SaveAnalysisFile(Tab<INode *> &nodes,char *filename);
	virtual void ClearAnalysisResults();
	typedef enum {doNoiseDetectorAnalysis=0,doSpikeDetectorAnalysis,getResults,
				loadAnalysisFile,saveAnalysisFile,clearAnalysisResults,
	doKneeWobbleAnalysis,doKneeExtensionAnalysis} AnalyzeFunctions;

	BEGIN_FUNCTION_MAP
			VFN_6(doNoiseDetectorAnalysis, DoNoiseDetectorAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_INODE);
			VFN_3(doSpikeDetectorAnalysis, DoSpikeDetectorAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT);
			FN_1(getResults,TYPE_TIMEVALUE_TAB_BV,GetResults,TYPE_INODE);
			VFN_1(loadAnalysisFile,LoadAnalysisFile,TYPE_STRING);
			VFN_2(saveAnalysisFile,SaveAnalysisFile,TYPE_INODE_TAB_BR,TYPE_STRING);
			VFN_0(clearAnalysisResults,ClearAnalysisResults);
			VFN_4(doKneeWobbleAnalysis, DoKneeWobbleAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT,TYPE_FLOAT);
			VFN_3(doKneeExtensionAnalysis, DoKneeExtensionAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT);
	END_FUNCTION_MAP



};
class IFixer : public FPStaticInterface
{
public:
	DECLARE_DESCRIPTOR(IFixer); 

	virtual void DoAngSmoothing(Tab<INode *>&nodes,int width, float damping);
	virtual void DoAngBlurring(Tab<INode *>&nodes,int width, float damping);
	virtual void DoAdvAngSmoothing(Tab<INode *>&nodes,int width, float damping);
	virtual void DoPosSmoothing(Tab<INode *>&nodes,int width, float damping);
	virtual void DoPosBlurring(Tab<INode *>&nodes,int width, float damping);
	virtual void DoRemoveKeys(Tab<INode *>&nodes,int intervalWidth,BOOL deleteKeys);
	virtual void DoKneeWobbleFix(Tab<INode *>&nodes,float frameThreshold,float fluctuationThreshold);
	virtual void DoKneeExtensionFix(Tab<INode *>&nodes,float kneeAngle);
	typedef enum {  
			doAngSmoothing = 0, doAngBlurring, doAdvAngSmoothing,doPosSmoothing,
			doPosBlurring,doRemoveKeys,doKneeWobbleFix,doKneeExtensionFix
	} FilterFunctions;


	BEGIN_FUNCTION_MAP
			VFN_3(doAngSmoothing, DoAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doAngBlurring, DoAngBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doAdvAngSmoothing, DoAdvAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);

			VFN_3(doPosSmoothing, DoPosSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doPosBlurring, DoPosBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doRemoveKeys, DoRemoveKeys,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_BOOL);
			VFN_3(doKneeWobbleFix, DoKneeWobbleFix,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_FLOAT);
			VFN_2(doKneeExtensionFix, DoKneeExtensionFix,TYPE_INODE_TAB_BR,TYPE_FLOAT);

	END_FUNCTION_MAP

};


class IFilter : public FPStaticInterface
{
public:	
	DECLARE_DESCRIPTOR(IFilter); 

	virtual void DoAngSmoothing(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoAngBlurring(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoAngBoosting(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoAdvAngSmoothing(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoPosSmoothing(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoPosBlurring(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoPosBoosting(Tab<INode *>&nodes,int width, float damping,Interval range);
	virtual void DoKeyReduction(Tab<INode *>&nodes,float tolerance,int keySpacing, float COMTolerance,
		float COMKeySpacing,Interval range);
	virtual void DoKeyPerFrame(Tab<INode *>&nodes);
	virtual void EnablePosSubAnim(Tab<INode *> &nodes,BOOL enable);
	virtual void EnableRotSubAnim(Tab<INode *> &nodes,BOOL enable);
	virtual void EnableScaleSubAnim(Tab<INode *> &nodes,BOOL enable);
	virtual void CollapsePosSubAnim(Tab<INode *> &nodes,BOOL perFrame, BOOL deleteSubAnim);
	virtual void CollapseRotSubAnim(Tab<INode *> &nodes, BOOL perFrame,BOOL deleteSubAnim);
	virtual void CreatePosSubAnim(Tab<INode *> &nodes,Control *toClone,BOOL checkIfOneExists);
	virtual void CreateRotSubAnim(Tab<INode *> &nodes,Control *toClone,BOOL checkIfOneExists);
	virtual void CreateScaleSubAnim(Tab<INode *> &nodes,Control *toClone,BOOL checkIfOneExists);
	virtual void DoKneeWobbleFilter(Tab<INode *>&nodes,float frameThreshold,float fluctuationThreshold,Interval range);
	virtual void DoKneeExtensionFilter(Tab<INode *>&nodes,float kneeAngle,Interval range);

	typedef enum {  
			doAngSmoothing = 0, doAngBlurring,doAngBoosting,doAdvAngSmoothing,doPosSmoothing,
				 doPosBlurring,doPosBoosting,doKeyReduction,doKeyPerFrame,
				 enablePosSubAnim,enableRotSubAnim,enableScaleSubAnim,
				 collapsePosSubAnim,collapseRotSubAnim,createPosSubAnim,createRotSubAnim,
				 createScaleSubAnim,doKneeWobbleFilter,doKneeExtensionFilter
	} FilterFunctions;


	BEGIN_FUNCTION_MAP
			VFN_4(doAngSmoothing, DoAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doAngBlurring, DoAngBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doAngBoosting, DoAngBoosting,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doAdvAngSmoothing, DoAdvAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);

			VFN_4(doPosSmoothing, DoPosSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doPosBlurring, DoPosBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doPosBoosting, DoPosBoosting,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_6(doKeyReduction, DoKeyReduction,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_INT,TYPE_FLOAT,TYPE_INT,TYPE_INTERVAL);
			VFN_1(doKeyPerFrame, DoKeyPerFrame,TYPE_INODE_TAB_BR);
			VFN_2(enablePosSubAnim, EnablePosSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL);
			VFN_2(enableRotSubAnim, EnableRotSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL);
			VFN_2(enableScaleSubAnim, EnableScaleSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL);
			VFN_3(collapsePosSubAnim, CollapsePosSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL,TYPE_BOOL);
			VFN_3(collapseRotSubAnim, CollapseRotSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL,TYPE_BOOL);
			VFN_3(createPosSubAnim, CreatePosSubAnim,TYPE_INODE_TAB_BR,TYPE_CONTROL,TYPE_BOOL);
			VFN_3(createRotSubAnim, CreateRotSubAnim,TYPE_INODE_TAB_BR,TYPE_CONTROL,TYPE_BOOL);
			VFN_3(createScaleSubAnim, CreateScaleSubAnim,TYPE_INODE_TAB_BR,TYPE_CONTROL,TYPE_BOOL);
			VFN_4(doKneeWobbleFilter, DoKneeWobbleFilter,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_3(doKneeExtensionFilter, DoKneeExtensionFilter,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_INTERVAL);
		

	END_FUNCTION_MAP

};




