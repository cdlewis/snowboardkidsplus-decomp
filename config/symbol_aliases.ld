/* Addressing aliases for canonical storage in symbol_addrs.txt.
 * Separate C declarations preserve the original GP-relative/absolute loads.
 * These are linker aliases, not additional objects for Splat to extract.
 */
debugPositionXFormatAbsolute = gDebugPositionXFormat;
debugPositionYFormatAbsolute = gDebugPositionYFormat;
debugPositionZFormatAbsolute = gDebugPositionZFormat;
currentPlayerAbsolute = D_8013B0D8;
raceFinishFlagAbsolute = D_8013AFD6;
readPixelRectGp = gReadPixelRect;
gameOptionPlayerModeGp = D_8013AFCD;
selectionMenuFadeStartedGp = D_8013B41F;
raceResultsModeGp = D_8013AFE1;
activeRacePlayerCountGp = D_8013AFD2;
playerSnapshotTaskGp = D_8013B304;
gameOptionPlayerModeAbsolute = D_8013AFCD;
taskTransitionFlagsAbsolute = D_8013AFE4;
selectedCourseIndexAbsolute = D_8013AFCE;
selectedFinishLapCountAbsolute = D_8013AFD0;
geometryPlacementDataAbsolute = D_8013B1F4;
selectionMenuRecordAbsolute = D_8013B560;
mainTailSelectionGp = D_8013B527;
taskTransitionFlagGp = D_8013B55E;
modeTransitionReadyGp = D_8013B55E;

/* GP-addressed view of the saved camera overlay selector. */
selectedCameraOverlayModeGp = D_8013B20C;
playbackTransitionByteAbsolute = D_8013B562;
scanlineCopyRectGp = gScanlineCopyRect;
selectedCameraStateGp = D_8013B20C;
D_8013B18D_gp = D_8013B18D;
D_8013B1A8_gp = D_8013B1A8;

/* Shared three-byte string literal immediately preceding the grid task flag. */
D_8013B514 = D_8013B517 - 3;

/* Registered callback tasks address the shared widget colors absolutely. */
gTaskWidgetRedAbsolute = D_8013B3A4;
gTaskWidgetGreenAbsolute = D_8013B3A5;
gTaskWidgetBlueAbsolute = D_8013B3A6;

/* The external view preserves GP-relative fixed accesses and absolute indexed accesses. */
gPlayerGroupedSelectionStatesReference = gPlayerGroupedSelectionStates;
