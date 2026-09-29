package com.dicebound.prototype

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Divider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

private val Ink = Color(0xFF0D1117)
private val Panel = Color(0xFF171D25)
private val PanelLift = Color(0xFF202938)
private val Ivory = Color(0xFFE8E2D6)
private val Gold = Color(0xFFE5B85C)
private val Muted = Color(0xFFA7B0BE)
private val Danger = Color(0xFFE57373)
private val Success = Color(0xFF81C784)

private val DiceboundColors = darkColorScheme(
    primary = Gold,
    onPrimary = Color(0xFF251A05),
    secondary = Color(0xFF8FB8D8),
    background = Ink,
    surface = Panel,
    surfaceVariant = PanelLift,
    onBackground = Ivory,
    onSurface = Ivory
)

@Composable
fun DiceboundApp() {
    val controller = remember { GameController() }
    MaterialTheme(colorScheme = DiceboundColors) {
        Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background) {
            when (controller.state.phase) {
                BattlePhase.SETUP -> SetupScreen(controller)
                else -> BattleScreen(controller)
            }
        }
    }
}

@Composable
private fun SetupScreen(controller: GameController) {
    val state = controller.state
    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text("DICEBOUND", fontSize = 30.sp, fontWeight = FontWeight.Black, color = Gold)
        Text("Combat prototype • offline AI match", color = Muted, fontSize = 14.sp)

        Card(colors = CardDefaults.cardColors(containerColor = PanelLift)) {
            Column(Modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(7.dp)) {
                Text("THE CORE", fontWeight = FontWeight.Bold, color = Gold)
                Text(
                    "Choose 3 opening heroes from an 8-character squad. Each round one side commits first, the other counters, then both roll six dice.",
                    fontSize = 14.sp
                )
                Text(
                    "Dice are never dead: ⚔ Attack • ◆ Guard • ➤ Speed • ✦ Power • ◉ Focus • ★ Wild.",
                    fontSize = 14.sp,
                    color = Muted
                )
                Text(
                    "Your dice also trigger hero abilities. Before resolution you may spend two matching dice on one universal tactical action.",
                    fontSize = 14.sp,
                    color = Muted
                )
            }
        }

        Text(
            "OPENING LINE  ${state.setupSelection.size}/3",
            fontWeight = FontWeight.Bold,
            color = Gold
        )

        state.playerRoster.forEach { fighter ->
            SetupHeroCard(
                fighter = fighter,
                selected = fighter.card.id in state.setupSelection,
                onClick = { controller.toggleSetupHero(fighter.card.id) }
            )
        }

        Button(
            onClick = { controller.startGame() },
            enabled = state.setupSelection.size == 3,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text(if (state.setupSelection.size == 3) "BEGIN MATCH" else "CHOOSE 3 HEROES")
        }

        Spacer(Modifier.height(20.dp))
    }
}

@Composable
private fun SetupHeroCard(
    fighter: FighterState,
    selected: Boolean,
    onClick: () -> Unit
) {
    val border = if (selected) BorderStroke(2.dp, Gold) else BorderStroke(1.dp, Color(0xFF333C49))
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        border = border,
        colors = CardDefaults.cardColors(containerColor = if (selected) Color(0xFF2A261D) else Panel)
    ) {
        Column(Modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Column(Modifier.weight(1f)) {
                    Text(fighter.card.name, fontWeight = FontWeight.Bold, fontSize = 18.sp)
                    Text(fighter.card.epithet, color = Muted, fontSize = 12.sp)
                }
                Text("♥ ${fighter.card.maxHp}", color = Danger, fontWeight = FontWeight.Bold)
                Text("   ⚔ ${fighter.card.baseAttack}", color = Gold, fontWeight = FontWeight.Bold)
            }
            Text(fighter.card.faction.uppercase(), color = factionColor(fighter.card.faction), fontSize = 11.sp)
            fighter.card.abilities.forEachIndexed { index, ability ->
                Text(
                    "${index + 1}. ${abilityRequirement(ability)}  ${ability.name} — ${ability.text}",
                    fontSize = 12.sp,
                    color = if (index == 2) Ivory else Muted
                )
            }
        }
    }
}

@Composable
private fun BattleScreen(controller: GameController) {
    val state = controller.state
    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(12.dp),
        verticalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Column(Modifier.weight(1f)) {
                Text("DICEBOUND", fontSize = 24.sp, fontWeight = FontWeight.Black, color = Gold)
                Text("Round ${state.round}", color = Muted)
            }
            Text(
                if (state.playerFirst) "YOU COMMIT FIRST" else "ENEMY COMMITS FIRST",
                color = if (state.playerFirst) Success else Danger,
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold
            )
        }

        WorldCards(state)

        SectionTitle("ENEMY ACTIVE")
        ActiveRow(
            fighters = controller.enemyActive(),
            selectedId = state.enemySelectedId,
            selectable = false,
            onSelect = {}
        )
        ReserveLine("Enemy reserve", controller.reserves(false))

        DuelPanel(controller)

        Divider(color = Color(0xFF303846))

        SectionTitle("YOUR ACTIVE")
        ActiveRow(
            fighters = controller.playerActive(),
            selectedId = state.playerSelectedId,
            selectable = state.phase == BattlePhase.PICK,
            onSelect = { controller.choosePlayerFighter(it) }
        )
        ReserveLine("Your reserve", controller.reserves(true))

        if (state.phase == BattlePhase.PICK) {
            Card(colors = CardDefaults.cardColors(containerColor = PanelLift)) {
                Text(
                    if (state.playerFirst) {
                        "Choose a fighter. The enemy will respond after you commit."
                    } else {
                        "The enemy has committed ${controller.selectedEnemy()?.card?.name ?: "a fighter"}. Choose your response."
                    },
                    modifier = Modifier.padding(14.dp),
                    color = Ivory,
                    fontSize = 14.sp
                )
            }
        }

        if (state.phase == BattlePhase.ROLL || state.phase == BattlePhase.TACTICS || state.phase == BattlePhase.RESULT) {
            DiceAndAbilities(controller)
        }

        if (state.phase == BattlePhase.TACTICS) {
            TacticsPanel(controller)
            Button(onClick = { controller.resolveRound() }, modifier = Modifier.fillMaxWidth()) {
                Text("RESOLVE DUEL")
            }
        }

        if (state.phase == BattlePhase.RESULT) {
            Button(onClick = { controller.nextRound() }, modifier = Modifier.fillMaxWidth()) {
                Text("NEXT ROUND")
            }
        }

        if (state.phase == BattlePhase.GAME_OVER) {
            GameOverPanel(controller)
        }

        CombatLog(state.log)
        Spacer(Modifier.height(22.dp))
    }
}

@Composable
private fun WorldCards(state: GameState) {
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
        Card(
            modifier = Modifier.weight(1f),
            colors = CardDefaults.cardColors(containerColor = Color(0xFF1C2B32))
        ) {
            Column(Modifier.padding(10.dp)) {
                Text("LOCATION", fontSize = 10.sp, color = Muted, fontWeight = FontWeight.Bold)
                Text(state.location?.name ?: "—", fontWeight = FontWeight.Bold, color = Color(0xFF9FD6DE))
                Text(state.location?.description ?: "", fontSize = 11.sp, color = Muted)
            }
        }
        Card(
            modifier = Modifier.weight(1f),
            colors = CardDefaults.cardColors(containerColor = Color(0xFF30251D))
        ) {
            Column(Modifier.padding(10.dp)) {
                Text("ROUND EVENT", fontSize = 10.sp, color = Muted, fontWeight = FontWeight.Bold)
                Text(state.event?.name ?: "—", fontWeight = FontWeight.Bold, color = Gold)
                Text(state.event?.description ?: "", fontSize = 11.sp, color = Muted)
            }
        }
    }
}

@Composable
private fun DuelPanel(controller: GameController) {
    val player = controller.selectedPlayer()
    val enemy = controller.selectedEnemy()
    if (player == null && enemy == null) return

    Card(colors = CardDefaults.cardColors(containerColor = Color(0xFF141A22))) {
        Column(
            Modifier.padding(12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Text("DUEL", color = Gold, fontSize = 11.sp, fontWeight = FontWeight.Bold)
            Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                Text(
                    player?.card?.name ?: "Choose...",
                    modifier = Modifier.weight(1f),
                    textAlign = TextAlign.End,
                    fontWeight = FontWeight.Bold
                )
                Text("   VS   ", color = Muted, fontWeight = FontWeight.Black)
                Text(
                    enemy?.card?.name ?: "Waiting...",
                    modifier = Modifier.weight(1f),
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

@Composable
private fun DiceAndAbilities(controller: GameController) {
    val state = controller.state
    val player = controller.selectedPlayer() ?: return
    val enemy = controller.selectedEnemy() ?: return

    SectionTitle("DICE")

    if (state.playerDice.isEmpty()) {
        Button(onClick = { controller.rollInitial() }, modifier = Modifier.fillMaxWidth()) {
            Text("ROLL 6 DICE")
        }
        Text("You get two rerolls. Tap results to lock or unlock them.", color = Muted, fontSize = 12.sp)
        return
    }

    Text("Your roll", fontWeight = FontWeight.Bold)
    DiceRow(
        dice = state.playerDice,
        interactive = state.phase == BattlePhase.ROLL,
        onDieClick = { controller.toggleDieLock(it) }
    )
    StatsLine(controller.previewStats(true))

    if (state.phase == BattlePhase.ROLL) {
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(
                onClick = { controller.rerollUnlocked() },
                enabled = state.rerollsLeft > 0,
                modifier = Modifier.weight(1f)
            ) {
                Text("REROLL (${state.rerollsLeft})")
            }
            Button(onClick = { controller.finishRolling() }, modifier = Modifier.weight(1f)) {
                Text("KEEP ROLL")
            }
        }
    }

    AbilityPanel(player.card, state.playerDice, controller)

    Spacer(Modifier.height(4.dp))
    Text("Enemy roll", fontWeight = FontWeight.Bold)
    DiceRow(dice = state.enemyDice, interactive = false, onDieClick = {})
    StatsLine(controller.previewStats(false))
    if (state.enemyTactic != null) {
        Text(
            "Enemy tactic: ${state.enemyTactic.title}",
            color = Danger,
            fontSize = 12.sp,
            fontWeight = FontWeight.Bold
        )
    }
    AbilityPanel(enemy.card, state.enemyDice, controller)
}

@Composable
private fun DiceRow(
    dice: List<DieState>,
    interactive: Boolean,
    onDieClick: (Int) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth().horizontalScroll(rememberScrollState()),
        horizontalArrangement = Arrangement.spacedBy(6.dp)
    ) {
        dice.forEachIndexed { index, die ->
            val borderColor = when {
                die.spent -> Danger
                die.locked -> Gold
                else -> Color(0xFF445064)
            }
            Box(
                modifier = Modifier
                    .size(48.dp)
                    .background(if (die.spent) Color(0xFF2B1717) else PanelLift, RoundedCornerShape(9.dp))
                    .border(2.dp, borderColor, RoundedCornerShape(9.dp))
                    .clickable(enabled = interactive && !die.spent) { onDieClick(index) },
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(die.face.icon, fontSize = 21.sp, color = faceColor(die.face))
                    if (die.spent) {
                        Text("SPENT", fontSize = 7.sp, color = Danger, fontWeight = FontWeight.Bold)
                    } else if (die.locked) {
                        Text("LOCK", fontSize = 7.sp, color = Gold, fontWeight = FontWeight.Bold)
                    }
                }
            }
        }
    }
}

@Composable
private fun StatsLine(stats: RolledStats) {
    Text(
        "⚔ ${stats.attack}   ◆ ${stats.guard}   ➤ ${stats.speed}   ✦ ${stats.power}   ◉ ${stats.focus}",
        color = Muted,
        fontSize = 12.sp
    )
}

@Composable
private fun AbilityPanel(card: FighterCard, dice: List<DieState>, controller: GameController) {
    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
        card.abilities.forEachIndexed { index, ability ->
            val active = dice.isNotEmpty() && controller.abilityTriggered(ability, dice)
            Row(
                Modifier
                    .fillMaxWidth()
                    .background(if (active) Color(0xFF1E3528) else Color.Transparent, RoundedCornerShape(6.dp))
                    .padding(horizontal = 6.dp, vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    if (active) "✓" else "·",
                    color = if (active) Success else Muted,
                    fontWeight = FontWeight.Black,
                    modifier = Modifier.padding(end = 6.dp)
                )
                Column {
                    Text(
                        "${index + 1}. ${abilityRequirement(ability)}  ${ability.name}",
                        fontSize = 12.sp,
                        fontWeight = if (active) FontWeight.Bold else FontWeight.Normal,
                        color = if (active) Ivory else Muted
                    )
                    Text(ability.text, fontSize = 10.sp, color = Muted)
                }
            }
        }
    }
}

@Composable
private fun TacticsPanel(controller: GameController) {
    val state = controller.state
    SectionTitle("TACTICAL ACTION")
    Text(
        if (state.playerTactic == null) {
            "Optional: spend two matching dice. Spent dice stop granting stats and cannot trigger abilities."
        } else {
            "Committed: ${state.playerTactic.title}. Your remaining dice determine abilities and base stats."
        },
        color = Muted,
        fontSize = 12.sp
    )

    TacticAction.entries.chunked(2).forEach { pair ->
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            pair.forEach { action ->
                OutlinedButton(
                    onClick = { controller.usePlayerTactic(action) },
                    enabled = controller.canUseTactic(action),
                    modifier = Modifier.weight(1f)
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Text(action.title, fontSize = 11.sp, fontWeight = FontWeight.Bold)
                        Text(
                            "${action.costFace.icon}${action.costFace.icon}",
                            fontSize = 13.sp,
                            color = faceColor(action.costFace)
                        )
                    }
                }
            }
            if (pair.size == 1) Spacer(Modifier.weight(1f))
        }
    }
}

@Composable
private fun ActiveRow(
    fighters: List<FighterState>,
    selectedId: String?,
    selectable: Boolean,
    onSelect: (String) -> Unit
) {
    Row(horizontalArrangement = Arrangement.spacedBy(7.dp)) {
        fighters.forEach { fighter ->
            val selected = fighter.card.id == selectedId
            Card(
                modifier = Modifier.weight(1f).clickable(enabled = selectable) { onSelect(fighter.card.id) },
                border = if (selected) BorderStroke(2.dp, Gold) else null,
                colors = CardDefaults.cardColors(containerColor = if (selected) Color(0xFF2C271C) else Panel)
            ) {
                Column(Modifier.padding(8.dp), horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        fighter.card.name,
                        textAlign = TextAlign.Center,
                        fontSize = 12.sp,
                        fontWeight = FontWeight.Bold,
                        maxLines = 2
                    )
                    Text(
                        "♥ ${fighter.hp}/${fighter.card.maxHp}",
                        color = if (fighter.hp <= fighter.card.maxHp / 3) Danger else Muted,
                        fontSize = 10.sp
                    )
                    Text("⚔ ${fighter.card.baseAttack}", color = Gold, fontSize = 10.sp)
                }
            }
        }
        repeat((3 - fighters.size).coerceAtLeast(0)) {
            Card(
                modifier = Modifier.weight(1f).height(66.dp),
                colors = CardDefaults.cardColors(containerColor = Color(0xFF11161D))
            ) {
                Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Text("EMPTY", color = Color(0xFF566170), fontSize = 10.sp)
                }
            }
        }
    }
}

@Composable
private fun ReserveLine(label: String, reserves: List<FighterState>) {
    Text(
        "$label: " + if (reserves.isEmpty()) "none" else reserves.joinToString { "${it.card.name} (${it.hp})" },
        color = Muted,
        fontSize = 10.sp
    )
}

@Composable
private fun CombatLog(lines: List<String>) {
    SectionTitle("COMBAT LOG")
    Card(colors = CardDefaults.cardColors(containerColor = Color(0xFF11161D))) {
        Column(Modifier.padding(10.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) {
            lines.takeLast(10).forEach { line ->
                Text("› $line", fontSize = 11.sp, color = Muted)
            }
        }
    }
}

@Composable
private fun GameOverPanel(controller: GameController) {
    val winner = controller.state.winner ?: "Match over"
    Card(
        colors = CardDefaults.cardColors(
            containerColor = if (winner == "Victory") Color(0xFF173020) else Color(0xFF301B1B)
        )
    ) {
        Column(
            Modifier.fillMaxWidth().padding(20.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Text(winner.uppercase(), fontSize = 28.sp, fontWeight = FontWeight.Black, color = Gold)
            Text("All eight characters on one side have been defeated.", color = Muted, fontSize = 12.sp)
            Button(onClick = { controller.restart() }) { Text("NEW MATCH") }
        }
    }
}

@Composable
private fun SectionTitle(text: String) {
    Text(text, color = Gold, fontSize = 11.sp, fontWeight = FontWeight.Bold)
}

private fun abilityRequirement(ability: AbilityCard): String =
    ability.requirements.entries.joinToString("") { (face, count) -> face.icon.repeat(count) }

private fun factionColor(faction: String): Color = when (faction) {
    "Wardens" -> Color(0xFF9EB7C8)
    "Wildborn" -> Color(0xFF8BC790)
    "Freeblades" -> Color(0xFFE7A779)
    "Cinders" -> Color(0xFFE88067)
    "Oracles" -> Color(0xFFB8A0E8)
    "Tempest" -> Color(0xFF78B8E8)
    else -> Muted
}

private fun faceColor(face: DiceFace): Color = when (face) {
    DiceFace.BLADE -> Color(0xFFE6D5B8)
    DiceFace.GUARD -> Color(0xFF8FC0D6)
    DiceFace.SPEED -> Color(0xFF8DD4A1)
    DiceFace.POWER -> Color(0xFFE88966)
    DiceFace.FOCUS -> Color(0xFFC1A3EA)
    DiceFace.WILD -> Gold
}
