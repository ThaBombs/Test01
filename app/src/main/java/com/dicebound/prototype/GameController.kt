package com.dicebound.prototype

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import kotlin.math.max
import kotlin.random.Random

class GameController(
    private val random: Random = Random.Default
) {
    var state by mutableStateOf(GameState())
        private set

    fun toggleSetupHero(id: String) {
        if (state.phase != BattlePhase.SETUP) return
        val selection = state.setupSelection.toMutableSet()
        if (id in selection) {
            selection.remove(id)
        } else if (selection.size < 3) {
            selection.add(id)
        }
        state = state.copy(setupSelection = selection)
    }

    fun startGame() {
        if (state.setupSelection.size != 3) return
        val enemyOpening = GameData.enemyRoster.shuffled(random).take(3).map { it.id }
        val location = GameData.locations.random(random)
        val opening = state.copy(
            phase = BattlePhase.PICK,
            round = 1,
            playerFirst = true,
            location = location,
            event = GameData.events.random(random),
            playerActiveIds = state.setupSelection.toList(),
            enemyActiveIds = enemyOpening,
            log = listOf(
                "Battle begins at ${location.name}.",
                "Round 1: you have first pick."
            )
        )
        state = opening
    }

    fun choosePlayerFighter(id: String) {
        if (state.phase != BattlePhase.PICK) return
        if (id !in state.playerActiveIds) return
        val fighter = fighter(state.playerRoster, id) ?: return
        if (!fighter.alive) return

        var next = state.copy(playerSelectedId = id)
        if (state.playerFirst) {
            val enemyId = chooseEnemyFighter(id, next)
            next = next.copy(enemySelectedId = enemyId)
        }
        if (next.enemySelectedId != null) {
            val pName = fighter(next.playerRoster, id)?.card?.name ?: "Hero"
            val eName = fighter(next.enemyRoster, next.enemySelectedId)?.card?.name ?: "Enemy"
            next = next.copy(
                phase = BattlePhase.ROLL,
                log = next.log + "Matchup: $pName vs $eName."
            )
        }
        state = next
    }

    fun rollInitial() {
        if (state.phase != BattlePhase.ROLL || state.playerDice.isNotEmpty()) return
        val playerDice = List(6) { DieState(randomFace()) }
        var enemyDice = List(6) { DieState(randomFace()) }
        val enemyCard = selectedEnemy()?.card
        if (enemyCard != null) {
            repeat(2) { enemyDice = aiReroll(enemyDice, enemyCard) }
        }
        state = state.copy(
            playerDice = playerDice,
            enemyDice = enemyDice,
            rerollsLeft = 2,
            log = state.log + "Dice rolled. Tap your dice to lock the results you want to keep."
        )
    }

    fun toggleDieLock(index: Int) {
        if (state.phase != BattlePhase.ROLL || index !in state.playerDice.indices) return
        val die = state.playerDice[index]
        if (die.spent) return
        state = state.copy(
            playerDice = state.playerDice.mapIndexed { i, d ->
                if (i == index) d.copy(locked = !d.locked) else d
            }
        )
    }

    fun rerollUnlocked() {
        if (state.phase != BattlePhase.ROLL || state.playerDice.isEmpty() || state.rerollsLeft <= 0) return
        val rerolled = state.playerDice.map { die ->
            if (die.locked || die.spent) die else DieState(randomFace())
        }
        state = state.copy(
            playerDice = rerolled,
            rerollsLeft = state.rerollsLeft - 1,
            log = state.log + "Reroll used. ${state.rerollsLeft - 1} reroll(s) remain."
        )
    }

    fun finishRolling() {
        if (state.phase != BattlePhase.ROLL || state.playerDice.isEmpty()) return
        var next = state.copy(phase = BattlePhase.TACTICS)
        next = applyAiTactic(next)
        state = next.copy(log = next.log + "Tactical window: you may spend two matching dice for one special action.")
    }

    fun usePlayerTactic(action: TacticAction) {
        if (state.phase != BattlePhase.TACTICS || state.playerTactic != null) return
        val indices = state.playerDice.indices
            .filter { !state.playerDice[it].spent && state.playerDice[it].face == action.costFace }
            .take(2)
        if (indices.size < 2) return

        val spentDice = state.playerDice.mapIndexed { index, die ->
            if (index in indices) die.copy(spent = true, locked = true) else die
        }
        state = state.copy(
            playerDice = spentDice,
            playerMods = applyTacticMods(state.playerMods, action),
            playerTactic = action,
            log = state.log + "You use ${action.title}."
        )
    }

    fun canUseTactic(action: TacticAction): Boolean {
        if (state.playerTactic != null) return false
        return state.playerDice.count { !it.spent && it.face == action.costFace } >= 2
    }

    fun resolveRound() {
        if (state.phase != BattlePhase.TACTICS) return
        val player = selectedPlayer() ?: return
        val enemy = selectedEnemy() ?: return

        val playerStats = previewStats(true)
        val enemyStats = previewStats(false)

        val p = Sim(
            side = "You",
            card = player.card,
            hp = healedStartHp(player),
            attack = player.card.baseAttack + playerStats.attack + playerStats.focus / 2,
            guard = playerStats.guard,
            speed = playerStats.speed,
            power = playerStats.power,
            focus = playerStats.focus
        )
        val e = Sim(
            side = "Enemy",
            card = enemy.card,
            hp = healedStartHp(enemy),
            attack = enemy.card.baseAttack + enemyStats.attack + enemyStats.focus / 2,
            guard = enemyStats.guard,
            speed = enemyStats.speed,
            power = enemyStats.power,
            focus = enemyStats.focus
        )

        val lines = mutableListOf<String>()
        if ((state.event?.startHeal ?: 0) > 0) {
            lines += "${state.event?.name}: both fighters recover ${state.event?.startHeal} HP."
        }

        val playerActsFirst = when {
            p.speed > e.speed -> true
            e.speed > p.speed -> false
            else -> state.playerFirst
        }
        lines += if (playerActsFirst) {
            "${p.card.name} has initiative (${p.speed} vs ${e.speed} Speed)."
        } else {
            "${e.card.name} has initiative (${e.speed} vs ${p.speed} Speed)."
        }

        for (slot in 0..2) {
            if (playerActsFirst) {
                runAbility(p, e, p.card.abilities.getOrNull(slot), state.playerDice, lines)
                if (e.hp > 0) runAbility(e, p, e.card.abilities.getOrNull(slot), state.enemyDice, lines)
            } else {
                runAbility(e, p, e.card.abilities.getOrNull(slot), state.enemyDice, lines)
                if (p.hp > 0) runAbility(p, e, p.card.abilities.getOrNull(slot), state.playerDice, lines)
            }
        }

        if (p.hp > 0 && e.hp > 0) {
            if (playerActsFirst) {
                basicAttack(p, e, lines)
                if (e.hp > 0) basicAttack(e, p, lines)
            } else {
                basicAttack(e, p, lines)
                if (p.hp > 0) basicAttack(p, e, lines)
            }
        }

        var playerRoster = updateHp(state.playerRoster, p.card.id, p.hp)
        var enemyRoster = updateHp(state.enemyRoster, e.card.id, e.hp)

        if (p.hp <= 0) lines += "${p.card.name} is defeated."
        if (e.hp <= 0) lines += "${e.card.name} is defeated."

        val playerPromoted = replenish(state.playerActiveIds, playerRoster)
        val enemyPromoted = replenish(state.enemyActiveIds, enemyRoster)

        if (playerPromoted != state.playerActiveIds) {
            val added = playerPromoted.filterNot { it in state.playerActiveIds }
            added.forEach { id -> fighter(playerRoster, id)?.let { lines += "${it.card.name} is promoted from reserve." } }
        }
        if (enemyPromoted != state.enemyActiveIds) {
            val added = enemyPromoted.filterNot { it in state.enemyActiveIds }
            added.forEach { id -> fighter(enemyRoster, id)?.let { lines += "Enemy promotes ${it.card.name} from reserve." } }
        }

        val playerAlive = playerRoster.any { it.alive }
        val enemyAlive = enemyRoster.any { it.alive }
        val winner = when {
            playerAlive && !enemyAlive -> "Victory"
            !playerAlive && enemyAlive -> "Defeat"
            !playerAlive && !enemyAlive -> "Draw"
            else -> null
        }

        state = state.copy(
            playerRoster = playerRoster,
            enemyRoster = enemyRoster,
            playerActiveIds = playerPromoted,
            enemyActiveIds = enemyPromoted,
            phase = if (winner == null) BattlePhase.RESULT else BattlePhase.GAME_OVER,
            winner = winner,
            log = (state.log + lines).takeLast(30)
        )
    }

    fun nextRound() {
        if (state.phase != BattlePhase.RESULT) return
        val first = !state.playerFirst
        var next = state.copy(
            round = state.round + 1,
            playerFirst = first,
            event = GameData.events.random(random),
            phase = BattlePhase.PICK,
            playerSelectedId = null,
            enemySelectedId = null,
            playerDice = emptyList(),
            enemyDice = emptyList(),
            rerollsLeft = 2,
            playerMods = CombatMods(),
            enemyMods = CombatMods(),
            playerTactic = null,
            enemyTactic = null,
            log = state.log + if (first) {
                "Round ${state.round + 1}: you have first pick."
            } else {
                "Round ${state.round + 1}: enemy has first pick."
            }
        )
        if (!first) {
            val enemyId = chooseEnemyFighter(null, next)
            next = next.copy(
                enemySelectedId = enemyId,
                log = next.log + "Enemy commits ${fighter(next.enemyRoster, enemyId)?.card?.name ?: "a fighter"}."
            )
        }
        state = next
    }

    fun restart() {
        state = GameState()
    }

    fun selectedPlayer(): FighterState? = fighter(state.playerRoster, state.playerSelectedId)
    fun selectedEnemy(): FighterState? = fighter(state.enemyRoster, state.enemySelectedId)

    fun playerActive(): List<FighterState> =
        state.playerActiveIds.mapNotNull { fighter(state.playerRoster, it) }.filter { it.alive }

    fun enemyActive(): List<FighterState> =
        state.enemyActiveIds.mapNotNull { fighter(state.enemyRoster, it) }.filter { it.alive }

    fun reserves(player: Boolean): List<FighterState> {
        val roster = if (player) state.playerRoster else state.enemyRoster
        val active = if (player) state.playerActiveIds else state.enemyActiveIds
        return roster.filter { it.alive && it.card.id !in active }
    }

    fun previewStats(player: Boolean): RolledStats {
        val dice = if (player) state.playerDice else state.enemyDice
        val mods = if (player) state.playerMods else state.enemyMods
        val location = state.location
        val event = state.event

        val blade = dice.count { !it.spent && it.face == DiceFace.BLADE }
        val guardFaces = dice.count { !it.spent && it.face == DiceFace.GUARD }
        val speedFaces = dice.count { !it.spent && it.face == DiceFace.SPEED }
        val powerFaces = dice.count { !it.spent && it.face == DiceFace.POWER }
        val focusFaces = dice.count { !it.spent && it.face == DiceFace.FOCUS }

        return RolledStats(
            attack = blade + (event?.attackBonus ?: 0) + mods.attack,
            guard = guardFaces * (1 + (location?.extraGuardPerFace ?: 0)) + (event?.guardBonus ?: 0) + mods.guard,
            speed = speedFaces * (1 + (location?.extraSpeedPerFace ?: 0)) + (event?.speedBonus ?: 0) + mods.speed,
            power = powerFaces * (1 + (location?.extraPowerPerFace ?: 0)) + (event?.powerBonus ?: 0) + mods.power,
            focus = focusFaces * (1 + (location?.extraFocusPerFace ?: 0)) + (event?.focusBonus ?: 0)
        )
    }

    fun abilityTriggered(ability: AbilityCard, dice: List<DieState>): Boolean {
        val available = dice.filter { !it.spent }
        var wilds = available.count { it.face == DiceFace.WILD }
        for ((face, needed) in ability.requirements) {
            if (face == DiceFace.WILD) {
                if (wilds < needed) return false
                wilds -= needed
                continue
            }
            val actual = available.count { it.face == face }
            val missing = max(0, needed - actual)
            if (missing > wilds) return false
            wilds -= missing
        }
        return true
    }

    private fun randomFace(): DiceFace = DiceFace.entries.random(random)

    private fun fighter(roster: List<FighterState>, id: String?): FighterState? =
        roster.firstOrNull { it.card.id == id }

    private fun chooseEnemyFighter(playerId: String?, snapshot: GameState): String {
        val candidates = snapshot.enemyActiveIds
            .mapNotNull { fighter(snapshot.enemyRoster, it) }
            .filter { it.alive }
        if (candidates.isEmpty()) return ""
        val playerCard = fighter(snapshot.playerRoster, playerId)?.card
        return candidates.maxByOrNull { enemy ->
            var score = enemy.hp + enemy.card.baseAttack * 2
            if (playerCard != null) {
                if (enemy.card.maxHp > playerCard.maxHp) score += 2
                if (enemy.card.baseAttack >= playerCard.baseAttack) score += 2
            }
            score + random.nextInt(0, 4)
        }?.card?.id ?: candidates.first().card.id
    }

    private fun aiReroll(dice: List<DieState>, card: FighterCard): List<DieState> {
        val target = card.abilities.maxByOrNull { ability ->
            closenessScore(ability, dice) * 4 + ability.amount
        } ?: return dice

        val needed = target.requirements.toMutableMap()
        var wildNeeded = needed.remove(DiceFace.WILD) ?: 0
        val keep = BooleanArray(dice.size)

        dice.forEachIndexed { index, die ->
            val count = needed[die.face] ?: 0
            if (die.face != DiceFace.WILD && count > 0) {
                keep[index] = true
                needed[die.face] = count - 1
            }
        }
        dice.forEachIndexed { index, die ->
            if (!keep[index] && die.face == DiceFace.WILD) {
                val remaining = needed.values.sum() + wildNeeded
                if (remaining > 0) {
                    keep[index] = true
                    if (wildNeeded > 0) wildNeeded-- else {
                        val key = needed.entries.firstOrNull { it.value > 0 }?.key
                        if (key != null) needed[key] = (needed[key] ?: 0) - 1
                    }
                }
            }
        }
        return dice.mapIndexed { index, die ->
            if (keep[index]) die.copy(locked = true) else DieState(randomFace())
        }
    }

    private fun closenessScore(ability: AbilityCard, dice: List<DieState>): Int {
        val faces = dice.groupingBy { it.face }.eachCount()
        var matched = 0
        var wilds = faces[DiceFace.WILD] ?: 0
        ability.requirements.forEach { (face, count) ->
            if (face == DiceFace.WILD) {
                val use = minOf(count, wilds)
                matched += use
                wilds -= use
            } else {
                val actual = minOf(count, faces[face] ?: 0)
                matched += actual
            }
        }
        return matched
    }

    private fun applyAiTactic(snapshot: GameState): GameState {
        if (snapshot.enemyTactic != null) return snapshot
        val hp = fighter(snapshot.enemyRoster, snapshot.enemySelectedId)?.hp ?: 99
        val order = if (hp <= 8) {
            listOf(TacticAction.BRACE, TacticAction.POWER_SURGE, TacticAction.QUICKSTEP, TacticAction.FATED_STRIKE)
        } else {
            listOf(TacticAction.POWER_SURGE, TacticAction.FATED_STRIKE, TacticAction.QUICKSTEP, TacticAction.BRACE)
        }
        val chosen = order.firstOrNull { action ->
            snapshot.enemyDice.count { !it.spent && it.face == action.costFace } >= 2
        } ?: return snapshot

        val indices = snapshot.enemyDice.indices
            .filter { !snapshot.enemyDice[it].spent && snapshot.enemyDice[it].face == chosen.costFace }
            .take(2)
        val dice = snapshot.enemyDice.mapIndexed { index, die ->
            if (index in indices) die.copy(spent = true, locked = true) else die
        }
        return snapshot.copy(
            enemyDice = dice,
            enemyMods = applyTacticMods(snapshot.enemyMods, chosen),
            enemyTactic = chosen,
            log = snapshot.log + "Enemy spends dice on ${chosen.title}."
        )
    }

    private fun applyTacticMods(mods: CombatMods, action: TacticAction): CombatMods = when (action) {
        TacticAction.POWER_SURGE -> mods.copy(power = mods.power + 4)
        TacticAction.BRACE -> mods.copy(guard = mods.guard + 5)
        TacticAction.QUICKSTEP -> mods.copy(speed = mods.speed + 5)
        TacticAction.FATED_STRIKE -> mods.copy(attack = mods.attack + 3, power = mods.power + 2)
    }

    private fun healedStartHp(fighter: FighterState): Int {
        val heal = state.event?.startHeal ?: 0
        return minOf(fighter.card.maxHp, fighter.hp + heal)
    }

    private fun runAbility(
        source: Sim,
        target: Sim,
        ability: AbilityCard?,
        dice: List<DieState>,
        lines: MutableList<String>
    ) {
        if (ability == null || source.hp <= 0) return
        if (!abilityTriggered(ability, dice)) return
        if (source.jammedNext) {
            source.jammedNext = false
            lines += "${source.card.name}'s ${ability.name} is jammed."
            return
        }

        val powerScale = source.power / 2
        val focusScale = source.focus / 2
        when (ability.effect) {
            EffectType.DAMAGE -> {
                val amount = ability.amount + powerScale
                val dealt = dealDamage(target, amount)
                lines += "${source.card.name} uses ${ability.name}: $dealt damage."
            }
            EffectType.SHIELD -> {
                val amount = ability.amount + powerScale
                source.shield += amount
                lines += "${source.card.name} uses ${ability.name}: +$amount shield."
            }
            EffectType.HEAL -> {
                val amount = ability.amount + focusScale
                val before = source.hp
                source.hp = minOf(source.card.maxHp, source.hp + amount)
                lines += "${source.card.name} uses ${ability.name}: +${source.hp - before} HP."
            }
            EffectType.ATTACK_UP -> {
                val amount = ability.amount + powerScale / 2
                source.attack += amount
                lines += "${source.card.name} uses ${ability.name}: +$amount Attack."
            }
            EffectType.GUARD_UP -> {
                val amount = ability.amount + powerScale / 2
                source.guard += amount
                lines += "${source.card.name} uses ${ability.name}: +$amount Guard."
            }
            EffectType.SPEED_UP -> {
                source.speed += ability.amount
                lines += "${source.card.name} uses ${ability.name}: +${ability.amount} Speed."
            }
            EffectType.POWER_UP -> {
                source.power += ability.amount
                lines += "${source.card.name} uses ${ability.name}: +${ability.amount} Power."
            }
            EffectType.DRAIN -> {
                val amount = ability.amount + powerScale
                val dealt = dealDamage(target, amount)
                val healed = (dealt + 1) / 2
                val before = source.hp
                source.hp = minOf(source.card.maxHp, source.hp + healed)
                lines += "${source.card.name} uses ${ability.name}: $dealt drain damage, +${source.hp - before} HP."
            }
            EffectType.JAM_NEXT -> {
                target.jammedNext = true
                lines += "${source.card.name} uses ${ability.name}: next enemy ability is jammed."
            }
            EffectType.ATTACK_DOWN -> {
                val amount = ability.amount + powerScale / 2
                target.attack = max(0, target.attack - amount)
                lines += "${source.card.name} uses ${ability.name}: enemy Attack -$amount."
            }
            EffectType.GUARD_DOWN -> {
                val amount = ability.amount + powerScale / 2
                target.guard = max(0, target.guard - amount)
                lines += "${source.card.name} uses ${ability.name}: enemy Guard -$amount."
            }
        }
    }

    private fun basicAttack(source: Sim, target: Sim, lines: MutableList<String>) {
        val raw = max(0, source.attack - target.guard)
        val dealt = dealDamage(target, raw)
        lines += "${source.card.name} attacks (${source.attack} ATK vs ${target.guard} Guard): $dealt damage."
    }

    private fun dealDamage(target: Sim, amount: Int): Int {
        if (amount <= 0) return 0
        var remaining = amount
        if (target.shield > 0) {
            val absorbed = minOf(target.shield, remaining)
            target.shield -= absorbed
            remaining -= absorbed
        }
        val before = target.hp
        target.hp = max(0, target.hp - remaining)
        return before - target.hp
    }

    private fun updateHp(roster: List<FighterState>, id: String, hp: Int): List<FighterState> =
        roster.map { if (it.card.id == id) it.copy(hp = hp) else it }

    private fun replenish(active: List<String>, roster: List<FighterState>): List<String> {
        val aliveActive = active.filter { id -> fighter(roster, id)?.alive == true }.toMutableList()
        val reserves = roster.filter { it.alive && it.card.id !in aliveActive }
        for (reserve in reserves) {
            if (aliveActive.size >= 3) break
            aliveActive += reserve.card.id
        }
        return aliveActive
    }

    private data class Sim(
        val side: String,
        val card: FighterCard,
        var hp: Int,
        var attack: Int,
        var guard: Int,
        var speed: Int,
        var power: Int,
        var focus: Int,
        var shield: Int = 0,
        var jammedNext: Boolean = false
    )
}
