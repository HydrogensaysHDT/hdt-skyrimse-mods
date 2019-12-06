Scriptname Faction extends Form Hidden

; Checks to see if the player can pay the crime gold for this faction
bool Function CanPayCrimeGold() native

; Gets the amount of gold the player is to pay to this faction for crimes
int Function GetCrimeGold() native

; Gets the amount of gold the player is to pay to this faction for non-violent crimes
int Function GetCrimeGoldNonViolent() native

; Gets the amount of gold the player is to pay to this faction for violent crimes
int Function GetCrimeGoldViolent() native

; Get the player's "infamy" with this faction (accumulated crime gold)
int Function GetInfamy() native

; Get the player's "non-violent infamy" with this faction (accumulated non-violent crime gold)
int Function GetInfamyNonViolent() native

; Get the player's "violent infamy" with this faction (accumulated violent crime gold)
int Function GetInfamyViolent() native

; Gets this faction's reaction towards the other
int Function GetReaction(Faction akOther) native

; Obtains the value of all items stolen by the player from this faction that was witnessed
int Function GetStolenItemValueCrime() native

; Obtains the value of all items stolen by the player from this faction that was NOT witnessed
int Function GetStolenItemValueNoCrime() native

; Is the passed in faction in this faction's crime group
bool Function IsFactionInCrimeGroup(Faction akOther) native

; Is the player expelled from this faction?
bool Function IsPlayerExpelled() native

; Modifies the amount of crime gold for this faction - violent or non-violent
Function ModCrimeGold(int aiAmount, bool abViolent = false) native

; Modifies this faction's reaction towards the other faction
Function ModReaction(Faction akOther, int aiAmount) native

; Has the player pay the crime gold for this faction
Function PlayerPayCrimeGold(bool abRemoveStolenItems = true, bool abGoToJail = true) native

; Finds a nearby NPC in this faction and has them behave as if assaulted
Function SendAssaultAlarm() native

; Sends the player to this faction's jail - removing inventory if requested, and to a "real" jail or not
Function SendPlayerToJail(bool abRemoveInventory = true, bool abRealJail = true) native

; Sets this faction and the other as allies or friends - if the friend booleans are true - the specified one-way relationship
; is a friend instead of an ally
Function SetAlly(Faction akOther, bool abSelfIsFriendToOther = false, bool abOtherIsFriendToSelf = false) native

; Sets the non-violent crime gold on this faction
Function SetCrimeGold(int aiGold) native

; Sets the violent crime gold on this faction
Function SetCrimeGoldViolent(int aiGold) native

; Sets this faction and the other as enemies or neutral - if the friend booleans are true - the specified one-way relationship
; is a neutral instead of an enemy
Function SetEnemy(Faction akOther, bool abSelfIsNeutralToOther = false, bool abOtherIsNeutralToSelf = false) native

; Sets or clears the player as an enemy of this faction
Function SetPlayerEnemy(bool abIsEnemy = true) native

; Sets or clears the expelled flag for this faction on the player
Function SetPlayerExpelled(bool abIsExpelled = true) native

; Sets this faction's reaction towards the other
Function SetReaction(Faction akOther, int aiNewValue) native

; SKSE64 additions built 2019-11-21 05:06:53.443000 UTC

int property kFaction_HiddenFromNPC				= 0x00000001 AutoReadOnly
int property kFaction_SpecialCombat				= 0x00000002 AutoReadOnly
int property kFaction_TrackCrime				= 0x00000010 AutoReadOnly
int property kFaction_IgnoreMurder				= 0x00000020 AutoReadOnly
int property kFaction_IgnoreAssault				= 0x00000040 AutoReadOnly
int property kFaction_IgnoreStealing			= 0x00000080 AutoReadOnly
int property kFaction_IgnoreTrespass			= 0x00000100 AutoReadOnly
int property kFaction_NoReportCrime				= 0x00000200 AutoReadOnly
int property kFaction_CrimeGoldDefaults			= 0x00000400 AutoReadOnly
int property kFaction_IgnorePickpocket			= 0x00000800 AutoReadOnly
int property kFaction_Vendor					= 0x00001000 AutoReadOnly
int property kFaction_CanBeOwner				= 0x00002000 AutoReadOnly
int property kFaction_IgnoreWerewolf			= 0x00004000 AutoReadOnly

; Not recommended unless the faction was previously a vendor
; due to the faction not having a package location the vendor
; may not be able to set up shop anywhere at all
Function MakeVendor()
	SetFactionFlag(self.kFaction_Vendor)
EndFunction

bool Function IsVendor()
	return IsFactionFlagSet(self.kFaction_Vendor)
EndFunction

Function ClearVendor()
	ClearFactionFlag(self.kFaction_Vendor)
EndFunction

bool Function IsFactionFlagSet(int flag) native
Function SetFactionFlag(int flag) native
Function ClearFactionFlag(int flag) native

bool Function OnlyBuysStolenItems() native
Function SetOnlyBuysStolenItems(bool onlyStolen) native

int Function GetVendorStartHour() native
Function SetVendorStartHour(int hour) native

int Function GetVendorEndHour() native
Function SetVendorEndHour(int hour) native

int Function GetVendorRadius() native
Function SetVendorRadius(int radius) native

ObjectReference Function GetMerchantContainer() native
Function SetMerchantContainer(ObjectReference akContainer) native

bool Function IsNotSellBuy() native
Function SetNotSellBuy(bool notSellBuy) native

FormList Function GetBuySellList() native
Function SetBuySellList(FormList akList) native