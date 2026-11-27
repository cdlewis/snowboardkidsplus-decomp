/* Addressing aliases for canonical storage in symbol_addrs.txt.
 * Separate C declarations preserve the original GP-relative/absolute loads.
 * These are linker aliases, not additional objects for Splat to extract.
 */
currentPlayerAbsolute = gCurrentRacePlayer;
selectedPlayerReplayActiveAbsolute = gSelectedPlayerReplayActive;
selectionMenuFadeStartedGp = gSelectionMenuFadeStarted;
activeRacePlayerCountGp = gActiveRacePlayerCount;
playerSnapshotTaskGp = gPlayerSnapshotTask;
playerSnapshotTrailCountGp = gPlayerSnapshotTrailCount;
playerCountAbsolute = gPlayerCount;
taskTransitionFlagsAbsolute = gTaskTransitionFlags;
selectedFinishLapCountAbsolute = gSelectedLapCount;
selectedLapCountAbsolute = gSelectedLapCount;
selectedCourseMenuItemAbsolute = gSelectedCourseMenuItem;
geometryPlacementDataAbsolute = gCourseGeometryRegionData;
previousSelectedCourseIndexAbsolute = gPreviousSelectedCourseIndex;
raceRulesetMenuRememberedSelectionGp = gRaceRulesetMenuRememberedSelection;
shopMenuRememberedSelectionGp = gShopMenuRememberedSelection;
boardShopRememberedSelectionAbsolute = gBoardShopRememberedSelection;
boardMenuRememberedSelectionAbsolute = gMoneyPanelRememberedSelection;
modeTaskTransitionStateGp = gModeTaskTransitionState;

/* GP-addressed view of the saved camera overlay selector. */
selectedCameraOverlayModeGp = gSelectedCameraOverlayMode;
reuseCourseResourcesAbsolute = gReuseCourseResources;
gAttackTimeExpired_gp = gAttackTimeExpired;
raceHudEnabledGp = gRaceHudEnabled;

/* Fourth place uses the first byte of the menu kind palette group. */
gRacePlacementPalettes = gMenuSpritePaletteGroups;

/* Shared three-byte string literal immediately preceding the grid task flag. */
D_8013B514 = gPaletteSelectionCurrentLabelEnabled - 3;
