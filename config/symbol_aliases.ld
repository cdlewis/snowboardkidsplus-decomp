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
selectionMenuRecordAbsolute = D_8013B560;
mainTailSelectionGp = D_8013B527;
shopMenuRememberedSelectionGp = gShopMenuRememberedSelection;
boardShopRememberedSelectionAbsolute = gBoardShopRememberedSelection;
boardMenuRememberedSelectionAbsolute = gMoneyPanelRememberedSelection;
taskTransitionFlagGp = D_8013B55E;

/* GP-addressed view of the saved camera overlay selector. */
selectedCameraOverlayModeGp = D_8013B20C;
playbackTransitionByteAbsolute = D_8013B562;
gAttackTimeExpired_gp = gAttackTimeExpired;
D_8013B1A8_gp = D_8013B1A8;

/* Fourth place uses the first byte of the menu kind palette group. */
gRacePlacementPalettes = gMenuSpritePaletteGroups;

/* Shared three-byte string literal immediately preceding the grid task flag. */
D_8013B514 = D_8013B517 - 3;
