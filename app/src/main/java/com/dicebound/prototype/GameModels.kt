package com.dicebound.prototype

enum class DiceFace(val icon: String, val label: String) {
    BLADE("⚔", "Blade"),
    GUARD("◆", "Guard"),
    SPEED("➤", "Speed"),
    POWER("✦", "Power"),
    FOCUS("◉", "Focus"),
    WILD("★", "Wild")
}

enum class EffectType {
    DAMAGE,
    SHIELD,
    HEAL,
    ATTACK_UP,
    GUARD_UP,
    SPEED_UP,
    DRAIN,
    JAM_NEXT,
    ATTACK_DOWN,
    GUARD_DOWN
}

data class AbilityCard(
    val name: String,
    val requirements: Map<DiceFace, Int>,
    val effect: EffectType,
    val amount: Int,
    val text: String
)

data class FighterCard(
    val id: String,
    val name: String,
    val epithet: String,
    val maxHp: Int,
    val baseAttack: Int,
    val faction: String,
    val abilities: List<AbilityCard>
)

data class FighterState(
    val card: FighterCard,
    val hp: Int = card.maxHp
) {
    val alive: Boolean get() = hp > 0
}

data class LocationCard(
    val name: String,
    val description: String,
    val extraGuardPerFace: Int = 0,
    val extraSpeedPerFace: Int = 0,
    val extraPowerPerFace: Int = 0,
    val extraFocusPerFace: Int = 0
)

data class EventCard(
    val name: String,
    val description: String,
    val attackBonus: Int = 0,
    val guardBonus: Int = 0,
    val speedBonus: Int = 0,
    val powerBonus: Int = 0,
    val focusBonus: Int = 0,
    val startHeal: Int = 0
)

data class DieState(
    val face: DiceFace,
    val locked: Boolean = false,
    val spent: Boolean = false
)

data class CombatMods(
    val attack: Int = 0,
    val guard: Int = 0,
    val speed: Int = 0,
    val power: Int = 0
)

data class RolledStats(
    val attack: Int,
    val guard: Int,
    val speed: Int,
    val power: Int,
    val focus: Int
)

enum class BattlePhase {
    SETUP,
    PICK,
    ROLL,
    TACTICS,
    RESULT,
    GAME_OVER
}

enum class TacticAction(
    val title: String,
    val costFace: DiceFace,
    val description: String
) {
    POWER_SURGE("Power Surge", DiceFace.POWER, "Spend 2 Power: +4 Power this duel."),
    BRACE("Brace", DiceFace.GUARD, "Spend 2 Guard: +5 Guard this duel."),
    QUICKSTEP("Quickstep", DiceFace.SPEED, "Spend 2 Speed: +5 Speed this duel."),
    FATED_STRIKE("Fated Strike", DiceFace.FOCUS, "Spend 2 Focus: +3 Attack and +2 Power.")
}

data class GameState(
    val phase: BattlePhase = BattlePhase.SETUP,
    val round: Int = 1,
    val playerFirst: Boolean = true,
    val location: LocationCard? = null,
    val event: EventCard? = null,
    val playerRoster: List<FighterState> = GameData.playerRoster.map { FighterState(it) },
    val enemyRoster: List<FighterState> = GameData.enemyRoster.map { FighterState(it) },
    val setupSelection: Set<String> = emptySet(),
    val playerActiveIds: List<String> = emptyList(),
    val enemyActiveIds: List<String> = emptyList(),
    val playerSelectedId: String? = null,
    val enemySelectedId: String? = null,
    val playerDice: List<DieState> = emptyList(),
    val enemyDice: List<DieState> = emptyList(),
    val rerollsLeft: Int = 2,
    val playerMods: CombatMods = CombatMods(),
    val enemyMods: CombatMods = CombatMods(),
    val playerTactic: TacticAction? = null,
    val enemyTactic: TacticAction? = null,
    val log: List<String> = listOf("Choose three heroes for your opening line."),
    val winner: String? = null
)

private fun req(vararg pairs: Pair<DiceFace, Int>) = mapOf(*pairs)

object GameData {
    val playerRoster = listOf(
        FighterCard(
            "ironclad", "Ironclad", "Last Bastion", 22, 3, "Wardens",
            listOf(
                AbilityCard("Shield Wall", req(DiceFace.GUARD to 2), EffectType.SHIELD, 4, "Gain a strong all-damage shield."),
                AbilityCard("Crushing Advance", req(DiceFace.BLADE to 1, DiceFace.GUARD to 1), EffectType.ATTACK_UP, 2, "Build momentum into the final strike."),
                AbilityCard("Unbreakable", req(DiceFace.BLADE to 2, DiceFace.GUARD to 1), EffectType.GUARD_UP, 3, "Reinforce Guard for the duel.")
            )
        ),
        FighterCard(
            "mire_witch", "Mire Witch", "Keeper of Rot", 17, 2, "Wildborn",
            listOf(
                AbilityCard("Hex", req(DiceFace.POWER to 1, DiceFace.FOCUS to 1), EffectType.ATTACK_DOWN, 2, "Sap the enemy's Attack."),
                AbilityCard("Blackwater", req(DiceFace.POWER to 2), EffectType.DAMAGE, 4, "Deal direct spell damage."),
                AbilityCard("Crooked Fate", req(DiceFace.POWER to 1, DiceFace.FOCUS to 1, DiceFace.WILD to 1), EffectType.JAM_NEXT, 0, "Cancel the enemy's next triggered ability.")
            )
        ),
        FighterCard(
            "windrunner", "Windrunner", "Knife on the Gale", 16, 3, "Freeblades",
            listOf(
                AbilityCard("Slipstream", req(DiceFace.SPEED to 2), EffectType.SPEED_UP, 4, "Gain a burst of Speed."),
                AbilityCard("Passing Cut", req(DiceFace.BLADE to 1, DiceFace.SPEED to 1), EffectType.DAMAGE, 2, "Deal quick direct damage."),
                AbilityCard("Afterimage", req(DiceFace.SPEED to 3), EffectType.ATTACK_UP, 3, "Turn overwhelming Speed into Attack.")
            )
        ),
        FighterCard(
            "ember_adept", "Ember Adept", "Spark Unbound", 15, 2, "Cinders",
            listOf(
                AbilityCard("Kindle", req(DiceFace.POWER to 1, DiceFace.FOCUS to 1), EffectType.POWER_UP, 2, "Build Power for later abilities."),
                AbilityCard("Flare", req(DiceFace.POWER to 2), EffectType.DAMAGE, 4, "Launch a concentrated flame."),
                AbilityCard("Solar Lance", req(DiceFace.BLADE to 1, DiceFace.POWER to 2), EffectType.DAMAGE, 6, "A costly burst of direct damage.")
            )
        ),
        FighterCard(
            "grave_warden", "Grave Warden", "Lantern Bearer", 21, 2, "Wardens",
            listOf(
                AbilityCard("Pale Lantern", req(DiceFace.GUARD to 1, DiceFace.FOCUS to 1), EffectType.HEAL, 3, "Recover vitality."),
                AbilityCard("Stone Vigil", req(DiceFace.GUARD to 2), EffectType.SHIELD, 4, "Raise a spectral barrier."),
                AbilityCard("Grave Tithe", req(DiceFace.POWER to 1, DiceFace.FOCUS to 1), EffectType.DRAIN, 3, "Drain life from the opponent.")
            )
        ),
        FighterCard(
            "glass_seer", "Glass Seer", "Eye of Tomorrow", 15, 2, "Oracles",
            listOf(
                AbilityCard("Foreseen Blow", req(DiceFace.FOCUS to 2), EffectType.SHIELD, 3, "Prepare for incoming damage."),
                AbilityCard("Fracture", req(DiceFace.FOCUS to 1, DiceFace.POWER to 1), EffectType.JAM_NEXT, 0, "Jam the enemy's next ability."),
                AbilityCard("Perfect Line", req(DiceFace.FOCUS to 1, DiceFace.POWER to 1, DiceFace.WILD to 1), EffectType.DAMAGE, 5, "Strike along a predicted weakness.")
            )
        ),
        FighterCard(
            "thorn_duelist", "Thorn Duelist", "Roseblade", 18, 3, "Wildborn",
            listOf(
                AbilityCard("Briar Nick", req(DiceFace.BLADE to 1, DiceFace.SPEED to 1), EffectType.DAMAGE, 2, "Deal fast direct damage."),
                AbilityCard("Thorn Dance", req(DiceFace.BLADE to 2), EffectType.ATTACK_UP, 2, "Sharpen the final attack."),
                AbilityCard("Drink the Dew", req(DiceFace.FOCUS to 1, DiceFace.SPEED to 1), EffectType.DRAIN, 2, "Steal a little life.")
            )
        ),
        FighterCard(
            "storm_herald", "Storm Herald", "Voice Above", 17, 2, "Tempest",
            listOf(
                AbilityCard("Tailwind", req(DiceFace.SPEED to 1, DiceFace.POWER to 1), EffectType.SPEED_UP, 3, "Accelerate before abilities resolve."),
                AbilityCard("Thunderhead", req(DiceFace.POWER to 2), EffectType.DAMAGE, 3, "Deal spell damage."),
                AbilityCard("Skybreaker", req(DiceFace.BLADE to 1, DiceFace.SPEED to 1, DiceFace.POWER to 1), EffectType.DAMAGE, 5, "Explode through the enemy line.")
            )
        )
    )

    val enemyRoster = listOf(
        FighterCard(
            "ash_hound", "Ash Hound", "Coal-Fanged", 18, 3, "Cinders",
            listOf(
                AbilityCard("Scent Blood", req(DiceFace.FOCUS to 2), EffectType.ATTACK_UP, 2, "Hunt a weakness."),
                AbilityCard("Cinder Bite", req(DiceFace.BLADE to 1, DiceFace.POWER to 1), EffectType.DAMAGE, 3, "Burning bite."),
                AbilityCard("Pack Rush", req(DiceFace.BLADE to 2, DiceFace.SPEED to 1), EffectType.ATTACK_UP, 3, "Overwhelm with momentum.")
            )
        ),
        FighterCard(
            "bastion_sentinel", "Bastion Sentinel", "Gate of Brass", 23, 2, "Wardens",
            listOf(
                AbilityCard("Brace Plate", req(DiceFace.GUARD to 2), EffectType.SHIELD, 4, "Raise a plated shield."),
                AbilityCard("Repulse", req(DiceFace.GUARD to 1, DiceFace.POWER to 1), EffectType.GUARD_DOWN, 2, "Break the enemy's stance."),
                AbilityCard("Gatefall", req(DiceFace.BLADE to 1, DiceFace.GUARD to 2), EffectType.DAMAGE, 4, "Bring the full bastion down.")
            )
        ),
        FighterCard(
            "rift_scholar", "Rift Scholar", "Unlicensed Theorist", 15, 2, "Oracles",
            listOf(
                AbilityCard("Calculation", req(DiceFace.FOCUS to 2), EffectType.POWER_UP, 2, "Convert insight into Power."),
                AbilityCard("Fold Space", req(DiceFace.FOCUS to 1, DiceFace.SPEED to 1), EffectType.JAM_NEXT, 0, "Interrupt the next enemy ability."),
                AbilityCard("Rift Burn", req(DiceFace.POWER to 2, DiceFace.FOCUS to 1), EffectType.DAMAGE, 5, "Tear open a damaging rift.")
            )
        ),
        FighterCard(
            "blood_dancer", "Blood Dancer", "Red Step", 17, 3, "Freeblades",
            listOf(
                AbilityCard("Red Rhythm", req(DiceFace.BLADE to 1, DiceFace.SPEED to 1), EffectType.SPEED_UP, 2, "Accelerate the duel."),
                AbilityCard("Open Vein", req(DiceFace.BLADE to 2), EffectType.DAMAGE, 3, "Direct blade damage."),
                AbilityCard("Feast", req(DiceFace.BLADE to 1, DiceFace.FOCUS to 1, DiceFace.POWER to 1), EffectType.DRAIN, 4, "Drain vitality.")
            )
        ),
        FighterCard(
            "marsh_oracle", "Marsh Oracle", "Tongue of Reeds", 19, 2, "Wildborn",
            listOf(
                AbilityCard("Moss Skin", req(DiceFace.GUARD to 1, DiceFace.FOCUS to 1), EffectType.SHIELD, 3, "Grow a living shield."),
                AbilityCard("Bog Word", req(DiceFace.POWER to 1, DiceFace.FOCUS to 1), EffectType.ATTACK_DOWN, 2, "Weaken the foe."),
                AbilityCard("Sink", req(DiceFace.GUARD to 1, DiceFace.POWER to 2), EffectType.DAMAGE, 4, "Pull the opponent beneath the mire.")
            )
        ),
        FighterCard(
            "chainbreaker", "Chainbreaker", "No Master", 20, 3, "Freeblades",
            listOf(
                AbilityCard("Shoulder In", req(DiceFace.GUARD to 1, DiceFace.BLADE to 1), EffectType.ATTACK_UP, 2, "Turn defense into offense."),
                AbilityCard("Snap Guard", req(DiceFace.POWER to 1, DiceFace.BLADE to 1), EffectType.GUARD_DOWN, 3, "Crack enemy Guard."),
                AbilityCard("No Chains", req(DiceFace.BLADE to 3), EffectType.DAMAGE, 5, "Heavy direct hit.")
            )
        ),
        FighterCard(
            "moonblade", "Moonblade", "Silver Quiet", 16, 3, "Oracles",
            listOf(
                AbilityCard("Still Mind", req(DiceFace.FOCUS to 2), EffectType.SHIELD, 2, "Read the attack before it lands."),
                AbilityCard("Crescent", req(DiceFace.BLADE to 1, DiceFace.FOCUS to 1), EffectType.DAMAGE, 3, "Precise cut."),
                AbilityCard("Eclipse", req(DiceFace.BLADE to 1, DiceFace.FOCUS to 2), EffectType.JAM_NEXT, 0, "Blank the next enemy ability.")
            )
        ),
        FighterCard(
            "tempest_idol", "Tempest Idol", "Walking Storm", 18, 2, "Tempest",
            listOf(
                AbilityCard("Charge", req(DiceFace.SPEED to 1, DiceFace.POWER to 1), EffectType.POWER_UP, 2, "Store a storm charge."),
                AbilityCard("Forked Bolt", req(DiceFace.POWER to 2), EffectType.DAMAGE, 4, "Blast the foe."),
                AbilityCard("Storm Crown", req(DiceFace.SPEED to 2, DiceFace.POWER to 1), EffectType.DAMAGE, 5, "Call down a focused storm.")
            )
        )
    )

    val locations = listOf(
        LocationCard("Ember Vault", "Each Power face grants +1 additional Power.", extraPowerPerFace = 1),
        LocationCard("Sunken Bastion", "Each Guard face grants +1 additional Guard.", extraGuardPerFace = 1),
        LocationCard("Windglass Spires", "Each Speed face grants +1 additional Speed.", extraSpeedPerFace = 1),
        LocationCard("Moon Garden", "Each Focus face grants +1 additional Focus.", extraFocusPerFace = 1),
        LocationCard("Old Crossroads", "No location modifier. Pure fundamentals.")
    )

    val events = listOf(
        EventCard("Arcane Bloom", "Both fighters gain +1 Power this round.", powerBonus = 1),
        EventCard("Iron Rain", "Both fighters gain +1 Guard this round.", guardBonus = 1),
        EventCard("Blood Moon", "Both fighters gain +1 Attack this round.", attackBonus = 1),
        EventCard("Tailwind", "Both fighters gain +1 Speed this round.", speedBonus = 1),
        EventCard("Clear Mind", "Both fighters gain +1 Focus this round.", focusBonus = 1),
        EventCard("Second Wind", "Both chosen fighters heal 1 before abilities.", startHeal = 1)
    )
}
