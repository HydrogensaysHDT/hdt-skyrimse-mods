Scriptname Game Hidden

; Adds the specified achievement to the player's profile
Function AddAchievement(int aiAchievementID) native global

; Add the specified number of perk points to the player
Function AddPerkPoints(int aiPerkPoints) native global

; Advance the given skill on the player by the provided amount of skill usage
Function AdvanceSkill(string asSkillName, float afMagnitude) native global

; Adds a ball-and-socket constraint between two rigid bodies, identified by their ref and node names
bool Function AddHavokBallAndSocketConstraint( ObjectReference arRefA, string arRefANode, ObjectReference arRefB, string arRefBNode, float afRefALocalOffsetX = 0.0, float afRefALocalOffsetY = 0.0, float afRefALocalOffsetZ = 0.0, float afRefBLocalOffsetX = 0.0, float afRefBLocalOffsetY = 0.0, float afRefBLocalOffsetZ = 0.0) native global

; Removes any constraint between two rigid bodies
bool Function RemoveHavokConstraints(ObjectReference arFirstRef, string arFirstRefNodeName, ObjectReference arSecondRef, string arSecondRefNodeName) native global

; Calculates how much a x point favor would cost the player
int Function CalculateFavorCost(int aiFavorPrice) native global

; Clears the prison variables on the player 
Function ClearPrison() native global

; Clears temp effects from game
Function ClearTempEffects() native global

; Disables the user's controls
Function DisablePlayerControls(bool abMovement = true, bool abFighting = true, bool abCamSwitch = false, bool abLooking = false, \
  bool abSneaking = false, bool abMenu = true, bool abActivate = true, bool abJournalTabs = false, int aiDisablePOVType = 0) native global
  
; Enables the user's controls
Function EnablePlayerControls(bool abMovement = true, bool abFighting = true, bool abCamSwitch = true, bool abLooking = true, \
  bool abSneaking = true, bool abMenu = true, bool abActivate = true, bool abJournalTabs = true, int aiDisablePOVType = 0) native global
  
; Enables or disables the ability to fast travel
Function EnableFastTravel(bool abEnable = true) native global

; Fades out the game to black, or vice versa
Function FadeOutGame(bool abFadingOut, bool abBlackFade, float afSecsBeforeFade, float afFadeDuration) native global
  
; Fast-travels the player to the specified object's location
Function FastTravel(ObjectReference akDestination) native global

; Finds the closest reference of a given base object within a given radius of a location
ObjectReference Function FindClosestReferenceOfType(Form arBaseObject, float afX, float afY, float afZ, float afRadius) native global

; Finds a random reference of a given base object within a given radius of a location
ObjectReference Function FindRandomReferenceOfType(Form arBaseObject, float afX, float afY, float afZ, float afRadius) native global

; Finds the closest reference of any base object in the list within a given radius of a location
ObjectReference Function FindClosestReferenceOfAnyTypeInList(FormList arBaseObjects, float afX, float afY, float afZ, float afRadius) native global

; Finds a random reference of a any base object in the list within a given radius of a location
ObjectReference Function FindRandomReferenceOfAnyTypeInList(FormList arBaseObjects, float afX, float afY, float afZ, float afRadius) native global

; Finds the closest reference of a given base object within a given radius of a reference
ObjectReference Function FindClosestReferenceOfTypeFromRef(Form arBaseObject, ObjectReference arCenter, float afRadius) global
	return FindClosestReferenceOfType(arBaseObject, arCenter.X, arCenter.Y, arCenter.Z, afRadius)
endFunction

; Finds a random reference of a given base object within a given radius of a reference
ObjectReference Function FindRandomReferenceOfTypeFromRef(Form arBaseObject, ObjectReference arCenter, float afRadius) global
	return FindRandomReferenceOfType(arBaseObject, arCenter.X, arCenter.Y, arCenter.Z, afRadius)
endFunction

; Finds the closest reference of a given base object within a given radius of a reference
ObjectReference Function FindClosestReferenceOfAnyTypeInListFromRef(FormList arBaseObjects, ObjectReference arCenter, float afRadius) global
	return FindClosestReferenceOfAnyTypeInList(arBaseObjects, arCenter.X, arCenter.Y, arCenter.Z, afRadius)
endFunction

; Finds a random reference of a given base object within a given radius of a reference
ObjectReference Function FindRandomReferenceOfAnyTypeInListFromRef(FormList arBaseObjects, ObjectReference arCenter, float afRadius) global
	return FindRandomReferenceOfAnyTypeInList(arBaseObjects, arCenter.X, arCenter.Y, arCenter.Z, afRadius)
endFunction

; Finds the closest actor within a given radius of a location
Actor Function FindClosestActor(float afX, float afY, float afZ, float afRadius) native global

; Finds a random actor within a given radius of a location
Actor Function FindRandomActor(float afX, float afY, float afZ, float afRadius) native global

; Finds the closest actor within a given radius of a reference
Actor Function FindClosestActorFromRef(ObjectReference arCenter, float afRadius) global
	return FindClosestActor(arCenter.X, arCenter.Y, arCenter.Z, afRadius)
endFunction

; Finds a random actor within a given radius of a reference
Actor Function FindRandomActorFromRef(ObjectReference arCenter, float afRadius) global
	return FindRandomActor(arCenter.X, arCenter.Y, arCenter.Z, afRadius)
endFunction

; Make the player got to 3rd person camera mode
Function ForceThirdPerson() native global

; Make the player got to 1st person camera mode
Function ForceFirstPerson() native global

; Show the players first person geometry.
Function ShowFirstPersonGeometry( bool abShow = true ) native global

; Returns the form specified by the ID
Form Function GetForm(int aiFormID) native global

; Returns the form specified by the ID originating in the given file
Form Function GetFormFromFile(int aiFormID, string asFilename) native global

; Obtains the value of a game setting - one for each type of game setting
float Function GetGameSettingFloat(string asGameSetting) native global
int Function GetGameSettingInt(string asGameSetting) native global
string Function GetGameSettingString(string asGameSetting) native global

; Returns the player actor
Actor Function GetPlayer() native global

; Returns the reference the player is currently grabbing
ObjectReference Function GetPlayerGrabbedRef() native global

; Returns the horse last ridden by the player
Actor Function GetPlayersLastRiddenHorse() native global

; Returns the X position of the Sun.
float Function GetSunPositionX() native global

; Returns the Y position of the Sun.
float Function GetSunPositionY() native global

; Returns the Z position of the Sun.
float Function GetSunPositionZ() native global

; Returns the number of days spent in play
float Function GetRealHoursPassed() native global

; Increment the given skill on the player by the one point
Function IncrementSkill(string asSkillName) native global

; Increment the given skill on the player by the given number of points
Function IncrementSkillBy(string asSkillName, int aiCount) native global

; Modifies the specified MiscStat by the given amount.
Function IncrementStat(string asStatName, int aiModAmount = 1) native global

; Are the activation controls enabled?
bool Function IsActivateControlsEnabled() native global

; Are the camera switch controls enabled?
bool Function IsCamSwitchControlsEnabled() native global

; Is fast travel controls enabled? Returns false if EnableFastTravel(false) has been called
bool Function IsFastTravelControlsEnabled() native global

; Is fast travel enabled?
bool Function IsFastTravelEnabled() native global

; Are the fighting controls enabled?
bool Function IsFightingControlsEnabled() native global

; Are the journal menu controls enabled?
bool Function IsJournalControlsEnabled() native global

; Are the looking controls enabled?
bool Function IsLookingControlsEnabled() native global

; Are the menu controls enabled?
bool Function IsMenuControlsEnabled() native global

; Are the movement controls enabled?
bool Function IsMovementControlsEnabled() native global

; Is the player looking at the sun?
bool Function IsPlayerSungazing() native global

; Are the sneaking controls enabled?
bool Function IsSneakingControlsEnabled() native global

; Is the specified Word of Power Unlocked?
bool Function IsWordUnlocked(WordOfPower akWord) native global

; Plays a bink video - does not return until bink has finished, use with care!
Function PlayBink(string asFileName, bool abInterruptible = false, bool abMuteAudio = true, bool abMuteMusic = true, \
  bool abLetterbox = true ) native global

; Precaches character gen data.
Function PrecacheCharGen() native global

; Clears Precached character gen data.
Function PrecacheCharGenClear() native global

; Queries the given stat and returns its value
int Function QueryStat(string asStat) native global

; Forces the game back to the main menu
Function QuitToMainMenu() native global

; Request that an auto-save be made
Function RequestAutoSave() native global

; Requests the specified model
Function RequestModel(string asModelName) native global

; Request that a normal save be made
Function RequestSave() native global

; Has the player serve their prison time
Function ServeTime() native global

; Finds an actor in high who can detect the player to call werewolf crime on the player
Function SendWereWolfTransformation() native global

; Called as we enter/exit beast form
Function SetBeastForm(bool abEntering) native global

; Sets the camera target actor
Function SetCameraTarget(Actor arTarget) native global

; Sets or clears "cart mode" for the HUD
Function SetHudCartMode(bool abSetCartMode = true) native global

; Informs the game whether we are in CharGen or not
Function SetInChargen(bool abDisableSaving, bool abDisableWaiting, bool abShowControlsDisabledMessage) native global

; Enables or disables the AI driven flag on Player
Function SetPlayerAIDriven(bool abAIDriven = true) native global

; Enables or disables  crime reporting on Player
Function SetPlayerReportCrime(bool abReportCrime = true) native global

; Set the players sitting camera rotation - in degrees, offset from the standard angle.
Function SetSittingRotation(float afValue) native global

; Shakes the object from the location of the passed-in object. If none, it will shake the camera from the player's location.
; Strength is clamped from 0 to 1
; Duration in seconds. By default (0.0) use the game setting.
Function ShakeCamera(ObjectReference akSource = None, float afStrength = 0.5, float afDuration = 0.0) native global

; Shakes the controller for the specified length of time (in seconds). The strength values are clamped from 0 to 1
Function ShakeController(float afSmallMotorStrength, float afBigMotorStreangth, float afDuration) native global

; Displays the race/sex menu
Function ShowRaceMenu() native global
Function ShowLimitedRaceMenu() native global

; Title Sequence menu functions
Function ShowTitleSequenceMenu() native global
Function HideTitleSequenceMenu() native global
Function StartTitleSequence(string asSequenceName) native global

; Allow or disallow player requests to have a flying mount land.
Function SetAllowFlyingMountLandingRequests(bool abAllow) native global

; Sets the Image Space Modifier that is triggered when the player gazes at the sun.
Function SetSunGazeImageSpaceModifier(ImageSpaceModifier apImod = NONE ) native global

; Displays the training menu based on passed in trainer actor
Function ShowTrainingMenu(Actor aTrainer) native global

; Teaches the specified word of power to the player
Function TeachWord(WordOfPower akWord) native global

; Trigger screen blood with the given count
Function TriggerScreenBlood(int aiValue) native global

; Unlocks the specified word of power so the player can use it
Function UnlockWord(WordOfPower akWord) native global

; Returns true if we're using a gamepad
bool Function UsingGamepad() native global


; SKSE64 additions built 2019-11-21 05:06:53.443000 UTC
; Get/Set Perk Points
int Function GetPerkPoints() global native
Function SetPerkPoints(int perkPoints) global native
Function ModPerkPoints(int perkPoints) global native

; returns the number of active mods
int Function GetModCount() native global

; returns the index of the specified mod
int Function GetModByName(string name) native global

; returns the name of the mod at the specified modIndex
string Function GetModName(int modIndex) native global

; returns the author of the mod at the specified modIndex
string Function GetModAuthor(int modIndex) native global

; returns the description of the mod at the specified modIndex
string Function GetModDescription(int modIndex) native global

; gets the count of mods the specified mod depends upon
int Function GetModDependencyCount(int modIndex) native global

; gets the index of the nth mod dependency of the specfied mod
; int Function GetNthModDependency(int modIndex, int n) native global

bool Function IsPluginInstalled(string name) native global

; light mod functions
int Function GetLightModCount() native global
int Function GetLightModByName(string name) native global
string Function GetLightModName(int idx) native global
string Function GetLightModAuthor(int idx) native global
string Function GetLightModDescription(int idx) native global
int Function GetLightModDependencyCount(int idx) native global
int Function GetNthLightModDependency(int modIdx, int idx) native global

; GameSetting functions - SKSE 1.5.10
Function SetGameSettingFloat(string setting, float value) global native
Function SetGameSettingInt(string setting, int value) global native
Function SetGameSettingBool(string setting, bool value) global native
Function SetGameSettingString(string setting, string value) global native

; save/load game
Function SaveGame(string name) native global
Function LoadGame(string name) native global

; TintMasks (AARRGGBB)

; Returns the total number of tints for the player
int Function GetNumTintMasks() native global

; Returns the color of the Nth tint mask
int Function GetNthTintMaskColor(int n) native global

; Returns the type of the Nth tint mask
int Function GetNthTintMaskType(int n) native global

; Sets the color of the Nth tint mask
Function SetNthTintMaskColor(int n, int color) native global

; Returns the texture path of the Nth tint mask
string Function GetNthTintMaskTexturePath(int n) native global

; Sets the texturepath of the Nth tint mask
Function SetNthTintMaskTexturePath(string path, int n) native global

; Types
; 0 - Frekles
; 1 - Lips
; 2 - Cheeks
; 3 - Eyeliner
; 4 - Upper Eyesocket
; 5 - Lower Eyesocket
; 6 - SkinTone
; 7 - Warpaint
; 8 - Frownlines
; 9 - Lower Cheeks
; 10 - Nose
; 11 - Chin
; 12 - Neck
; 13 - Forehead
; 14 - Dirt

; Returns how many indexes there are for this type
int Function GetNumTintsByType(int type) native global

; Returns the color for the particular tintMask type and index
int Function GetTintMaskColor(int type, int index) global native

; Sets the tintMask color for the particular type and index
Function SetTintMaskColor(int color, int type, int index) global native

; Returns the texture path for the particular tintMask type and index
string Function GetTintMaskTexturePath(int type, int index) global native

; Sets the tintMask texture for the particular type and index
Function SetTintMaskTexturePath(string path, int type, int index) global native

; Updates tintMask colors without updating the entire model
Function UpdateTintMaskColors() global native

; Updates the players hair color immediately
Function UpdateHairColor() global native

; Returns the character's current camera state
; 0 - first person
; 1 - auto vanity
; 2 - VATS
; 3 - free
; 4 - iron sights
; 5 - furniture
; 6 - transition
; 7 - tweenmenu
; 8 - third person 1
; 9 - third person 2
; 10 - horse
; 11 - bleedout
; 12 - dragon
int Function GetCameraState() global
	return Camera.GetCameraState()
EndFunction

; set a misc stat value
; use QueryStat to read the value
Function SetMiscStat(string name, int value) global native

; Sets the players last ridden horse, None will clear the lastRiddenHorse
Function SetPlayersLastRiddenHorse(Actor horse) global native

; Returns the legendary level for the skill
; -1 indicates the particular skill cannot have a legendary level
; DEPRECATED
int Function GetSkillLegendaryLevel(string actorValue) global
	return ActorValueInfo.GetActorValueInfoByName(actorValue).GetSkillLegendaryLevel()
EndFunction

; Sets the legendary level for the skill
; DEPRECATED
Function SetSkillLegendaryLevel(string actorValue, int level) global
	ActorValueInfo.GetActorValueInfoByName(actorValue).SetSkillLegendaryLevel(level)
EndFunction

; Returns the players experience for this level (not total experience)
float Function GetPlayerExperience() global native

; Sets the players experience, does not trigger level-up notification
Function SetPlayerExperience(float exp) global native

; Calculates the experience required for to level-up
; (fXPLevelUpBase + currentLevel * fXPLevelUpMult)
float Function GetExperienceForLevel(int currentLevel) global native

; Returns true if in run mode, false if in walk mode
; Does not reflect actual movement state, only the control mode
bool Function GetPlayerMovementMode() global native

; Updates the camera when changing Shoulder positions
Function UpdateThirdPerson() global
	Camera.UpdateThirdPerson()
EndFunction

; Hotkeys 0-7 reflect keys 1-8
; Unbinds a favorited item bound to the specified hotkey
Function UnbindObjectHotkey(int hotkey) global native

; Returns the base form object that is bound to the specified hotkey
Form Function GetHotkeyBoundObject(int hotkey) global native

; Returns if base form is favorited by the player
bool Function IsObjectFavorited(Form form) global native

; Same as GetForm, but also works for formIds >= 0x80000000
Form Function GetFormEx(int formId) global native

; Returns the object reference the player is in dialogue with
ObjectReference Function GetDialogueTarget() global native

; Returns the current crosshair ref
ObjectReference Function GetCurrentCrosshairRef() global native

; Returns the currently selected ref in the console
ObjectReference Function GetCurrentConsoleRef() global native

; Sets the player level
Function SetPlayerLevel(int level) global native