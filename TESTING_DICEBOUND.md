# Dicebound v0.1 test checklist

## Setup
- [ ] App launches to the opening-line screen.
- [ ] Exactly three heroes can be selected.
- [ ] A selected hero can be deselected.
- [ ] Begin Match only enables at 3/3.

## Match flow
- [ ] Location remains the same through the match.
- [ ] Event changes between rounds.
- [ ] First commit alternates between player and AI.
- [ ] When AI commits first, its hero is visible before player selection.
- [ ] When player commits first, AI responds after the selection.

## Dice
- [ ] Six dice appear.
- [ ] Tapping a die toggles lock.
- [ ] Locked dice survive rerolls.
- [ ] Exactly two rerolls are available.
- [ ] Wild dice can substitute for non-Wild ability symbols.
- [ ] Location/event modifiers appear in the stat preview.

## Tactics
- [ ] A tactic only enables with two matching unspent dice.
- [ ] Only one tactic can be used per duel.
- [ ] Spent dice are visibly marked.
- [ ] Spent dice stop granting stats.
- [ ] Spending dice can turn an otherwise-active ability off.

## Resolution
- [ ] Active abilities highlight before resolution.
- [ ] Speed decides resolution order; commit order breaks ties.
- [ ] Ability tiers alternate rather than one hero resolving all abilities at once.
- [ ] Guard reduces basic attack damage.
- [ ] Shields absorb damage.
- [ ] Jam cancels the target's next triggered ability.
- [ ] HP persists across rounds.
- [ ] Defeated characters leave the active line.
- [ ] Living reserves are promoted.
- [ ] Match ends once one full 8-character roster is defeated.
- [ ] New Match returns to roster selection.

## Balance notes to collect
- Which face is most/least desirable?
- How often does a roll activate zero / one / two / three abilities?
- Is spending two dice on a tactic usually meaningful or obviously wrong?
- Does Speed matter too much because it controls ability and attack order?
- Are events interesting without feeling arbitrary?
- Do tank/heal characters make matches too long?
