# makefile for a few functions in the manual directory of PDP++

all:	pdp_all ta_all clean help_links

clean:
	rm *.aux *.cp *.cps *.dvi *.fn *.fns *.ky *.log *.pg *.toc *.tp *.tps *.vr *.vrs

pdp_all: pdp_toinfo pdp_tohtml pdp_tops

ta_all: ta_toinfo ta_tohtml ta_tops

pdp_toinfo:
	makeinfo --fill-column=70 pdp-user.texi
	mv pdp-user-[0-9]* pdp-user info

# expect errors for this due to bad references..
ta_toinfo:
	- makeinfo --force --fill-column=70 ta_css.texi
	mv ta_css-[0-9]* ta_css info

pdp_tohtml_new:
	cd html;\
	rm pdp-user*.html; rm pdp-user*.gif;\
	texi2html -I icons -number -split_node -verbose -menu ../pdp-user.texi

pdp_tohtml:
	cd html;\
	rm pdp-user*.html; rm pdp-user*.gif;\
	../texi2html -b icons -tex -number -split_node -verbose -menu -maxtoc 3 ../pdp-user.texi;\
	/bin/rm base.gif fcn.gif fnc.gif next2.gif next.gif prev.gif toc.gif top.gif up.gif var.gif;\
	/bin/cp -f icons/*.gif .

pdp_tohtml_notex:
	cd html;\
	rm pdp-user*.html;\
	../texi2html -b icons -number -split_node -verbose -menu -maxtoc 3 ../pdp-user.texi

ta_tohtml:
	cd html;\
	rm ta_css*.html;\
	../texi2html -b icons -number -split_node -verbose -menu -maxtoc 3 ../ta_css.texi

pdp_tops:
	texi2dvi pdp-user.texi
	dvips -o pdp-user.ps pdp-user.dvi
	ps2pdf pdp-user.ps
	- rm pdp-user.ps.gz
	gzip pdp-user.ps

ta_tops:
	texi2dvi ta_css.texi
	dvips -o ta_css.ps ta_css.dvi
	ps2pdf ta_css.ps
	- rm ta_css.ps.gz
	gzip ta_css.ps

# get these links from the html file containing the Type Index.
# this was pdp-user_266.html last time.

help_links:
	cd html;\
	/bin/rm -f Help_*;\
	ln -s pdp-user_177.html Help_ActThreshRTStat.html;\
	ln -s pdp-user_168.html Help_Aggregate.html;\
	ln -s pdp-user_214.html Help_APBpCycle.html;\
	ln -s pdp-user_214.html Help_APBpMaxDa_De.html;\
	ln -s pdp-user_214.html Help_APBpSettle.html;\
	ln -s pdp-user_214.html Help_APBpTrial.html;\
	ln -s pdp-user_96.html Help_Array.html;\
	ln -s pdp-user_154.html Help_BatchProcess.html;\
	ln -s pdp-user_200.html Help_BpCon.html;\
	ln -s pdp-user_200.html Help_BpConSpec.html;\
	ln -s pdp-user_204.html Help_BpContextSpec.html;\
	ln -s pdp-user_202.html Help_BpTrial.html;\
	ln -s pdp-user_201.html Help_BpUnit.html;\
	ln -s pdp-user_201.html Help_BpUnitSpec.html;\
	ln -s pdp-user_203.html Help_BumpBpUnitSpec.html;\
	ln -s pdp-user_172.html Help_CE_Stat.html;\
	ln -s pdp-user_231.html Help_ClConSpec.html;\
	ln -s pdp-user_166.html Help_ClearLogProc.html;\
	ln -s pdp-user_232.html Help_ClLayerSpec.html;\
	ln -s pdp-user_174.html Help_ClosestEventStat.html;\
	ln -s pdp-user_61.html Help_ColorScaleSpec.html;\
	ln -s pdp-user_175.html Help_CompareStat.html;\
	ln -s pdp-user_175.html Help_ComputeStat.html;\
	ln -s pdp-user_127.html Help_Con_Group.html;\
	ln -s pdp-user_127.html Help_Connection.html;\
	ln -s pdp-user_127.html Help_ConSpec.html;\
	ln -s pdp-user_179.html Help_CopyToEnvStat.html;\
	ln -s pdp-user_219.html Help_CsCon.html;\
	ln -s pdp-user_219.html Help_CsConSpec.html;\
	ln -s pdp-user_221.html Help_CsCycle.html;\
	ln -s pdp-user_224.html Help_CsDistStat.html;\
	ln -s pdp-user_223.html Help_CsGoodStat.html;\
	ln -s pdp-user_225.html Help_CsMaxDa.html;\
	ln -s pdp-user_221.html Help_CsSample.html;\
	ln -s pdp-user_221.html Help_CsSettle.html;\
	ln -s pdp-user_224.html Help_CsTargStat.html;\
	ln -s pdp-user_224.html Help_CsTIGstat.html;\
	ln -s pdp-user_221.html Help_CsTrial.html;\
	ln -s pdp-user_220.html Help_CsUnit.html;\
	ln -s pdp-user_220.html Help_CsUnitSpec.html;\
	ln -s pdp-user_124.html Help_CustomPrjnSpec.html;\
	ln -s pdp-user_160.html Help_CycleProcess.html;\
	ln -s pdp-user_179.html Help_CyclesToSettle.html;\
	ln -s pdp-user_196.html Help_DA_GridViewSpec.html;\
	ln -s pdp-user_203.html Help_DeltaBarDeltaBpCon.html;\
	ln -s pdp-user_203.html Help_DeltaBarDeltaBpConSpec.html;\
	ln -s pdp-user_166.html Help_DispDataEnvProc.html;\
	ln -s pdp-user_166.html Help_DispNetWeightsProc.html;\
	ln -s pdp-user_196.html Help_DT_GridViewSpec.html;\
	ln -s pdp-user_135.html Help_Environment.html;\
	ln -s pdp-user_140.html Help_EnviroView.html;\
	ln -s pdp-user_178.html Help_EpochCounterStat.html;\
	ln -s pdp-user_156.html Help_EpochProcess.html;\
	ln -s pdp-user_203.html Help_ErrScaleBpConSpec.html;\
	ln -s pdp-user_137.html Help_Event.html;\
	ln -s pdp-user_137.html Help_EventSpec.html;\
	ln -s pdp-user_203.html Help_ExpBpUnitSpec.html;\
	ln -s pdp-user_164.html Help_ForkProcess.html;\
	ln -s pdp-user_146.html Help_FreqEnv.html;\
	ln -s pdp-user_146.html Help_FreqEvent.html;\
	ln -s pdp-user_146.html Help_FreqEvent_MGroup.html;\
	ln -s pdp-user_120.html Help_FullPrjnSpec.html;\
	ln -s pdp-user_123.html Help_GpFullPrjnSpec.html;\
	ln -s pdp-user_123.html Help_GpOneToManyPrjnSpec.html;\
	ln -s pdp-user_123.html Help_GpOneToOnePrjnSpec.html;\
	ln -s pdp-user_188.html Help_GraphLog.html;\
	ln -s pdp-user_188.html Help_GraphLogView.html;\
	ln -s pdp-user_189.html Help_DA_GraphViewSpec.html;\
	ln -s pdp-user_194.html Help_GridLog.html;\
	ln -s pdp-user_194.html Help_GridLogView.html;\
	ln -s pdp-user_154.html Help_GridSearchBatch.html;\
	ln -s pdp-user_203.html Help_HebbBpConSpec.html;\
	ln -s pdp-user_231.html Help_HebbConSpec.html;\
	ln -s pdp-user_245.html Help_ImRewLayerSpec.html;\
	ln -s pdp-user_166.html Help_InitWtsProc.html;\
	ln -s pdp-user_163.html Help_InteractiveEpoch.html;\
	ln -s pdp-user_147.html Help_InteractiveScriptEnv.html;\
	ln -s pdp-user_115.html Help_Layer.html;\
	ln -s pdp-user_237.html Help_LeabraBiasSpec.html;\
	ln -s pdp-user_237.html Help_LeabraCon.html;\
	ln -s pdp-user_237.html Help_LeabraConSpec.html;\
	ln -s pdp-user_245.html Help_LeabraContextLayerSpec.html;\
	ln -s pdp-user_240.html Help_LeabraCycle.html;\
	ln -s pdp-user_242.html Help_LeabraGoodStat.html;\
	ln -s pdp-user_239.html Help_LeabraLayer.html;\
	ln -s pdp-user_239.html Help_LeabraLayerSpec.html;\
	ln -s pdp-user_243.html Help_LeabraMaxDa.html;\
	ln -s pdp-user_245.html Help_LeabraNegBiasSpec.html;\
	ln -s pdp-user_240.html Help_LeabraSettle.html;\
	ln -s pdp-user_245.html Help_LeabraTabledConSpec.html;\
	ln -s pdp-user_245.html Help_LeabraTimeConSpec.html;\
	ln -s pdp-user_245.html Help_LeabraTimeUnit.html;\
	ln -s pdp-user_245.html Help_LeabraTimeUnitSpec.html;\
	ln -s pdp-user_240.html Help_LeabraTrial.html;\
	ln -s pdp-user_238.html Help_LeabraUnit.html;\
	ln -s pdp-user_238.html Help_LeabraUnitSpec.html;\
	ln -s pdp-user_245.html Help_LeabraWiz.html;\
	ln -s pdp-user_245.html Help_LebraACLayerSpec.html;\
	ln -s pdp-user_203.html Help_LinearBpUnitSpec.html;\
	ln -s pdp-user_124.html Help_LinkPrjnSpec.html;\
	ln -s pdp-user_166.html Help_LoadWtsProc.html;\
	ln -s pdp-user_184.html Help_LogView.html;\
	ln -s pdp-user_245.html Help_MarkerConSpec.html;\
	ln -s pdp-user_245.html Help_MatrixLayerSpec.html;\
	ln -s pdp-user_245.html Help_MatrixUnitSpec.html;\
	ln -s pdp-user_172.html Help_MaxActTrgStat.html;\
	ln -s pdp-user_231.html Help_MaxInConSpec.html;\
	ln -s pdp-user_252.html Help_MemberDef.html;\
	ln -s pdp-user_252.html Help_MethodDef.html;\
	ln -s pdp-user_173.html Help_MonitorStat.html;\
	ln -s pdp-user_164.html Help_MultiEnvProcess.html;\
	ln -s pdp-user_109.html Help_NameValue.html;\
	ln -s pdp-user_155.html Help_NEpochProcess.html;\
	ln -s pdp-user_187.html Help_NetLog.html;\
	ln -s pdp-user_187.html Help_NetLogView.html;\
	ln -s pdp-user_129.html Help_NetView.html;\
	ln -s pdp-user_112.html Help_Network.html;\
	ln -s pdp-user_203.html Help_NoisyBpUnitSpec.html;\
	ln -s pdp-user_213.html Help_NoisyRBpUnitSpec.html;\
	ln -s pdp-user_124.html Help_OneToOnePrjnSpec.html;\
	ln -s pdp-user_245.html Help_PatchLayerSpec.html;\
	ln -s pdp-user_137.html Help_Pattern.html;\
	ln -s pdp-user_137.html Help_PatternSpec.html;\
	ln -s pdp-user_181.html Help_PDPLog.html;\
	ln -s pdp-user_245.html Help_PFCLayerSpec.html;\
	ln -s pdp-user_245.html Help_PhaseOrderEventSpec.html;\
	ln -s pdp-user_122.html Help_PolarRndPrjnSpec.html;\
	ln -s pdp-user_227.html Help_ProbEventSpec.html;\
	ln -s pdp-user_227.html Help_ProbPattern.html;\
	ln -s pdp-user_227.html Help_ProbPatternSpec_Group.html;\
	ln -s pdp-user_178.html Help_ProcCounterStat.html;\
	ln -s pdp-user_102.html Help_Project.html;\
	ln -s pdp-user_117.html Help_Projection.html;\
	ln -s pdp-user_118.html Help_ProjectionSpec.html;\
	ln -s pdp-user_179.html Help_ProjectionStat.html;\
	ln -s pdp-user_101.html Help_Random.html;\
	ln -s pdp-user_203.html Help_RBFBpUnitSpec.html;\
	ln -s pdp-user_172.html Help_RBpSE_Stat.html;\
	ln -s pdp-user_211.html Help_RBpTrial.html;\
	ln -s pdp-user_210.html Help_RBpUnitSpec.html;\
	ln -s pdp-user_245.html Help_RewPredLayerSpec.html;\
	ln -s pdp-user_61.html Help_RGBA.html;\
	ln -s pdp-user_166.html Help_SaveNetsProc.html;\
	ln -s pdp-user_166.html Help_SaveWtsProc.html;\
	ln -s pdp-user_245.html Help_ScalarValLayerSpec.html;\
	ln -s pdp-user_150.html Help_SchedProcess.html;\
	ln -s pdp-user_110.html Help_Script.html;\
	ln -s pdp-user_147.html Help_ScriptEnv.html;\
	ln -s pdp-user_124.html Help_ScriptPrjnSpec.html;\
	ln -s pdp-user_179.html Help_ScriptStat.html;\
	ln -s pdp-user_172.html Help_SE_Stat.html;\
	ln -s pdp-user_111.html Help_SelectEdit.html;\
	ln -s pdp-user_162.html Help_SequenceEpoch.html;\
	ln -s pdp-user_162.html Help_SequenceProcess.html;\
	ln -s pdp-user_159.html Help_SettleProcess.html;\
	ln -s pdp-user_103.html Help_SimLog.html;\
	ln -s pdp-user_245.html Help_SNcLayerSpec.html;\
	ln -s pdp-user_231.html Help_SoCon.html;\
	ln -s pdp-user_231.html Help_SoCon_Group.html;\
	ln -s pdp-user_231.html Help_SoConSpec.html;\
	ln -s pdp-user_231.html Help_SoftClConSpec.html;\
	ln -s pdp-user_232.html Help_SoftClLayerSpec.html;\
	ln -s pdp-user_232.html Help_SoftClUnitSpec.html;\
	ln -s pdp-user_203.html Help_SoftMaxBpUnitSpec.html;\
	ln -s pdp-user_232.html Help_SomLayerSpec.html;\
	ln -s pdp-user_232.html Help_SomUnitSpec.html;\
	ln -s pdp-user_232.html Help_SoUnit.html;\
	ln -s pdp-user_232.html Help_SoUnitSpec.html;\
	ln -s pdp-user_167.html Help_Stat.html;\
	ln -s pdp-user_169.html Help_StatVal.html;\
	ln -s pdp-user_203.html Help_StochasticBpUnitSpec.html;\
	ln -s pdp-user_124.html Help_SymmetricPrjnSpec.html;\
	ln -s pdp-user_164.html Help_SyncEpochProc.html;\
	ln -s pdp-user_253.html Help_taBase.html;\
	ln -s pdp-user_58.html Help_taMisc.html;\
	ln -s pdp-user_245.html Help_TdModUnit.html;\
	ln -s pdp-user_245.html Help_TdModUnitSpec.html;\
	ln -s pdp-user_121.html Help_TessEl.html;\
	ln -s pdp-user_121.html Help_TesselPrjnSpec.html;\
	ln -s pdp-user_186.html Help_TextLog.html;\
	ln -s pdp-user_186.html Help_TextLogView.html;\
	ln -s pdp-user_203.html Help_ThreshLinBpUnitSpec.html;\
	ln -s pdp-user_178.html Help_TimeCounterStat.html;\
	ln -s pdp-user_166.html Help_TimeCounterStatResetProc.html;\
	ln -s pdp-user_212.html Help_TimeEnvironment.html;\
	ln -s pdp-user_212.html Help_TimeEvent.html;\
	ln -s pdp-user_212.html Help_TimeEvent_MGroup.html;\
	ln -s pdp-user_155.html Help_TrainProcess.html;\
	ln -s pdp-user_158.html Help_TrialProcess.html;\
	ln -s pdp-user_252.html Help_TypeDef.html;\
	ln -s pdp-user_109.html Help_TypeDefault.html;\
	ln -s pdp-user_122.html Help_UniformRandomPrjnSpec.html;\
	ln -s pdp-user_125.html Help_Unit.html;\
	ln -s pdp-user_115.html Help_Unit_Group.html;\
	ln -s pdp-user_176.html Help_UnitActRFStat.html;\
	ln -s pdp-user_166.html Help_UnitActRFStatResetProc.html;\
	ln -s pdp-user_125.html Help_UnitSpec.html;\
	ln -s pdp-user_48.html Help_WinBase.html;\
	ln -s pdp-user_50.html Help_WinView.html;\
	ln -s pdp-user_45.html Help_Wizard.html;\
	ln -s pdp-user_147.html Help_XYPattern.html;\
	ln -s pdp-user_147.html Help_XYPatternSpec.html;\
	ln -s pdp-user_231.html Help_ZshConSpec.html
